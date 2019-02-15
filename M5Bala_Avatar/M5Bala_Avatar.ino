/********************************************************
 * M5Bala balance car Basic Example
 * Reading encoder and writting the motor via I2C
 ********************************************************/

#include <M5Stack.h>
#include <NeoPixelBus.h>
#include <Wire.h>
#include <Preferences.h>
#include "M5Bala.h"
#include <Avatar.h>

//#define USE_AQUESTALK
#ifdef USE_AQUESTALK
#include <tasks/LipSync.h>
// AquesTalk License Key
// NULL or wrong value is just ignored
const char* AQUESTALK_KEY = "XXXX-XXXX-XXXX-XXXX";
#endif

using namespace m5avatar;

Avatar avatar;
Face* myFace;

Preferences preferences;
M5Bala m5bala(Wire);

// ==================== NeoPixel =====================
const uint16_t PixelCount = 10;
const uint8_t PixelPin = 15;

#define colorSaturation 10
// three element pixels, in different order and speeds
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

void LED_start(RgbColor color) {
	strip.Begin();
	for (int i = 0; i < 10; i++) {
		strip.SetPixelColor(i, color);
	}
	strip.Show();
}

// ================ GYRO offset param ==================
void auto_tune_gyro_offset() {
	M5.Speaker.tone(500, 200);
	delay(300);
	M5.update();
	M5.Lcd.println("Start IMU calculate gyro offsets");
	M5.Lcd.println("DO NOT MOVE A MPU6050...");
	delay(2000);

	m5bala.imu->calcGyroOffsets(true);
	float gyroXoffset = m5bala.imu->getGyroXoffset();
	float gyroYoffset = m5bala.imu->getGyroYoffset();
	float gyroZoffset = m5bala.imu->getGyroZoffset();
	M5.Lcd.println("Done!!!");
	M5.Lcd.print("X : ");M5.Lcd.println(gyroXoffset);
	M5.Lcd.print("Y : ");M5.Lcd.println(gyroYoffset);
	M5.Lcd.print("Z : ");M5.Lcd.println(gyroZoffset);
	M5.Lcd.println("Program will start after 3 seconds");
	M5.Lcd.print("========================================");

	// Save
	preferences.putFloat("gyroXoffset", gyroXoffset);
	preferences.putFloat("gyroYoffset", gyroYoffset);
	preferences.putFloat("gyroZoffset", gyroZoffset);
	preferences.end();
}

void setup() {
	// Power ON Stabilizing...
	delay(500);
#ifdef USE_AQUESTALK
  int iret = TTS.create(AQUESTALK_KEY);
#endif
	M5.begin();

	// Turn on LED BAR
	LED_start(white);

	// Init I2C
	Wire.begin();
	Wire.setClock(400000UL);  // Set I2C frequency to 400kHz
	delay(500);

	// Display info
	M5.Lcd.setTextFont(2);
	M5.Lcd.setTextColor(WHITE, BLACK);
	M5.Lcd.println("M5Stack Balance Mode start");

	// Init M5Bala
	m5bala.begin();

	// Loading the IMU parameters
	if (M5.BtnC.isPressed()) {
		preferences.begin("m5bala-cfg", false);
		auto_tune_gyro_offset();

	} else {
		preferences.begin("m5bala-cfg", true);
		m5bala.imu->setGyroOffsets( preferences.getFloat("gyroXoffset"), 
                                preferences.getFloat("gyroYoffset"), 
                                preferences.getFloat("gyroZoffset"));
	}

  avatar.init();
#ifdef USE_AQUESTALK
  avatar.addTask(lipSync, "lipSync");
#endif
}

void loop() {

	// Avatar
	static uint32_t print_interval = millis() + 30;
	if (millis() > print_interval) {
    print_interval = millis() + 500;
    float angle = m5bala.getAngle();
    if(abs(angle) < 2.0) {
      avatar.setExpression(Expression::Happy);      
      avatar.setSpeechText("");
    } else if(abs(angle) < 3.5) {
      avatar.setExpression(Expression::Neutral);      
      avatar.setSpeechText("");
    } else if(abs(angle) < 30) {
      avatar.setExpression(Expression::Angry);
      avatar.setSpeechText("Don't touch me !  ");
#ifdef USE_AQUESTALK
      if (!TTS.isPlay()) TTS.play("yamero", 150);
#endif
    } else {
      avatar.setExpression(Expression::Sad);
      avatar.setSpeechText("Help me ! ");
    }
	}

	// M5Bala balance run
	m5bala.run();

	// M5 Loop
	M5.update();
}

#include <Arduino.h>
#include <Wire.h>
#include <Keyboard.h>
#include "RTClib.h"
#include "PT6961.h"
#include "IRremote.h"

#define UP_BUTTON A2
#define DOWN_BUTTON A3
#define LEFT_BUTTON 7
#define RIGHT_BUTTON 5
#define ENTER_BUTTON 6
#define MENU_BUTTON 4

#define POWER_BUTTON 8
#define POWER_LED 9

#define SCREEN_DIN 10
#define SCREEN_CLK 16
#define SCREEN_CS 14

#define IR_RECEIVER 15

#define POWER_RELAY A0

#define RASPBERRY_TIMEOUT 15000
#define FORCE_SHUTDOWN_TIMEOUT 8000

bool upPressed = false;
bool downPressed = false;
bool leftPressed = false;
bool rightPressed = false;
bool enterPressed = false;
bool menuPressed = false;
bool powerPressed = false;

#define OFF 0
#define GOING_ON 1
#define ON 2
#define REBOOTING 3
#define SHUTTING_DOWN 4

int powerState = OFF;
unsigned long shuttingDownTime = 0L;
unsigned long powerPressedTime = 0L;

#define REMOTE_PROTOCOL NEC
#define REMOTE_POWER 0x807F02FD
#define REMOTE_SETUP 0x807FC23D
#define REMOTE_APP 0x807FF00F
#define REMOTE_VOL_DOWN 0x807F08F7
#define REMOTE_VOL_UP 0x807F18E7
#define REMOTE_HOME 0x807F8877
#define REMOTE_BACK 0x807F9867
#define REMOTE_MENU 0x807F32CD
#define REMOTE_CONTEXT 0x807F00FF
#define REMOTE_UP 0x807F6897
#define REMOTE_DOWN 0x807F58A7
#define REMOTE_LEFT 0x807F8A75
#define REMOTE_RIGHT 0x807F0AF5
#define REMOTE_OK 0x807FC837
#define REMOTE_1 0x807F728D
#define REMOTE_2 0x807FB04F
#define REMOTE_3 0x807F30CF
#define REMOTE_4 0x807F52AD
#define REMOTE_5 0x807F906F
#define REMOTE_6 0x807F10EF
#define REMOTE_7 0x807F629D
#define REMOTE_8 0x807FA05F
#define REMOTE_9 0x807F20DF
#define REMOTE_0 0x807F807F
#define REMOTE_MUTE 0x807F827D
#define REMOTE_BACKSPACE 0x807F42BD
#define REMOTE_REPEAT 0xFFFFFFFF

#define KEY_NUMPAD_1 225
#define KEY_NUMPAD_2 226
#define KEY_NUMPAD_3 227
#define KEY_NUMPAD_4 228
#define KEY_NUMPAD_5 229
#define KEY_NUMPAD_6 230
#define KEY_NUMPAD_7 231
#define KEY_NUMPAD_8 232
#define KEY_NUMPAD_9 233
#define KEY_NUMPAD_0 234
#define KEY_MENU KEY_F1
#define KEY_CONTEXT KEY_F2
#define KEY_HOME_SCREEN KEY_F3
#define KEY_SETUP KEY_F4
#define KEY_APP KEY_F5
#define KEY_MUTE KEY_F8
#define KEY_VOLUME_MINUS KEY_F9
#define KEY_VOLUME_PLUS KEY_F10
#define KEY_SHUTDOWN KEY_F12

#define REMOTE_PRESSED_TIMEOUT 150

unsigned int lastIrCode = 0;
unsigned long lastRemotePressedTime = 0L;

PT6961 screen(SCREEN_DIN, SCREEN_CLK, SCREEN_CS);
IRrecv irReceiver(IR_RECEIVER);
RTC_DS3231 rtc;

void setup() {
  Serial.begin(9600);
  initControlButtons();
  initPowerButton();
  initScreen();
  initIRReciever();
  initPowerRelay();
  initRTC();
}

void initControlButtons() {
  pinMode(UP_BUTTON, INPUT_PULLUP);
  pinMode(DOWN_BUTTON, INPUT_PULLUP);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  pinMode(ENTER_BUTTON, INPUT_PULLUP);
  pinMode(MENU_BUTTON, INPUT_PULLUP);
  Keyboard.begin();
}

void initPowerButton() {
  pinMode(POWER_BUTTON, INPUT_PULLUP);
  pinMode(POWER_LED, OUTPUT);
  analogWrite(POWER_LED, 0);
}

void initScreen() {
  screen.initDisplay();
  screen.sendCmd(PT6961::_DISPLAY_1_16);
}

void initIRReciever() {
  irReceiver.enableIRIn();
}

void initPowerRelay() {
  pinMode(POWER_RELAY, OUTPUT);
  digitalWrite(POWER_RELAY, HIGH);
}

void initRTC() {
  if (!rtc.begin()) {
    while (true);
  }
}

void loop() {
  controlButtons();
  powerManagement();
  powerButton();
  serialComm();
  displayTime();
  irComm();
}

void controlButtons() {
  checkButtonStateChange(UP_BUTTON, upPressed, KEY_UP_ARROW);
  checkButtonStateChange(DOWN_BUTTON, downPressed, KEY_DOWN_ARROW);
  checkButtonStateChange(LEFT_BUTTON, leftPressed, KEY_LEFT_ARROW);
  checkButtonStateChange(RIGHT_BUTTON, rightPressed, KEY_RIGHT_ARROW);
  checkButtonStateChange(ENTER_BUTTON, enterPressed, KEY_RETURN);
  checkButtonStateChange(MENU_BUTTON, menuPressed, KEY_MENU);
}

void checkButtonStateChange(int button, bool & buttonState, char buttonKey) {
  if (isButtonPressed(button) ^ buttonState) {
    buttonState = !buttonState;
    if (powerState == ON) {
      if (buttonState) {
        Keyboard.press(buttonKey);
      } else {
        Keyboard.release(buttonKey);
      }
    }
  }
}

void powerManagement() {
  if (powerState == SHUTTING_DOWN && millis() - shuttingDownTime >= RASPBERRY_TIMEOUT) {
    digitalWrite(POWER_RELAY, HIGH);
    analogWrite(POWER_LED, 0);
    powerState = OFF;

  }
}

void powerButton() {
  if (isButtonPressed(POWER_BUTTON)) {
    if (powerPressed) {
      if (millis() - powerPressedTime >= FORCE_SHUTDOWN_TIMEOUT) {
        digitalWrite(POWER_RELAY, HIGH);
        analogWrite(POWER_LED, 0);
        powerState = OFF;
      }
    } else {
      powerPressedTime = millis();
      powerPressed = true;
	  pressPower();
    }
  } else {
    powerPressed = false;
  }
}

void pressPower(){
	if (powerState == ON) {
		/*Keyboard.press(KEY_LEFT_CTRL);
		Keyboard.press(KEY_END);
		Keyboard.release(KEY_LEFT_CTRL);
		Keyboard.release(KEY_END);*/
		Keyboard.press(KEY_SHUTDOWN);
		Keyboard.release(KEY_SHUTDOWN);
		} else if (powerState == OFF) {
		digitalWrite(POWER_RELAY, LOW);
		analogWrite(POWER_LED, voltage(3.3));
		powerState = GOING_ON;
	}
}

int voltage(double value) {
  return (255 / 5) * value;
}

bool isButtonPressed(int button) {
  return digitalRead(button) == LOW;
}

void displayTime() {
  DateTime now = rtc.now();
  screen.sendNum(now.hour() * 100 + now.minute(), now.second() % 2 == 0);
}

void serialComm() {
  while (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    switch (input[0]) {
      case 'U':
        powerState = ON;
        break;
      case 'T': {
          int pipeIndex = input.indexOf('|');
          String date = input.substring(1, pipeIndex);
          String time = input.substring(pipeIndex + 1);
          DateTime newTime(date.c_str(), time.c_str());
          rtc.adjust(newTime);
        }
        break;
      case 'H':
        powerState = SHUTTING_DOWN;
        shuttingDownTime = millis();
        break;
      case 'R':
        powerState = REBOOTING;
        break;
    }
  }
}

void irComm(){
	decode_results results;
	bool hasIrData = irReceiver.decode(&results);
	if(hasIrData && results.decode_type == REMOTE_PROTOCOL) {
		unsigned long tempTime = lastRemotePressedTime;
		lastRemotePressedTime = millis();
		switch(results.value){
			case REMOTE_POWER:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressPower();
				break;
			case REMOTE_UP:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_UP_ARROW);
				break;
			case REMOTE_DOWN:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_DOWN_ARROW);
				break;
			case REMOTE_LEFT:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_LEFT_ARROW);
				break;
			case REMOTE_RIGHT:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_RIGHT_ARROW);
				break;
			case REMOTE_OK:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_RETURN);
				break;
			case REMOTE_MUTE:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_MUTE);
				break;
			case REMOTE_BACKSPACE:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_BACKSPACE);
				break;
			case REMOTE_1:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_NUMPAD_1);
				break;
			case REMOTE_2:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_NUMPAD_2);
				break;
			case REMOTE_3:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_NUMPAD_3);
				break;
			case REMOTE_4:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_NUMPAD_4);
				break;
			case REMOTE_5:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_NUMPAD_5);
				break;
			case REMOTE_6:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_NUMPAD_6);
				break;
			case REMOTE_7:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_NUMPAD_7);
				break;
			case REMOTE_8:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_NUMPAD_8);
				break;
			case REMOTE_9:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_NUMPAD_9);
				break;
			case REMOTE_0:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_NUMPAD_0);
				break;
			case REMOTE_MENU:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_MENU);
				break;
			case REMOTE_CONTEXT:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_CONTEXT);
				break;
			case REMOTE_BACK:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_ESC);
				break;
			case REMOTE_HOME:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_HOME_SCREEN);
				break;
			case REMOTE_VOL_UP:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_VOLUME_PLUS);
				break;
			case REMOTE_VOL_DOWN:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_VOLUME_MINUS);
				break;
			case REMOTE_SETUP:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_SETUP);
				break;
			case REMOTE_APP:
				releaseRemoteButtons();
				lastIrCode = results.value;
				pressKeyIfOn(KEY_APP);
				break;
			case REMOTE_REPEAT:
				break;
			default:
				lastRemotePressedTime = tempTime;
		}
	}
	if(lastIrCode != 0 && millis() - lastRemotePressedTime >= REMOTE_PRESSED_TIMEOUT){
		releaseRemoteButtons();
		lastIrCode = 0;
	}
	if(hasIrData){
		irReceiver.resume();
	}
}

void releaseRemoteButtons(){
	if(powerState == ON){
		switch(lastIrCode){
			case REMOTE_UP:
				releaseKeyIfOn(KEY_UP_ARROW);
				break;
			case REMOTE_DOWN:
				releaseKeyIfOn(KEY_DOWN_ARROW);
				break;
			case REMOTE_LEFT:
				releaseKeyIfOn(KEY_LEFT_ARROW);
				break;
			case REMOTE_RIGHT:
				releaseKeyIfOn(KEY_RIGHT_ARROW);
				break;
			case REMOTE_OK:
				releaseKeyIfOn(KEY_RETURN);
				break;
			case REMOTE_MUTE:
				releaseKeyIfOn(KEY_F8);
				break;
			case REMOTE_BACKSPACE:
				releaseKeyIfOn(KEY_BACKSPACE);
				break;
			case REMOTE_1:
				releaseKeyIfOn(KEY_NUMPAD_1);
				break;
			case REMOTE_2:
				releaseKeyIfOn(KEY_NUMPAD_2);
				break;
			case REMOTE_3:
				releaseKeyIfOn(KEY_NUMPAD_3);
				break;
			case REMOTE_4:
				releaseKeyIfOn(KEY_NUMPAD_4);
				break;
			case REMOTE_5:
				releaseKeyIfOn(KEY_NUMPAD_5);
				break;
			case REMOTE_6:
				releaseKeyIfOn(KEY_NUMPAD_6);
				break;
			case REMOTE_7:
				releaseKeyIfOn(KEY_NUMPAD_7);
				break;
			case REMOTE_8:
				releaseKeyIfOn(KEY_NUMPAD_8);
				break;
			case REMOTE_9:
				releaseKeyIfOn(KEY_NUMPAD_9);
				break;
			case REMOTE_0:
				releaseKeyIfOn(KEY_NUMPAD_0);
				break;
			case REMOTE_MENU:
				releaseKeyIfOn(KEY_MENU);
				break;
			case REMOTE_CONTEXT:
				releaseKeyIfOn(KEY_CONTEXT);
				break;
			case REMOTE_BACK:
				releaseKeyIfOn(KEY_ESC);
				break;
			case REMOTE_HOME:
				releaseKeyIfOn(KEY_HOME_SCREEN);
				break;
			case REMOTE_VOL_UP:
				releaseKeyIfOn(KEY_VOLUME_PLUS);
				break;
			case REMOTE_VOL_DOWN:
				releaseKeyIfOn(KEY_VOLUME_MINUS);
				break;
			case REMOTE_SETUP:
				releaseKeyIfOn(KEY_SETUP);
				break;
			case REMOTE_APP:
				releaseKeyIfOn(KEY_APP);
				break;
		}
	}
}

void pressKeyIfOn(char key){
	if(powerState == ON){
		Keyboard.press(key);
	}
}

void releaseKeyIfOn(char key){
	if(powerState == ON){
		Keyboard.release(key);
	}
}

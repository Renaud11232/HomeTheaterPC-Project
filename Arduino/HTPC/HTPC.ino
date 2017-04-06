//TODO INFRARED

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

#define IR_RECIEVER 15

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

PT6961 screen(SCREEN_DIN, SCREEN_CLK, SCREEN_CS);
IRrecv irReciever(IR_RECIEVER);
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
  irReciever.enableIRIn();
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
}

void controlButtons() {
  checkButtonStateChange(UP_BUTTON, upPressed, KEY_UP_ARROW);
  checkButtonStateChange(DOWN_BUTTON, downPressed, KEY_DOWN_ARROW);
  checkButtonStateChange(LEFT_BUTTON, leftPressed, KEY_LEFT_ARROW);
  checkButtonStateChange(RIGHT_BUTTON, rightPressed, KEY_RIGHT_ARROW);
  checkButtonStateChange(ENTER_BUTTON, enterPressed, KEY_RETURN);
  checkButtonStateChange(MENU_BUTTON, menuPressed, ';');
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
      if (powerState == ON) {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(KEY_END);
        Keyboard.release(KEY_LEFT_CTRL);
        Keyboard.release(KEY_END);
      } else if (powerState == OFF) {
        digitalWrite(POWER_RELAY, LOW);
        analogWrite(POWER_LED, voltage(3.3));
        powerState = GOING_ON;
      }
    }
  } else {
    powerPressed = false;
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


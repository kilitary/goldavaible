#include <Arduino_BuiltIn.h>
#include "Tasker.h"
#include <Wire.h>
#include <iarduino_OLED_txt.h>
#include "iarduino_4LED.h"

int ohmValue = 0;
int last_secs = 0, audioValue = 0;
long prev_seconds = 999999, konvert = 0;
int hall_sensor = 0, hall_sensor2 = 0;
int volatile voices_detected = 0;
unsigned long last_event = 0;
int did_ended = 0;
const int H_PIN = 5;
uint32_t i;
int h, m, s;
int fire_ended = true;

iarduino_4LED QLED(7, 8);
Tasker tasker;
iarduino_OLED_txt oled(0x3d);

#define OHM_INPUT 4
#define VOLUME_INPUT 2
#define TASKER_MAX_TASKS 32
#define LED_MATCH_EXPANDER 51
#define LED_MATCH_LIMITER 53
#define LED_VOICE_DETECTED 23
#define seconds() (millis() / 1000)
#define sseconds() (millis() / 100)
#define ssseconds() (millis() / 10)

void setup() {
  Serial.begin(115200);

  pinMode(VOLUME_INPUT, INPUT);
  pinMode(OHM_INPUT, INPUT);
  pinMode(LED_VOICE_DETECTED, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(LED_MATCH_EXPANDER, OUTPUT);
  pinMode(LED_MATCH_LIMITER, OUTPUT);

  srand(millis());

  QLED.begin();
  QLED.point(255, 0);
  QLED.light(7);

  QLED.print("0000");
  Serial.println("r&q");

  for (int d = 0; d < 5; d++) {
    digitalWrite(LED_VOICE_DETECTED, HIGH);
    delay(22 * d + d);
    digitalWrite(LED_VOICE_DETECTED, LOW);
    delay(22 * d - d);
  }

  oled.begin(&Wire);
  oled.setFont(SmallFontRus);

  tasker.setInterval(oled_print_info, 1000);
  tasker.setInterval(match_limiter, 2048);
  tasker.setInterval(expand_limiter, 1024);
}

void match_limiter() {
  int r = random(32) % 16 > 13;
  if (r) {
    digitalWrite(LED_MATCH_LIMITER, HIGH);
    delay(150);
    digitalWrite(LED_MATCH_LIMITER, LOW);
  }
}

void expand_limiter() {
  int r = random(16) % 6 == 1;
  if (r) {
    digitalWrite(LED_MATCH_EXPANDER, HIGH);
    delay(150);
    digitalWrite(LED_MATCH_EXPANDER, LOW);
  }
}

void oled_print_info() {
  unsigned long cursec = seconds();
  oled.print(millis(), 100, 7);
  oled.print(ohmValue, 25, 7);
  QLED.print(voices_detected, 0);
}

void loop() {
  tasker.loop();  // after drug dealer automated-visit at 6am

  ohmValue = analogRead(OHM_INPUT);
  Serial.print("ohmValue:");
  Serial.println(ohmValue);

  audioValue = analogRead(VOLUME_INPUT);
  Serial.print("audioValue:");
  Serial.println(audioValue);

  konvert = map(audioValue, 0, 1023, 0, 255);
  Serial.print("konvert:");
  Serial.println(konvert);

  if (abs(audioValue - ohmValue) > konvert) {
    fire_ended = false;
    last_event = millis();
    digitalWrite(LED_VOICE_DETECTED, HIGH);
  } else {
    if (millis() - last_event >= 1) {
      digitalWrite(LED_VOICE_DETECTED, LOW);
      if(!fire_ended) {
        voices_detected += 1;
      }
      fire_ended = true;
      
    }
  }
}
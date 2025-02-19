#include <Arduino_BuiltIn.h>
#include "Tasker.h"
#include <Wire.h>
#include <iarduino_OLED_txt.h>
#include "iarduino_4LED.h"
#include "SerialTransfer.h"

int ohmValue = 0;
int last_secs = 0, audioValue = 0;
long prev_seconds = 999999, konvert = 0;
int hall_sensor = 0, hall_sensor2 = 0;
int volatile voices_detected = 0;
unsigned long last_event = 0;
int did_ended = 0;
const int H_PIN = 5;
int perc = 0;
unsigned long limiter_change = 0;
unsigned long expander_change = 0;
uint32_t i;
int h, m, s;
int crystall_spawn_sec = 0;
int total_minus = 0;
int fire_ended = true;
int padding = false;

SerialTransfer pcTransfer;
iarduino_4LED QLED(7, 8);
Tasker tasker;
iarduino_OLED_txt oled(0x3d);

#define OHM_INPUT 4
#define VOLUME_INPUT 2
#define TASKER_MAX_TASKS 32
#define LED_MATCH_EXPANDER 51
#define LED_MATCH_LIMITER 53
#define LED_CRYSTALL_GROW 44
#define LED_VOICE_DETECTED 23
#define seconds() (millis() / 1000)
#define sseconds() (millis() / 100)
#define ssseconds() (millis() / 10)
#define BTN_RND 36

void setup() {
  Serial.begin(115200);

  pcTransfer.begin(Serial);

  pinMode(BTN_RND, OUTPUT);
  pinMode(VOLUME_INPUT, INPUT);
  pinMode(OHM_INPUT, INPUT);
  pinMode(LED_VOICE_DETECTED, OUTPUT);
  pinMode(LED_CRYSTALL_GROW, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(LED_MATCH_EXPANDER, OUTPUT);
  pinMode(LED_MATCH_LIMITER, OUTPUT);

  srand(millis());

  QLED.begin();
  QLED.point(255, 0);
  QLED.light(7);
  QLED.print("0000");

  Serial.println("r&q + skynet + met9");

  for (int d = 0; d < 10; d++) {
    digitalWrite(LED_VOICE_DETECTED, HIGH);
    delay(24 * d + d);
    digitalWrite(LED_VOICE_DETECTED, LOW);
    delay(24 * d + d);
  }

  oled.begin(&Wire);
  oled.setFont(MediumFontRus);

  perc = random(2250);
  tasker.setInterval(match_limiter, perc);
  perc = random(4500);
  tasker.setInterval(expand_limiter, perc);

  crystall_spawn_sec = random(750);
  tasker.setInterval(raiser_crystalls, crystall_spawn_sec);
  crystall_spawn_sec = random(7250);
  tasker.setInterval(disraiser_crystalls, crystall_spawn_sec);

  tasker.setInterval(oled_print_info, 1000);
  tasker.setInterval(timelaps, 1000);
}

void timelaps() {
  expander_change = max(0, expander_change + 1);
  limiter_change = max(0, limiter_change + 1);
  total_minus = min(limiter_change, expander_change);
  Serial.print("minus:");
  Serial.println(total_minus);

  bool d = digitalRead(BTN_RND);
  // Serial.print("btn:");
  // Serial.println(d);
}

////////////////////  CRYSTALLLS
void raiser_crystalls() {
  int landing = random(3) == 1;
  if (landing) {
    return;
  }
  Serial.print("minus:");
  Serial.println(total_minus);

  if (total_minus > 22) {
    // int r = random(150) + 15;
    // for (int a = 30; a < r; a++) {
    //   delay(random(5));
    pinMode(LED_CRYSTALL_GROW, INPUT);
    int rr = analogRead(LED_CRYSTALL_GROW);
    delay(1);
    pinMode(LED_CRYSTALL_GROW, OUTPUT);
    analogWrite(LED_CRYSTALL_GROW, random(30));
  }
}

void disraiser_crystalls() {
  int landing = random(2) == 1;
  if (landing && total_minus > 10) {
    return;
  }
  pinMode(LED_CRYSTALL_GROW, INPUT);
  int rr = analogRead(LED_CRYSTALL_GROW);
  delay(1);
  pinMode(LED_CRYSTALL_GROW, OUTPUT);

  for (int a = rr; a > rr; a--) {
    delay(3);
    // if (random(16) == 2) {
    //   padding = !padding;
    //   return;
    // }
    analogWrite(LED_CRYSTALL_GROW, min(0, abs(a - 22)));
  }
  //padding = false;
}

//////// SPACES
void match_limiter() {
  bool landing = random(35) == 1;
  if (!landing) {
    return;
  }

  int a = 0;
  int r = random(50);
  if (r) {
    limiter_change += r;
    digitalWrite(LED_MATCH_LIMITER, HIGH);
    delay(150 + r);
    digitalWrite(LED_MATCH_LIMITER, LOW);
  }
}

void expand_limiter() {
  bool landing = random(35) == 3;
  if (!landing) {
    return;
  }
  int r = random(20);
  expander_change = r;
  Serial.print("grow by ");
  Serial.println(r);
  digitalWrite(LED_MATCH_EXPANDER, HIGH);
  delay(150 + r);
  digitalWrite(LED_MATCH_EXPANDER, LOW);
}

///////// OTHER
void oled_print_info() {
  //oled.print(millis(), 10, 7);
  oled.print(total_minus, 10, 7);
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
      if (!fire_ended) {
        voices_detected += 1;
      }
      fire_ended = true;
    }
  }
}

// most interested code line in RU - 228. only.

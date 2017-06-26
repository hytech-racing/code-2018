/*
 * Nathan Cheek
 * 2016-11-23
 * Test cooling board
 */
#include <Metro.h>

int p = 0;
//bool rising = true;

Metro timer = Metro(10);

void setup() {
  // put your setup code here, to run once:
  pinMode(13, OUTPUT);
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(13, p);
  digitalWrite(A0, p);
  digitalWrite(A1, p);

  if (p < 255 && timer.check()) {
    p++;
    Serial.println(p);
  }
}

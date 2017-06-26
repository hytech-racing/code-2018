#include <Metro.h>

int pulses = 0;
float flowRate = 0.0;
int milli = 300 //time in milliseconds of each cycle to poll for
Metro pulseTimer = Metro(milli);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, OUTPUT);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  if(pulseTimer.check()) {
    
    int q1 = digitalRead(2);
    int q2 = digitalRead(3);
    int q3 = digitalRead(4);
    int q4 = digitalRead(5);


  // if a plastic sensor use the following calculation
  // Sensor Frequency (Hz) = 7.5 * Q (Liters/min)
  // Liters = Q * time elapsed (seconds) / 60 (seconds/minute)
  // Liters = (Frequency (Pulses/second) / 7.5) * time elapsed (seconds) / 60
  // Liters = Pulses / (7.5 * 60)
    pulses = q1 * (1 << 0) + q2 * (1 << 1) + q3 * (1 << 2) + q4 * (1 << 3);
    Serial.print("pulses: ");
    Serial.println(pulses);
    float freq = pulses/((float)milli/1000);
    flowRate = freq/7.5;
    Serial.print("flow rate: ");
    Serial.println(flowRate);
    digitalWrite(6, HIGH);
    digitalWrite(6, LOW);
    pulseTimer.reset();
  }



}

#include <Servo.h> 

Servo myservo;

void setup() 
{ 
  pinMode(LED_BUILTIN, OUTPUT);
  myservo.attach(6);
} 

void loop() {
  if (millis()%5000 > 2500){
    myservo.write(50);
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    myservo.write(150);
    digitalWrite(LED_BUILTIN, HIGH);
  }
} 

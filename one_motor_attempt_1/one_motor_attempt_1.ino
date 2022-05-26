#define X_STEP 2
#define X_DIR 5

void setup() {
  pinMode(X_STEP, OUTPUT);
  pinMode(X_DIR, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  Serial.println("forward");
  digitalWrite(X_DIR, HIGH);
  delay(1000);
  for (int i = 0; i < 2; i++) {
     digitalWrite(X_STEP, HIGH);
     delayMicroseconds(500);
     digitalWrite(X_STEP, LOW);
     delayMicroseconds(500);
  }
  Serial.println("reverse");
  digitalWrite(X_DIR, LOW);
  delay(1000);
  for (int i = 0; i < 200; i++) {
     digitalWrite(X_STEP, HIGH);
     delayMicroseconds(500);
     digitalWrite(X_STEP, LOW);
     delayMicroseconds(500);
     
  }
}

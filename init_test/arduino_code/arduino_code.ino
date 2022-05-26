

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    if (command.startsWith("on ")) {
      Serial.write("1");
      int num = command.substring(3).toInt();
      for (int i = 0; i < num; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
        delay(500);
      }
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else if (command == "off") {
      digitalWrite(LED_BUILTIN, LOW);
      Serial.write("0");
    }
    //Serial.println("value: " + command);
  }
}

#include <Servo.h>
//#include <AccelStepper.h>

//motor controllers pins
#define H_DIR_PIN 2
#define H_STEP_PIN 3

#define V_DIR_PIN 4
#define V_STEP_PIN 5

#define V_DIR_FORWARD HIGH
#define V_DIR_BACKWARD LOW
#define H_DIR_FORWARD HIGH
#define H_DIR_BACKWARD LOW

//motor controller microstep settings
#define MS1_PIN 8
#define MS2_PIN 9
#define MS3_PIN 10
/* 
 * +-----+-----+-----+-----------+
 * | MS1 | MS2 | MS3 | Step Size |
 * +-----+-----+-----+-----------+
 * |  0  |  0  |  0  |     1     |
 * +-----+-----+-----+-----------+
 * |  1  |  0  |  0  |    1/2    |
 * +-----+-----+-----+-----------+
 * |  0  |  1  |  0  |    1/4    |
 * +-----+-----+-----+-----------+
 * |  1  |  1  |  0  |    1/8    |
 * +-----+-----+-----+-----------+
 * |  1  |  1  |  1  |    1/16   |
 * +-----+-----+-----+-----------+
 */

//limit switch pins
#define H_LIMIT_PIN 11
#define V_LIMIT_PIN 12

//servo
#define SERVO_SIGNAL_PIN 6
int penUpSP = 40;
int penDownSP = 30;
Servo penServo;

//motors
//AccelStepper hStepper(AccelStepper::DRIVER, H_STEP_PIN, H_DIR_PIN);
//AccelStepper vStepper(AccelStepper::DRIVER, V_STEP_PIN, V_DIR_PIN);

void setStep(bool ms1, bool ms2, bool ms3) {
	digitalWrite(MS1_PIN, ms1 ? HIGH : LOW);
	digitalWrite(MS2_PIN, ms2 ? HIGH : LOW);
	digitalWrite(MS3_PIN, ms3 ? HIGH : LOW);
}

void setup() {
  //setting pinmodes
  pinMode(H_DIR_PIN, OUTPUT);
  pinMode(H_STEP_PIN, OUTPUT);
  pinMode(V_DIR_PIN, OUTPUT);
  pinMode(V_STEP_PIN, OUTPUT);

  pinMode(MS1_PIN, OUTPUT);
  pinMode(MS2_PIN, OUTPUT);
  pinMode(MS3_PIN, OUTPUT);

  pinMode(H_LIMIT_PIN, INPUT);
  pinMode(V_LIMIT_PIN, INPUT);

  //setting up outputs
  setStep(false, false, false);
  digitalWrite(H_DIR_PIN, LOW);
  digitalWrite(H_STEP_PIN, LOW);
  digitalWrite(V_DIR_PIN, LOW);
  digitalWrite(V_STEP_PIN, LOW);

  //setting up servo
  penServo.attach(SERVO_SIGNAL_PIN);

  //setting up communication(
  Serial.begin(9600);
} 

int getServoSetpoint() {
  int setpoint = 90;
  int delta = 30;
  while (true) {
      penServo.write(setpoint);
      delay(100);
      //wait for signal
      while (Serial.available() == 0) {}
      //respond according to signal
      String command = Serial.readStringUntil("\n");
      if (command.startsWith("+")) {
        setpoint+=delta;
      }
      else if (command.startsWith("-")) {
        setpoint-=delta;
      }
      else if (command.startsWith("=")) {
        break;
      }
      else if (command.startsWith("<")) {
        delta*=4;
      }
      delta/=2;
      delta|=1;
    }
    return setpoint;
}

//listener for pen commands
void penCommandListener(String command) {
  if (command.startsWith("penPress")) {
	penCommandListener("penDown");
	penCommandListener("penUp");
  } else if (command.startsWith("penUp")) {
    penServo.write(penUpSP);
    //delay(20);
    //penServo.write(penUpSP);
  } else if (command.startsWith("penDown")) {
    penServo.write(penDownSP + 4);
    delay(100);
    penServo.write(penDownSP + 1);
    delay(20);
    penServo.write(penDownSP);
    delay(50);
    //TEST? penServo.detach();
  } else if (command.startsWith("penReset")) {
    int setpoint = 90;
    int delta = 45;//gonna be 45, 23, 11, 5, 2*

    //set lower setpoint
    Serial.println("Setting lower pen setpoint");
    penDownSP = getServoSetpoint();
    Serial.println("LowSP: " + String(penDownSP));

    //set upper setpoint
    Serial.println("Setting upper pen setpoint");
    penUpSP = penDownSP + 10;//getServoSetpoint();
    Serial.println("UpSP: " + String(penUpSP));
    
    Serial.write("Done resetting pen setpoints");
  } else if (command.startsWith("penSet")) {
    penDownSP = command.substring(command.indexOf(" ") + 1).toInt();
    penUpSP = penDownSP + 10;
  }
}

void step(int step_pin, int count) {
	for (int i = 0; i < count; i++) {
		digitalWrite(step_pin, HIGH);
		delayMicroseconds(500);
		digitalWrite(step_pin, LOW);
		delayMicroseconds(500);
	}
}

void move(int h_step, int v_step, int h_sixteenth_step, int v_sixteenth_step) {
	//* full step movement
	//setting step and direction
	setStep(false, false, false);
	if (h_step < 0) {
		digitalWrite(H_DIR_PIN, H_DIR_BACKWARD);
		h_step*=-1;
	} else {
		digitalWrite(H_DIR_PIN, H_DIR_FORWARD);
	}
	if (v_step < 0) {
		digitalWrite(V_DIR_PIN, V_DIR_BACKWARD);
		v_step*=-1;
	} else {
		digitalWrite(V_DIR_PIN, V_DIR_FORWARD);
	}
	//signaling to step both
	int limit = min(h_step, v_step);
	for (int i = 0; i < limit; i++) {
		digitalWrite(H_STEP_PIN, HIGH);
		digitalWrite(V_STEP_PIN, HIGH);
		delayMicroseconds(500);
		digitalWrite(H_STEP_PIN, LOW);
		digitalWrite(V_STEP_PIN, LOW);
		delayMicroseconds(500);
	}
	//signaling to step the one with steps left
	limit = max(h_step, v_step) - limit;
	int cur_step_pin = h_step > v_step ? H_STEP_PIN : V_STEP_PIN;
	for (int i = 0; i < limit; i++) {
		digitalWrite(cur_step_pin, HIGH);
		delayMicroseconds(500);
		digitalWrite(cur_step_pin, LOW);
		delayMicroseconds(500);
	}

	//* 16th step movement
	//setting step and direction
	setStep(true, true, true);
	if (h_sixteenth_step < 0) {
		digitalWrite(H_DIR_PIN, H_DIR_BACKWARD);
		h_sixteenth_step*=-1;
	} else {
		digitalWrite(H_DIR_PIN, H_DIR_FORWARD);
	}
	if (v_sixteenth_step < 0) {
		digitalWrite(V_DIR_PIN, V_DIR_BACKWARD);
		v_sixteenth_step*=-1;
	} else {
		digitalWrite(V_DIR_PIN, V_DIR_FORWARD);
	}
	//signaling to step both
	limit = min(h_sixteenth_step, v_sixteenth_step);
	for (int i = 0; i < limit; i++) {
		digitalWrite(H_STEP_PIN, HIGH);
		digitalWrite(V_STEP_PIN, HIGH);
		delayMicroseconds(1000);
		digitalWrite(H_STEP_PIN, LOW);
		digitalWrite(V_STEP_PIN, LOW);
		delayMicroseconds(1000);
	}
	//signaling to step the one with steps left
	limit = max(h_sixteenth_step, v_sixteenth_step) - limit;
	cur_step_pin = h_sixteenth_step > v_sixteenth_step ? H_STEP_PIN : V_STEP_PIN;
	step(cur_step_pin, limit);
}

bool getLimitSwitchActivated(int pin) {
  return digitalRead(pin) == HIGH;
}

void apparatusCommandListener(String command) {
  if (command.startsWith("aMove")) {//ex. 'aMove 16,3;17,2'
    String horizontal = command.substring(command.indexOf(' ') + 1, command.indexOf(';'));
    String vertical = command.substring(command.indexOf(';') + 1);
    int h_step = horizontal.substring(0, horizontal.indexOf(',')).toInt();
    int h_sixteenth_step = horizontal.substring(horizontal.indexOf(',') + 1).toInt();
    int v_step = vertical.substring(0, horizontal.indexOf(',')).toInt();
    int v_sixteenth_step = vertical.substring(horizontal.indexOf(',') + 1).toInt();
    
    move(h_step, v_step, h_sixteenth_step, v_sixteenth_step);
  }
  else if (command.startsWith("aReset")) {
    setStep(true,true,true);
  	//Fast Horizontal Reset
    digitalWrite(H_DIR_PIN, H_DIR_BACKWARD);//TODO: set correct direction
    while (false) (!getLimitSwitchActivated(H_LIMIT_PIN)) {
      step(H_STEP_PIN, 1);
    }
  	//Fast Vertical Reset
    digitalWrite(V_DIR_PIN, V_DIR_FORWARD);//TODO: set correct direction
    while(!getLimitSwitchActivated(V_LIMIT_PIN)) {
		step(V_STEP_PIN, 1);
    }
    //Taking a Step Back
    digitalWrite(H_DIR_PIN, H_DIR_FORWARD);//TODO: set correct dirrection
    digitalWrite(V_DIR_PIN, V_DIR_BACKWARD);
    step(H_STEP_PIN, 100);
    step(V_STEP_PIN, 100);
    //Slow Horizontal Reset
    digitalWrite(H_DIR_PIN, H_DIR_BACKWARD);//TODO: set correct direction
     while (false) (!getLimitSwitchActivated(H_LIMIT_PIN)) {
       step(H_STEP_PIN, 1);
       delay(5);
     }
     //Slow Vertical Reset
     digitalWrite(V_DIR_PIN, V_DIR_FORWARD);//TODO: set correct direction
     while(!getLimitSwitchActivated(V_LIMIT_PIN)) {
       step(V_STEP_PIN, 1);
       delay(5);
     }
  }
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    if (command.startsWith("p")) penCommandListener(command);
    else if (command.startsWith("a")) apparatusCommandListener(command);
    else if (command.startsWith("info")) {
      Serial.println("Upper servo setpoint: " + penUpSP);
      Serial.println("Lower servo setpoint: " + penUpSP);
    }
    Serial.println("executed: " + command);
  }
}

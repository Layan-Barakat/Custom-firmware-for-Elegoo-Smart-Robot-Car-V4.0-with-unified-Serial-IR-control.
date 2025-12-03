/*
  Custom Elegoo Smart Robot Car V4.0 firmware
  - Uses IRremote 2.6.0
  - Control from:
      * PC Serial: W/A/S/D/X, 1/2/3 for speed, +/- fine adjust
      * IR Remote: arrows + OK + 1/2/3
  - Forward/backward: continuous motion until stopped
  - Left/right: short step turns (single press = one small turn)
*/

#include <IRremote.h>

// ====== Pin mapping (from DeviceDriverSet_xxx0.h) ======
const int STBY = 3;   // Motor driver standby

// Motor A = RIGHT wheel
const int PWMA = 5;
const int AIN1 = 7;

// Motor B = LEFT wheel
const int PWMB = 6;
const int BIN1 = 8;

// IR receiver
const int IR_PIN = 9;

// ====== IR codes from Elegoo header (support A + B remotes) ======
const unsigned long IR_UP_A      = 16736925;
const unsigned long IR_DOWN_A    = 16754775;
const unsigned long IR_LEFT_A    = 16720605;
const unsigned long IR_RIGHT_A   = 16761405;
const unsigned long IR_OK_A      = 16712445;
const unsigned long IR_1_A       = 16738455;
const unsigned long IR_2_A       = 16750695;
const unsigned long IR_3_A       = 16756815;

const unsigned long IR_UP_B      = 5316027;
const unsigned long IR_DOWN_B    = 2747854299;
const unsigned long IR_LEFT_B    = 1386468383;
const unsigned long IR_RIGHT_B   = 553536955;
const unsigned long IR_OK_B      = 3622325019;
const unsigned long IR_1_B       = 3238126971;
const unsigned long IR_2_B       = 2538093563;
const unsigned long IR_3_B       = 4039382595;

// ====== Movement / speed config ======
const int SPEED_LEVELS[3] = {120, 180, 230};  // slow, medium, fast
int currentSpeedLevel = 1;                    // start at medium
int motorSpeed = SPEED_LEVELS[currentSpeedLevel];

// Step time only for turns (half of old 300 ms)
const unsigned long TURN_STEP_TIME_MS = 150;

// State for auto-stop (only for turns)
bool motionActive = false;
unsigned long motionEndTime = 0;

// IRremote v2.x objects
IRrecv irrecv(IR_PIN);
decode_results results;

// ====== Low-level motor helpers ======
void setMotorA(int speed, int dir) {
  // dir: 1 = forward, -1 = backward, 0 = stop
  speed = constrain(speed, 0, 255);

  if (dir == 0 || speed == 0) {
    analogWrite(PWMA, 0);
  } else if (dir > 0) {
    digitalWrite(AIN1, HIGH);
    analogWrite(PWMA, speed);
  } else {
    digitalWrite(AIN1, LOW);
    analogWrite(PWMA, speed);
  }
}

void setMotorB(int speed, int dir) {
  speed = constrain(speed, 0, 255);

  if (dir == 0 || speed == 0) {
    analogWrite(PWMB, 0);
  } else if (dir > 0) {
    digitalWrite(BIN1, HIGH);
    analogWrite(PWMB, speed);
  } else {
    digitalWrite(BIN1, LOW);
    analogWrite(PWMB, speed);
  }
}

void stopMotors() {
  setMotorA(0, 0);
  setMotorB(0, 0);
  motionActive = false;
}

// ====== High-level movement functions ======
// Continuous forward / backward
void startForwardContinuous() {
  digitalWrite(STBY, HIGH);
  setMotorA(motorSpeed, 1);
  setMotorB(motorSpeed, 1);
  motionActive = false;  // do NOT auto-stop
}

void startBackwardContinuous() {
  digitalWrite(STBY, HIGH);
  setMotorA(motorSpeed, -1);
  setMotorB(motorSpeed, -1);
  motionActive = false;  // do NOT auto-stop
}

// Step turns (single press = one small turn)
void stepTurnLeft() {
  digitalWrite(STBY, HIGH);
  // Pivot: right forward, left backward
  setMotorA(motorSpeed, 1);
  setMotorB(motorSpeed, -1);
  motionActive = true;
  motionEndTime = millis() + TURN_STEP_TIME_MS;
}

void stepTurnRight() {
  digitalWrite(STBY, HIGH);
  // Pivot: right backward, left forward
  setMotorA(motorSpeed, -1);
  setMotorB(motorSpeed, 1);
  motionActive = true;
  motionEndTime = millis() + TURN_STEP_TIME_MS;
}

// ====== Speed helpers ======
void applySpeedLevel() {
  motorSpeed = SPEED_LEVELS[currentSpeedLevel];
  Serial.print("Speed level ");
  Serial.print(currentSpeedLevel + 1);
  Serial.print(" (");
  Serial.print(motorSpeed);
  Serial.println(")");
}

void nextSpeedLevel() {
  if (currentSpeedLevel < 2) {
    currentSpeedLevel++;
    applySpeedLevel();
  }
}

void prevSpeedLevel() {
  if (currentSpeedLevel > 0) {
    currentSpeedLevel--;
    applySpeedLevel();
  }
}

// ====== Command handlers ======
void handlePCCommand(char c) {
  switch (c) {
    case 'w': case 'W':
      startForwardContinuous();
      Serial.println("PC: Forward (continuous)");
      break;
    case 's': case 'S':
      startBackwardContinuous();
      Serial.println("PC: Backward (continuous)");
      break;
    case 'a': case 'A':
      stepTurnLeft();
      Serial.println("PC: Left (step)");
      break;
    case 'd': case 'D':
      stepTurnRight();
      Serial.println("PC: Right (step)");
      break;
    case 'x': case 'X':
      stopMotors();
      Serial.println("PC: Stop");
      break;
    case '+':
      motorSpeed = min(motorSpeed + 10, 255);
      Serial.print("PC: Speed fine + -> ");
      Serial.println(motorSpeed);
      break;
    case '-':
      motorSpeed = max(motorSpeed - 10, 0);
      Serial.print("PC: Speed fine - -> ");
      Serial.println(motorSpeed);
      break;
    case '1':
      currentSpeedLevel = 0;
      applySpeedLevel();
      break;
    case '2':
      currentSpeedLevel = 1;
      applySpeedLevel();
      break;
    case '3':
      currentSpeedLevel = 2;
      applySpeedLevel();
      break;
    default:
      // ignore
      break;
  }
}

bool matchesCode(unsigned long value,
                 unsigned long a, unsigned long b) {
  return (value == a || value == b);
}

void handleIRCommand(unsigned long code) {
  if (matchesCode(code, IR_UP_A, IR_UP_B)) {
    startForwardContinuous();
    Serial.println("IR: Forward (continuous)");
  } else if (matchesCode(code, IR_DOWN_A, IR_DOWN_B)) {
    startBackwardContinuous();
    Serial.println("IR: Backward (continuous)");
  } else if (matchesCode(code, IR_LEFT_A, IR_LEFT_B)) {
    stepTurnLeft();
    Serial.println("IR: Left (step)");
  } else if (matchesCode(code, IR_RIGHT_A, IR_RIGHT_B)) {
    stepTurnRight();
    Serial.println("IR: Right (step)");
  } else if (matchesCode(code, IR_OK_A, IR_OK_B)) {
    stopMotors();
    Serial.println("IR: Stop");
  } else if (matchesCode(code, IR_1_A, IR_1_B)) {
    currentSpeedLevel = 0;
    applySpeedLevel();
  } else if (matchesCode(code, IR_2_A, IR_2_B)) {
    currentSpeedLevel = 1;
    applySpeedLevel();
  } else if (matchesCode(code, IR_3_A, IR_3_B)) {
    currentSpeedLevel = 2;
    applySpeedLevel();
  } else {
    Serial.print("IR: Unknown code ");
    Serial.println(code);
  }
}

// ====== Arduino setup / loop ======
void setup() {
  pinMode(STBY, OUTPUT);

  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);

  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);

  digitalWrite(STBY, HIGH);   // enable driver
  stopMotors();

  Serial.begin(9600);
  Serial.println("Custom Elegoo V4 firmware with IR + Serial");
  Serial.println("PC: W/A/S/D/x (W/S continuous, A/D step), +/- fine speed, 1/2/3 speed levels");
  Serial.println("Remote: arrows (up/down continuous, left/right step), OK stop, 1/2/3 speed levels");
  Serial.println();

  irrecv.enableIRIn();        // start IR receiver
  applySpeedLevel();
}

void loop() {
  // 1) Handle PC serial input
  if (Serial.available() > 0) {
    char c = Serial.read();
    handlePCCommand(c);
  }

  // 2) Handle IR remote input
  if (irrecv.decode(&results)) {
    handleIRCommand(results.value);
    irrecv.resume();
  }

  // 3) Auto-stop after turn step duration
  if (motionActive && millis() > motionEndTime) {
    stopMotors();
    // Serial.println("Auto stop (turn)");
  }
}

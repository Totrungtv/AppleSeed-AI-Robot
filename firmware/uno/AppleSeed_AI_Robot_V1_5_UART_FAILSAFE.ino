/*
================================================
       APPLE SEED AI ROBOT V1.2
       MAIN CONTROL SYSTEM
================================================
*/

#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// ==============================================
// MODULE CONFIGURATION
// ==============================================

LiquidCrystal_I2C lcd(0x27, 16, 2);

RTC_DS3231 rtc;

Servo scanner;

// ==============================================
// PIN CONFIGURATION
// ==============================================

#define MOTOR_IN1 5
#define MOTOR_IN2 6
#define MOTOR_IN3 7
#define MOTOR_IN4 8

#define SERVO_PIN 3

#define TRIG_PIN 9
#define ECHO_PIN 10

#define RAIN_PIN A0
#define BATTERY_PIN A1

// ==============================================
// SETTINGS
// ==============================================

#define RAIN_THRESHOLD 500

#define OBSTACLE_DISTANCE 25

#define BATTERY_DIVIDER 2.0

#define ADC_REFERENCE 5.0

// ==============================================
// SYSTEM VARIABLES
// ==============================================

bool rtcOK = false;
bool rainDetected = false;
bool obstacleDetected = false;

float distanceCM = 0;
float batteryVoltage = 0;

int batteryPercent = 0;

unsigned long lastDisplay = 0;

int screenMode = 0;

const unsigned long DISPLAY_INTERVAL = 3000;

// Dieu khien tu ESP32: mac dinh MANUAL + STOP de robot khong tu chay khi khoi dong.
enum RobotMode { MODE_MANUAL, MODE_AUTO };
RobotMode robotMode = MODE_MANUAL;
char manualMotion = 'S';
String uartCommand;
unsigned long lastManualCommandMs = 0;
const unsigned long MANUAL_FAILSAFE_MS = 700; // Tu dung neu mat heartbeat tu ESP32

// Servo quet trai-phai khi robot di chuyen
int scanAngle = 90;
int scanDirection = 1;
unsigned long lastServoScan = 0;
const unsigned long SERVO_SCAN_INTERVAL = 250;

// ==============================================
// MOTOR CONTROL
// ==============================================

void stopMotor() {

  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);

  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, LOW);

}

void forward() {

  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);

  digitalWrite(MOTOR_IN3, HIGH);
  digitalWrite(MOTOR_IN4, LOW);

}

void backward() {

  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, HIGH);

  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, HIGH);

}

void turnLeft() {

  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, HIGH);

  digitalWrite(MOTOR_IN3, HIGH);
  digitalWrite(MOTOR_IN4, LOW);

}

void turnRight() {

  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);

  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, HIGH);

}

// ==============================================
// ULTRASONIC SENSOR
// ==============================================

float readDistance() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(
    ECHO_PIN,
    HIGH,
    25000
  );

  if (duration == 0) {
    return -1;
  }

  return duration * 0.0343 / 2.0;

}

// ==============================================
// SERVO SCANNING
// ==============================================

float scanLeft() {

  scanner.write(150);
  delay(400);

  return readDistance();

}

float scanRight() {

  scanner.write(30);
  delay(400);

  return readDistance();

}

void centerServo() {

  scanAngle = 90;
  scanDirection = 1;
  scanner.write(90);
  delay(200);

}

// Quet dau robot qua lai khi dang di chuyen.
void updateServoScan() {

  if (millis() - lastServoScan < SERVO_SCAN_INTERVAL) {
    return;
  }

  lastServoScan = millis();
  scanAngle += scanDirection * 15;

  if (scanAngle >= 135) {
    scanAngle = 135;
    scanDirection = -1;
  }
  else if (scanAngle <= 45) {
    scanAngle = 45;
    scanDirection = 1;
  }

  scanner.write(scanAngle);
}

// ==============================================
// RAIN SENSOR
// ==============================================

void checkRain() {

  int rainValue = analogRead(RAIN_PIN);

  rainDetected = (
    rainValue < RAIN_THRESHOLD
  );

}

// ==============================================
// BATTERY MONITOR 2S
// ==============================================

int calculateBatteryPercent(float voltage) {

  // Muc 8.2V duoc dat lam 100% theo dien ap pin
  // thuc te da do sau khi sac day.
  if (voltage >= 8.2) return 100;
  if (voltage <= 6.8) return 0;

  const float voltageTable[] = {
    6.8, 7.2, 7.4, 7.6,
    7.8, 8.0, 8.2
  };

  const int percentTable[] = {
    0, 10, 25, 40,
    60, 80, 100
  };

  for (int i = 0; i < 6; i++) {

    if (
      voltage >= voltageTable[i] &&
      voltage <= voltageTable[i + 1]
    ) {

      float ratio =
        (voltage - voltageTable[i]) /
        (voltageTable[i + 1] -
         voltageTable[i]);

      return (int)(percentTable[i] +
        ratio *
        (percentTable[i + 1] -
         percentTable[i]) + 0.5);

    }

  }

  return 0;

}

void checkBattery() {

  // Lay nhieu mau de giam nhay so do nhieu va tai motor.
  const int SAMPLE_COUNT = 30;
  unsigned long total = 0;

  for (int i = 0; i < SAMPLE_COUNT; i++) {
    total += analogRead(BATTERY_PIN);
    delay(3);
  }

  float raw = total / (float)SAMPLE_COUNT;

  float measuredVoltage =
    raw * (ADC_REFERENCE / 1023.0);

  batteryVoltage =
    measuredVoltage * BATTERY_DIVIDER;

  batteryPercent =
    calculateBatteryPercent(batteryVoltage);

}

// ==============================================
// BATTERY STATUS
// ==============================================

const char* getBatteryStatus() {

  if (batteryVoltage >= 8.1) {
    return "FULL";
  }

  if (batteryVoltage >= 7.4) {
    return "MEDIUM";
  }

  if (batteryVoltage >= 6.8) {
    return "LOW";
  }

  return "CRITICAL";

}

// ==============================================
// LCD STARTUP
// ==============================================

void displayStartup() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("APPLE SEED AI");

  lcd.setCursor(0, 1);
  lcd.print("SYSTEM STARTING");

  delay(2000);

}

// ==============================================
// DISPLAY CLOCK
// ==============================================

void displayClock() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("APPLE SEED AI");

  lcd.setCursor(0, 1);

  if (rtcOK) {

    DateTime now = rtc.now();

    char buffer[17];

    sprintf(
      buffer,
      "%02d/%02d %02d:%02d",
      now.day(),
      now.month(),
      now.hour(),
      now.minute()
    );

    lcd.print(buffer);

  } else {

    lcd.print("ERROR: RTC");

  }

}

// ==============================================
// DISPLAY BATTERY
// ==============================================

void displayBattery() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("BATTERY STATUS");

  lcd.setCursor(0, 1);

  lcd.print(batteryVoltage, 1);
  lcd.print("V ");

  lcd.print(batteryPercent);
  lcd.print("% ");

  lcd.print(getBatteryStatus());

}

// ==============================================
// DISPLAY SENSOR
// ==============================================

void displaySensorStatus() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("DISTANCE:");

  if (distanceCM < 0) {
    lcd.print("ERROR");
  } else {
    lcd.print(distanceCM, 0);
    lcd.print("cm");
  }

  lcd.setCursor(0, 1);

  if (rainDetected) {
    lcd.print("RAIN DETECTED");
  } else {
    lcd.print("WEATHER NORMAL");
  }

}

// ==============================================
// WARNING SYSTEM
// ==============================================

void displayWarning() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("APPLE SEED AI");

  lcd.setCursor(0, 1);

  if (!rtcOK) {

    lcd.print("ERROR: RTC");

  }

  else if (rainDetected) {

    lcd.print("WARNING: RAIN!");

  }

  else if (batteryVoltage <= 6.8) {

    lcd.print("BATTERY CRITICAL");

  }

  else if (obstacleDetected) {

    lcd.print("OBSTACLE!");

  }

}

// ==============================================
// OBSTACLE AVOIDANCE
// ==============================================

void avoidObstacle() {

  stopMotor();
  displayWarning();
  delay(300);

  // Lui lai de tao khoang trong.
  backward();
  delay(500);
  stopMotor();
  delay(200);

  // Quet trai va phai, chon ben co duong rong hon.
  float leftDistance = scanLeft();
  float rightDistance = scanRight();
  centerServo();

  if (leftDistance > rightDistance &&
      leftDistance > OBSTACLE_DISTANCE) {
    turnLeft();
    delay(550);
  }
  else if (rightDistance >= leftDistance &&
           rightDistance > OBSTACLE_DISTANCE) {
    turnRight();
    delay(550);
  }
  else {
    // Hai ben deu bi chan: quay dau gan 180 do.
    turnRight();
    delay(900);
  }

  stopMotor();
  delay(200);
  centerServo();

  distanceCM = readDistance();

  if (distanceCM >= 0 && distanceCM < OBSTACLE_DISTANCE) {
    stopMotor();
  }

}

// ==============================================
// UART COMMANDS FROM ESP32 (9600 baud)
// A= AUTO, M= MANUAL, F/B/L/R/U/S= movement
// ==============================================
void processEsp32Command(char cmd) {
  if (cmd == 'H') {
    // Heartbeat: ESP32 gui lap lai khi nut dieu khien dang duoc giu.
    if (robotMode == MODE_MANUAL && manualMotion != 'S') lastManualCommandMs = millis();
  } else if (cmd == 'A') {
    robotMode = MODE_AUTO;
    manualMotion = 'S';
    lastManualCommandMs = millis();
    stopMotor();
  } else if (cmd == 'M') {
    robotMode = MODE_MANUAL;
    manualMotion = 'S';
    lastManualCommandMs = millis();
    stopMotor();
  } else if (cmd == 'S') {
    manualMotion = 'S';
    stopMotor();
  } else if (cmd == 'F' || cmd == 'B' || cmd == 'L' || cmd == 'R' || cmd == 'U') {
    robotMode = MODE_MANUAL;
    manualMotion = cmd;
    lastManualCommandMs = millis();
  }
}

void readEsp32Commands() {
  while (Serial.available() > 0) {
    char ch = Serial.read();
    if (ch == '\n' || ch == '\r') {
      // Chi chap nhan dong lenh dung 1 ky tu, bo qua dong debug.
      if (uartCommand.length() == 1) {
        processEsp32Command(uartCommand.charAt(0));
      }
      uartCommand = "";
    } else if (uartCommand.length() < 8) {
      uartCommand += ch;
    } else {
      uartCommand = "";
    }
  }
}

void runManualMotion() {
  // Failsafe: mat heartbeat trong 700ms thi dung motor.
  if (manualMotion != 'S' && millis() - lastManualCommandMs > MANUAL_FAILSAFE_MS) {
    manualMotion = 'S';
    stopMotor();
    return;
  }
  // Safety: stop forward movement if obstacle is detected.
  if (manualMotion == 'F' && obstacleDetected) { stopMotor(); return; }
  switch (manualMotion) {
    case 'F': forward(); break;
    case 'B': backward(); break;
    case 'L': turnLeft(); break;
    case 'R': turnRight(); break;
    case 'U': turnRight(); delay(900); stopMotor(); manualMotion = 'S'; break;
    default: stopMotor(); break;
  }
}

unsigned long lastTelemetry = 0;
const unsigned long TELEMETRY_INTERVAL = 1000;
void sendTelemetryToESP32() {
  if (millis() - lastTelemetry < TELEMETRY_INTERVAL) return;
  lastTelemetry = millis();
  Serial.print("T,"); Serial.print(batteryVoltage, 2); Serial.print(",");
  Serial.print(batteryPercent); Serial.print(","); Serial.print(distanceCM, 1);
  Serial.print(","); Serial.print(rainDetected ? 1 : 0); Serial.print(",");
  Serial.println(obstacleDetected ? 1 : 0);
}

// SETUP
// ==============================================

void setup() {

  Serial.begin(9600);
  lastManualCommandMs = millis();

  Wire.begin();

  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);

  pinMode(MOTOR_IN3, OUTPUT);
  pinMode(MOTOR_IN4, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(RAIN_PIN, INPUT);
  pinMode(BATTERY_PIN, INPUT);

  stopMotor();

  scanner.attach(SERVO_PIN);

  centerServo();

  lcd.init();
  lcd.backlight();

  displayStartup();

  rtcOK = rtc.begin();

  if (rtcOK && rtc.lostPower()) {

    rtc.adjust(
      DateTime(F(__DATE__), F(__TIME__))
    );

  }

  displayClock();

}

// ==============================================
// MAIN LOOP
// ==============================================

void loop() {

  readEsp32Commands();
  checkRain();
  checkBattery();
  distanceCM = readDistance();
  obstacleDetected = (distanceCM >= 0 && distanceCM < OBSTACLE_DISTANCE);

  // Gui telemetry dung dinh dang ESP32 V2.9 dang doc.
  sendTelemetryToESP32();

  // Loi an toan luon uu tien, bat ke AUTO hay MANUAL.
  if (!rtcOK || batteryVoltage <= 6.8 || rainDetected) {
    stopMotor();
    displayWarning();
    delay(100);
    return;
  }

  if (robotMode == MODE_AUTO) {
    if (obstacleDetected) {
      avoidObstacle();
    } else if (distanceCM < 0) {
      stopMotor();
      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("APPLE SEED AI");
      lcd.setCursor(0, 1); lcd.print("SENSOR ERROR");
    } else {
      updateServoScan();
      forward();
    }
  } else {
    runManualMotion();
  }

  if (millis() - lastDisplay >= DISPLAY_INTERVAL) {
    lastDisplay = millis();
    screenMode++;
    if (screenMode > 2) screenMode = 0;
    if (screenMode == 0) displayClock();
    else if (screenMode == 1) displayBattery();
    else displaySensorStatus();
  }
}

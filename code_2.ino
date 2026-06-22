/*
  Color Sorting System with Servo and LDR
  (Based on original example by Tom Igoe & Arturo Guadalupi)
*/

#include <LiquidCrystal.h>
#include <Servo.h>

// LCD Pins
const int lcdRS = 2, lcdEN = 3, lcdD4 = 4, lcdD5 = 8, lcdD6 = 12, lcdD7 = 13;
LiquidCrystal lcd(lcdRS, lcdEN, lcdD4, lcdD5, lcdD6, lcdD7);

// Servo setup
Servo armServo;
Servo ejectServo;

// LDR sensor values
int redLDR = 0;
int greenLDR = 0;
int blueLDR = 0;
int baseRedLDR = 0;
int baseGreenLDR = 0;
int baseBlueLDR = 0;

// LED intensity calibration
int redIntensity = 245;
int greenIntensity = 245;
int blueIntensity = 245;
int maxLDRValue = 0;
int calibrationFlag = 1;

// LED pins
int redLED = 11;
int greenLED = 10;
int blueLED = 9;

// other variables
int calibrationMode = 0;
int targetColor = 0;
int detectedColor = -1;
int electromagnetPin = 7;

void setup() {
  Serial.begin(9600);
  
  armServo.attach(5);
  armServo.write(160);
  
  ejectServo.attach(6);
  ejectServo.write(0);
  
  delay(100);

  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
  pinMode(electromagnetPin, OUTPUT);
  pinMode(A2, INPUT);

  lcd.begin(16, 2);
}

void autoCalibrate() {
  while (calibrationFlag == 1) {
    analogWrite(redLED, redIntensity);
    delay(150);
    redLDR = analogRead(A2);
    analogWrite(redLED, 0);

    analogWrite(greenLED, greenIntensity);
    delay(150);
    greenLDR = analogRead(A2);
    analogWrite(greenLED, 0);

    analogWrite(blueLED, blueIntensity);
    delay(150);
    blueLDR = analogRead(A2);
    analogWrite(blueLED, 0);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("R:");
    lcd.print(redLDR);
    lcd.print(" G:");
    lcd.print(greenLDR);

    lcd.setCursor(0, 1);
    lcd.print("B:");
    lcd.print(blueLDR);
    lcd.display();
    delay(100);

    if (calibrationFlag == 1) {
      if (redLDR > greenLDR && redLDR > blueLDR) {
        maxLDRValue = redLDR;
      } else if (greenLDR > blueLDR) {
        maxLDRValue = greenLDR;
      } else {
        maxLDRValue = blueLDR;
      }

      if (maxLDRValue - redLDR > 20) redIntensity -= 1;
      if (maxLDRValue - greenLDR > 20) greenIntensity -= 1;
      if (maxLDRValue - blueLDR > 20) blueIntensity -= 1;

      Serial.print("R Intensity: "); Serial.println(redIntensity);
      Serial.print("G Intensity: "); Serial.println(greenIntensity);
      Serial.print("B Intensity: "); Serial.println(blueIntensity);

      if ((maxLDRValue - redLDR < 20) && (maxLDRValue - greenLDR < 20) && (maxLDRValue - blueLDR < 20)) {
        calibrationFlag = 0;
        Serial.println("Auto Calibration Complete!");
      }
    }
  }
}

void manualCalibrate() {
  while (calibrationFlag == 1) {
    readLDR();
    if (calibrationFlag == 1) {
      if (redLDR > greenLDR && redLDR > blueLDR) {
        maxLDRValue = redLDR;
      } else if (greenLDR > blueLDR) {
        maxLDRValue = greenLDR;
      } else {
        maxLDRValue = blueLDR;
      }

      Serial.print("R Intensity: "); Serial.println(redIntensity);
      Serial.print("G Intensity: "); Serial.println(greenIntensity);
      Serial.print("B Intensity: "); Serial.println(blueIntensity);

      if ((maxLDRValue - redLDR < 60) && (maxLDRValue - greenLDR < 60) && (maxLDRValue - blueLDR < 60)) {
        calibrationFlag = 0;
        Serial.println("Manual Calibration Complete!");
      }
    }
  }

  baseRedLDR = redLDR;
  baseGreenLDR = greenLDR;
  baseBlueLDR = blueLDR;
}

void readLDR() {
  digitalWrite(redLED, HIGH);
  delay(150);
  redLDR = analogRead(A2);
  digitalWrite(redLED, LOW);

  digitalWrite(greenLED, HIGH);
  delay(150);
  greenLDR = analogRead(A2);
  digitalWrite(greenLED, LOW);

  digitalWrite(blueLED, HIGH);
  delay(150);
  blueLDR = analogRead(A2);
  digitalWrite(blueLED, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("R:");
  lcd.print(redLDR);
  lcd.print(" G:");
  lcd.print(greenLDR);
  lcd.setCursor(0, 1);
  lcd.print("B:");
  lcd.print(blueLDR);
  lcd.display();
  delay(100);
}

void reachColor() {
  int servoPos = 160;
  int successCount = 0;
  calibrationFlag = 1;

  for (servoPos = 160; servoPos >= 0; servoPos -= 5) {
    armServo.write(servoPos);
    delay(700);
    readLDR();
    Serial.print("Servo Position: ");
    Serial.println(servoPos);

    int minLDR = min(min(redLDR, greenLDR), blueLDR);
    int sensitivity = 50;

    if (((minLDR == redLDR && greenLDR - baseGreenLDR > sensitivity && blueLDR - baseBlueLDR > sensitivity) && (servoPos >= 40 && servoPos <= 50)) || (servoPos >= 40 && servoPos <= 50)) {
      Serial.println("Detected: RED");
      detectedColor = 0;
      if (targetColor == detectedColor) {
        successCount++;
        if (successCount == 2) break;
      }
    }

    if (((minLDR == greenLDR && redLDR - baseRedLDR > sensitivity && blueLDR - baseBlueLDR > sensitivity) && (servoPos >= 0 && servoPos <= 7)) || (servoPos >= 0 && servoPos <= 7)) {
      Serial.println("Detected: GREEN");
      detectedColor = 1;
      if (targetColor == detectedColor) {
        successCount++;
        if (successCount == 2) break;
      }
    }

    if (((minLDR == blueLDR && redLDR - baseRedLDR > sensitivity && greenLDR - baseGreenLDR > sensitivity) && (servoPos <= 110 && servoPos >= 97)) || (servoPos <= 110 && servoPos >= 97)) {
      Serial.println("Detected: BLUE");
      detectedColor = 2;
      if (targetColor == detectedColor) {
        successCount++;
        if (successCount == 2) break;
      }
    }
  }
}

void loop() {
  // Calibration
  Serial.println("Enter 0 for Manual Calibration or 1 for Auto Calibration:");
  while (Serial.available() == 0) {}

  while (Serial.available() > 0) {
    calibrationMode = Serial.parseInt();
    if (calibrationMode == 1) {
      Serial.println("Starting Auto Calibration...");
      autoCalibrate();
    } else if (calibrationMode == 0) {
      Serial.println("Starting Manual Calibration...");
      manualCalibrate();
    }
  }

  // Color Pickup
  Serial.println("Pick Up: Enter 0 (Red), 1 (Green), or 2 (Blue):");
  while (Serial.available() == 0) {}

  while (Serial.available() > 0) {
    targetColor = Serial.parseInt();
  }

  reachColor();
  Serial.println("Target Color Position Reached");
  ejectServo.write(35);
  digitalWrite(electromagnetPin, HIGH);
  delay(2000);

  ejectServo.write(0);
  delay(2000);

  // Drop
  Serial.println("Drop: Enter 0 (Red), 1 (Green), or 2 (Blue):");
  while (Serial.available() == 0) {}

  while (Serial.available() > 0) {
    targetColor = Serial.parseInt();
  }

  reachColor();
  Serial.println("Target Drop Position Reached");
  ejectServo.write(30);
  delay(2000);
  digitalWrite(electromagnetPin, LOW);
  delay(1000);
  ejectServo.write(0);

  Serial.println("Process Complete. Have a Nice Day!");
  delay(1000);
}
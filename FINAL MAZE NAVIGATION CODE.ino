//***********************************//
// BENCH 11                          //
// UNI OF NOTTINGHAM EEE             //
// MAZE NAV PROGRAM                  //
//***********************************//

#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <Keypad.h>

//IMU GYROSCOPE
#include <MPU6050_tockn.h>
MPU6050 mpu6050(Wire);

#define I2C_SLAVE_ADDR 0x04
#define i2c_Address 0x3c

#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
long timer = 0;
int leftSpeed;
int rightSpeed;
float rightDistance;
float leftDistance;
int steeringAngle;
long receivedValue1;
long receivedValue2;
byte receivedByte1;
byte receivedByte2;


int rightMotor, leftMotor, angle;


const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { '*', '0', '#' }
};

LiquidCrystal lcd(13, 12, 14, 27, 26, 25);
byte rowPins[ROWS] = { 15, 2, 4, 16 };
byte colPins[COLS] = { 17, 5, 18 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

int menu = -1;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  lcd.begin(16, 2);
  mpu6050.calcGyroOffsets(true);

  //HMI MENU / FD = FORWARDS / BD = BACKWARDS / LT = LEFT / RT = RIGHT
  lcd.print("FD = 2");
  lcd.print(" | BD = 5");
  lcd.setCursor(0, 1);
  lcd.print("LT = 4");
  lcd.print(" | RT = 6");
}

void moveCar(int leftMotor, int rightMotor, int angle) {
  Wire.beginTransmission(I2C_SLAVE_ADDR);
  Wire.write((byte)((leftMotor & 0x0000FF00) >> 8));
  Wire.write((byte)(leftMotor & 0x000000FF));
  Wire.write((byte)((rightMotor & 0x0000FF00) >> 8));
  Wire.write((byte)(rightMotor & 0x000000FF));
  Wire.write((byte)((angle & 0x0000FF00) >> 8));
  Wire.write((byte)(angle & 0x000000FF));
  Wire.endTransmission();
  delay(550);
}

void getIMUdata(void) {
  mpu6050.update();
  {
    Serial.println("=======================================================");
    Serial.print("gyroX : ");
    Serial.print(mpu6050.getGyroX());
    Serial.print("\tgyroY : ");
    Serial.print(mpu6050.getGyroY());
    Serial.print("\tgyroZ : ");
    Serial.println(mpu6050.getGyroZ());

    Serial.print("gyroAngleX : ");
    Serial.print(mpu6050.getGyroAngleX());
    Serial.print("\tgyroAngleY : ");
    Serial.print(mpu6050.getGyroAngleY());
    Serial.print("\tgyroAngleZ : ");
    Serial.println(mpu6050.getGyroAngleZ());

    Serial.print("angleX : ");
    Serial.print(mpu6050.getAngleX());
    Serial.print("\tangleY : ");
    Serial.print(mpu6050.getAngleY());
    Serial.print("\tangleZ : ");
    Serial.println(mpu6050.getAngleZ());
    Serial.println("=======================================================\n");
  }
}

void GetEncoderData(void) {
  // Requesting encoder count from arduino
  Wire.requestFrom(I2C_SLAVE_ADDR, sizeof(byte) * 2);
  if (Wire.available() >= sizeof(byte) * 2)
  {
    // reading the data
    byte receivedByte1 = Wire.read();
    byte receivedByte2 = Wire.read();
    // converting the data to a distance
    float leftDistance = receivedByte1 * 0.7853981634;
    float rightDistance = receivedByte2 * 0.7853981634;
    // printing the data
    Serial.print("Left: ");
    Serial.print(leftDistance);
    Serial.print(" Right: ");
    Serial.println(rightDistance);
  }
}

char moves[20];       // Array to store user moves
int moveCount = 0;    // Counter for number of moves
int currentMove = 0;  // An Index for the current move being executed

void loop() {
  char key = keypad.getKey();
  if (key) {
    if (key == '*')  // Reset moves if * key is pressed
    {
      moveCount = 0;
      currentMove = 0;
      lcd.clear();
      lcd.print("Moves reset.");
      delay(500);
      lcd.clear();
    } else if (key == '#')  // Execute moves if # key is pressed
    {
      lcd.clear();
      for (int i = 0; i < moveCount; i++) {
        switch (moves[i]) {
          case '2':
            {
              lcd.print("Execute Forward");
              do 
              {
                leftDistance = 0;
                rightDistance = 0;
                moveCar(120, 120, 90);
                GetEncoderData();
              } while (leftDistance > 10);
              moveCar(0, 0, 90);
              delay(100);
            }
            break;

          case '4':
            {
              lcd.print("Execute Left");
              // Code to turn left
              mpu6050.update();
              double endangle = (mpu6050.getAngleZ() + 95);
              do 
              {
                getIMUdata();
                moveCar(150, 150, 45);  //rotate ccw 180 deg
              } while (mpu6050.getAngleZ() < endangle);
              moveCar(0, 0, 90);
              delay(100);
            }
            break;

          case '6':
            {
              lcd.print("Execute Right");
              // Code to turn right
              mpu6050.update();
              double endangle2 = (mpu6050.getAngleZ() - 120);
              do {
                moveCar(150, 150, 135);  //rotate cw 90 deg
                mpu6050.update();
              } while (mpu6050.getAngleZ() > endangle2);
              moveCar(0, 0, 90);
              delay(100);
            }
            break;

          case '5':
            {
              lcd.print("Execute Reverse");
              // Code to move back
              moveCar(-130, -130, 90);
              delay(200);
              moveCar(0, 0, 90);
              delay(100);
            }
            break;
        }
        lcd.clear();
      }
      moveCount = 0;
      currentMove = 0;
    } 
    else
    {
      lcd.clear();
      if (key == '2') {
        lcd.print("Forwards 10cm");
        moves[moveCount++] = key;  // Store move in array
    } else if (key == '4') {
        lcd.print("Turn Left");
        moves[moveCount++] = key;  // Store move in array
    } else if (key == '6') {
        lcd.print("Turn Right");
        moves[moveCount++] = key;  // Store move in array
    } else if (key == '5') {
        lcd.print("Reverse");
        moves[moveCount++] = key;  // Store move in array
     }
     else{}
    }
  }
}

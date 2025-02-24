#include <Servo.h>

// Khai báo chân cảm biến ánh sáng
const int ldrLeft = A1;    
const int ldrRight = A0;   
const int ldrTop = A4;     
const int ldrBottom = A5;  

// Khai báo servo
Servo servoX; 
Servo servoY; 

const int servoXPin = 10;  
const int servoYPin = 9; 

// Biến lưu góc của servo
int servoXAngle = 50;
int servoYAngle = 90;

// Ngưỡng sai lệch ánh sáng để điều chỉnh
const int threshold = 5;
const int angleChangeThreshold = 0;
const int nightThreshold = 50;  // Ngưỡng ánh sáng để xác định ban đêm

// Biến lọc tín hiệu
float filteredLeft = 0, filteredRight = 0, filteredTop = 0, filteredBottom = 0;
const float alpha = 0.1; // Hệ số lọc

// Hàm đọc trung bình động
int readSmooth(int pin, int samples = 10) {
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delay(5);
  }
  return sum / samples;
}

void setup() {
  pinMode(ldrLeft, INPUT);
  pinMode(ldrRight, INPUT);
  pinMode(ldrTop, INPUT);
  pinMode(ldrBottom, INPUT);

  servoX.attach(servoXPin);
  servoY.attach(servoYPin);
  
  servoX.write(servoXAngle);
  servoY.write(servoYAngle);

  Serial.begin(9600); 
}

void loop() {
  // Đọc giá trị từ cảm biến ánh sáng
  int leftRaw = readSmooth(ldrLeft);
  int rightRaw = readSmooth(ldrRight);
  int topRaw = readSmooth(ldrTop);
  int bottomRaw = readSmooth(ldrBottom);

  // Lọc giá trị
  filteredLeft =  leftRaw +22 ;
  filteredRight = rightRaw+20;
  filteredTop = topRaw+90;
  filteredBottom = alpha * bottomRaw + (1 - alpha) * filteredBottom ;

  int leftValue = (int)filteredLeft;
  int rightValue = (int)filteredRight;
  int topValue = (int)filteredTop;
  int bottomValue = (int)filteredBottom+170;

  // Tính tổng ánh sáng
  int totalLight = leftValue + rightValue + topValue + bottomValue;

  // Kiểm tra ban đêm
  bool isNight = (totalLight < nightThreshold);

  // In giá trị ra Serial Monitor
  Serial.print("Left: "); Serial.print(leftValue);
  Serial.print(" | Right: "); Serial.print(rightValue);
  Serial.print(" | Top: "); Serial.print(topValue);
  Serial.print(" | Bottom: "); Serial.print(bottomValue);
  Serial.print(" | Total Light: "); Serial.print(totalLight);
  //Serial.println(" | Servo X: "); Serial.print(servoXAngle);
  //Serial.println(" | Servo Y: "); Serial.print(servoYAngle);
  Serial.print(" | Is Night: "); Serial.println(isNight);


  // Nếu là ban đêm, đặt servo về vị trí nghỉ
  if (isNight) {
    servoXAngle = 90;  // Giữ servo trục X ở giữa
    servoYAngle = 20;  // Hạ xuống vị trí thấp
    servoX.write(servoXAngle);
    servoY.write(servoYAngle);
    Serial.println("Night mode: Servo returned to rest position.");
  } else {
    // Điều chỉnh theo trục X (Trái/Phải)
    if (abs(leftValue - rightValue) > threshold) {
      int newXAngle = servoXAngle;
      if (leftValue > rightValue) {
        newXAngle -= 1; // Xoay trái
      } else {
        newXAngle += 1; // Xoay phải
      }

      if (abs(newXAngle - servoXAngle) > angleChangeThreshold) {
        servoXAngle = constrain(newXAngle, 0, 90);
        servoX.write(servoXAngle);
      }
    }

    // Điều chỉnh theo trục Y (Lên/Xuống)
    if (abs(topValue - bottomValue) > threshold) {
      int newYAngle = servoYAngle;
      if (topValue > bottomValue) {
        newYAngle -= 1; // Xoay xuống
      } else {
        newYAngle += 1; // Xoay lên
      }

      if (abs(newYAngle - servoYAngle) > angleChangeThreshold) {
        servoYAngle = constrain(newYAngle, 50, 90);
        servoY.write(servoYAngle);
      }
    }
  }

  // Thời gian trễ để ổn định servo
  delay(100);
}

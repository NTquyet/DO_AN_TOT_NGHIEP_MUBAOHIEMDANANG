#include <TinyGPSPlus.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>

// ==================== CẤU HÌNH ====================
String SDT = "0365562281";  // Số điện thoại nhận SMS

TinyGPSPlus gps;
#define gpsSerial Serial2   // RX2=16, TX2=17
Adafruit_MPU6050 mpu;
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// MQ3 và buzzer
#define MQ3_PIN 34
#define BUZZER_PIN 25
float alcohol = 0.0;
float threshold = 4.3; // ppm
unsigned long lastBuzzerTime = 0;
const unsigned long buzzerCooldown = 3000;

// Ngưỡng ngã
const float FALL_Z_THRESHOLD = 0.5;  // Z < 0.5g → ngã
bool fallDetected = false;
unsigned long lastAccidentTime = 0;
const unsigned long COOLDOWN_TIME = 10000;

// Cảm biến VL53Lx (quai mũ)
float distance = 0;
const float DIST_THRESHOLD = 4;  // >4cm = chưa cài quai
unsigned long lastStrapCheck = 0;
const unsigned long STRAP_INTERVAL = 1000;
bool strapWarning = false;

// === BIẾN GỬI DỮ LIỆU CHO MATLAB (Serial1) ===
HardwareSerial matlabSerial(1);  // UART1: TX=18, RX=19
unsigned long lastPrint = 0;
float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float angleX = 0, angleY = 0, angleZ = 0;
unsigned long prevTime = 0;
float offsetX = 0, offsetY = 0, offsetZ = 0;
bool calibrated = false;
unsigned long calibStart = 0;

// === DỮ LIỆU MPU CHO CẢ HỆ THỐNG ===
sensors_event_t a, g, temp;

// ==================== HÀM HỖ TRỢ ====================

void SIMfeedback() {
  String feedback = "";
  unsigned long start = millis();
  while (millis() - start < 1500) {
    if (Serial.available()) feedback += (char)Serial.read();
  }
  if (feedback.length() > 0) Serial.println(">> SIM: " + feedback);
}

void sendAT(const String& cmd, int wait = 800) {
  Serial.println(cmd);
  delay(wait);
  SIMfeedback();
}

void SIMsetup() {
  Serial.println("=== KHOI DONG SIM768C ===");
  sendAT("AT");
  sendAT("ATE0");
  sendAT("AT+CSCS=\"GSM\"");
  sendAT("AT+CMGF=1");
  sendAT("AT+CNMI=2,2,0,0,0");
  sendAT("AT+CLIP=1");
  sendAT("AT+CSQ");
  sendAT("AT&W");
}

void sendAccidentSMS(float lat, float lng) {
  Serial.println("=== GUI SMS CANH BAO NGA XE ===");
  String msg = "CANH BAO: TAI NAN!\n";
  msg += "Toa do: Lat=" + String(lat, 6) + ", Lng=" + String(lng, 6) + "\n";
  msg += "Ban do: https://maps.google.com/?q=" + String(lat, 6) + "," + String(lng, 6) + "\n";

  if (gps.time.isValid() && gps.date.isValid()) {
    int h = gps.time.hour() + 7;
    int d = gps.date.day();
    if (h >= 24) { h -= 24; d++; }
    msg += "Thoi gian (VN): " + String(d) + "/" + String(gps.date.month()) + "/" + String(gps.date.year());
    msg += " " + String(h) + ":" + (gps.time.minute() < 10 ? "0" : "") + String(gps.time.minute());
  }

  Serial.print("AT+CMGS=\"" + SDT + "\"\r");
  unsigned long start = millis();
  bool promptFound = false;

  while (millis() - start < 5000) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '>') { promptFound = true; break; }
    }
  }

  if (!promptFound) {
    Serial.println("Loi: SIM khong tra ve '>'");
    return;
  }

  Serial.print(msg);
  delay(500);
  Serial.write(26);
  delay(3000);
  SIMfeedback();
  Serial.println(">>> SMS DA GUI!");
}

void printAccidentInfo() {
  Serial.println("========================================");
  Serial.println("=== PHAT HIEN NGA XE! ===");
  Serial.printf("Gia toc: X=%.2f, Y=%.2f, Z=%.2f g\n", a.acceleration.x, a.acceleration.y, a.acceleration.z);

  if (gps.location.isValid()) {
    Serial.printf("Toa do: %.6f, %.6f\n", gps.location.lat(), gps.location.lng());
    Serial.print("Link: https://maps.google.com/?q=");
    Serial.print(gps.location.lat(), 6);
    Serial.print(",");
    Serial.println(gps.location.lng(), 6);
  } else {
    Serial.println("GPS chua fix!");
  }
  Serial.println("========================================");
}

// ==================== MQ3 ====================
float readAlcoholPPM() {
  int raw = analogRead(MQ3_PIN);
  float voltage = (raw / 4095.0) * 3.3;
  float ppm = (voltage / 3.3) * 10.0;
  return ppm;
}

// ==================== VL53Lx ====================
float readHelmetDistance() {
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    return measure.RangeMilliMeter / 10.0; // mm → cm
  } else {
    return -1;
  }
}

// ==================== GỬI DỮ LIỆU CHO MATLAB (Serial1) ====================
// ==================== GỬI DỮ LIỆU CHO MATLAB ====================
void sendToMATLAB() {
  if (millis() - lastPrint < 100) return;
  lastPrint = millis();

  unsigned long now = millis();
  float dt = (now - prevTime) / 1000.0;
  prevTime = now;

  if (!calibrated && millis() - calibStart > 2000) {
    offsetX = GyroX; offsetY = GyroY; offsetZ = GyroZ;
    calibrated = true;
    Serial.println("MPU6050: Da hieu chuan gyro!"); // COM4: debug
  }

  if (calibrated) {
    angleX += (GyroX - offsetX) * dt;
    angleY += (GyroY - offsetY) * dt;
    angleZ += (GyroZ - offsetZ) * dt;
  }

  // GỬI RA COM6 (USB-TTL → MATLAB)
  matlabSerial.printf("ACC,%.2f,%.2f,%.2f\n", AccX, AccY, AccZ);

  // DEBUG: In ra COM4 để bạn kiểm tra
  Serial.printf("[COM4 DEBUG] ACC,%.2f,%.2f,%.2f\n", AccX, AccY, AccZ);
}
// ==================== SETUP ====================
void setup() {
  Serial.begin(9600);           // UART0: SIM768C
  delay(2000);
  SIMsetup();

  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);  // GPS
  Wire.begin(21, 22);           // I2C: MPU6050 + VL53L0X
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // === KHỞI TẠO Serial1 CHO MATLAB (TX=18, RX=19) ===
  matlabSerial.begin(115200, SERIAL_8N1, 19, 18);  // RX=19, TX=18

  delay(60000);  // Chờ MQ3 ổn định

  // MPU6050
  if (!mpu.begin()) {
    Serial.println("LOI: Khong tim thay MPU6050!");
    while (1) delay(10);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  Serial.println("MPU6050: OK");

  // Bắt đầu hiệu chuẩn Gyro
  calibStart = millis();
  prevTime = millis();

  // VL53Lx
  if (!lox.begin()) {
    Serial.println("LOI: Khong tim thay VL53Lx!");
    while (1) delay(10);
  }
  Serial.println("VL53Lx: OK");
}

// ==================== LOOP ====================
void loop() {
  // GPS
  while (gpsSerial.available() > 0) gps.encode(gpsSerial.read());

  // MPU6050: Đọc dữ liệu (dùng chung cho cả hệ thống + MATLAB)
  mpu.getEvent(&a, &g, &temp);
  AccX = a.acceleration.x;
  AccY = a.acceleration.y;
  AccZ = a.acceleration.z;
  GyroX = g.gyro.x * 57.3;  // rad/s → °/s
  GyroY = g.gyro.y * 57.3;
  GyroZ = g.gyro.z * 57.3;

  // Gửi dữ liệu cho MATLAB
  sendToMATLAB();

  // MQ3
  alcohol = readAlcoholPPM();
  Serial.print("Nong do con: ");
  Serial.print(alcohol);
  Serial.println(" ppm");

  if (alcohol >= threshold && millis() - lastBuzzerTime > buzzerCooldown) {
    Serial.println(">>> CANH BAO: Phat hien nong do con cao!");
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
    lastBuzzerTime = millis();
  }

    // VL53Lx - Cảnh báo chưa cài quai mũ
  if (millis() - lastStrapCheck > STRAP_INTERVAL) {
    lastStrapCheck = millis();
    distance = readHelmetDistance();

    if (distance > DIST_THRESHOLD && distance > 0) {
      Serial.println(">>> CANH BAO: Chua cai quai mu!");
      
      // 4 tiếng bíp ngắn nhanh
      for (int i = 0; i < 4; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(150);
        digitalWrite(BUZZER_PIN, LOW);
        delay(150);
      }
      
      delay(2000); // Nghỉ 2s trước khi có thể kêu lại
    }

    Serial.print("Khoang cach quai mu: ");
    Serial.print(distance);
    Serial.println(" cm");
  }

  // Phát hiện ngã
  bool isFall = (abs(AccZ) < FALL_Z_THRESHOLD);
  if (isFall && !fallDetected && gps.location.isValid() && millis() - lastAccidentTime > COOLDOWN_TIME) {
    fallDetected = true;
    lastAccidentTime = millis();
    printAccidentInfo();
    sendAccidentSMS(gps.location.lat(), gps.location.lng());
  }
  if (!isFall) fallDetected = false;

  delay(50);
}
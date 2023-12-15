#include <Arduino.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <BH1750.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <FastLED.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h> 

// kết nối chân
#define ledPin      4 // Chân kết nối đến LED (thay đổi thành chân tương ứng)
#define pumpPin     2
#define DHTPIN      32
#define soilPin     35
#define buzzerPin   27
#define SS_PIN      5
#define RST_PIN     0
#define servoPin    33
#define buttonDoor  12
#define buttonLed   13
#define buttonPump  14
//servo
Servo myServo;

//Led
#define NUM_LEDS    17  // Số lượng LED trong dãy
CRGB leds[NUM_LEDS]; 

//RFID
MFRC522 mfrc522(SS_PIN, RST_PIN);
int UID[4], i;
int ID1[4] = {6, 43, 534, 63};// thẻ mở cửa cho user1
int ID2[4] = {243, 141, 180, 247}; //Thẻ mở cửa cho user2

//Cảm biến ánh sáng
BH1750 lightMeter(0x23);

//LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

//DHT11
const int DHTTYPE = DHT11;   
DHT dht(DHTPIN, DHTTYPE);

WiFiClient client;
PubSubClient mqtt_client(client);

//kết nối mqtt
const char *ssid = "manhtrannnnnn";
const char *password = "12345678";
const char *sever_mqtt = "172.20.10.3";
const int port_mqtt = 1883;
const char *mqtt_id = "smartgarden";

//khai báo biến
uint16_t count = 0;
String payload; //gửi JSON
float tmp = 32;
float humi = 69 ;
uint16_t lux = 0;
uint16_t soil = 0;

//trạng thái của các thiết bị
bool pumpState = false;
bool ledState = false;
bool doorState = false;
bool warning = false;
bool flagButtonDoor = true;
bool flagButtonLed = true;
bool flagButtonPump = true;
//Các kênh gửi thông tin(publish)
const char *dataStore_publish = "dataStore";
const char *dataSend_publish = "dataSend";
const char* warning_publish = "warning_publish";
const char* doorControl_publish = "doorControl_publish";
const char* pumpControl_publish = "pumpControl_publish";
const char* ledControl_publish = "ledControl_publish";

// Các kênh đăng kí(subcribe)
const char* pumpControl_subcribe = "pumpControl";
const char* ledControl_subcribe = "ledControl";
const char* doorControl_subcribe = "doorControl";
const char* warning_subcribe = "warning_subcribe";

void updateLCD(String text, int col, int row) {
  lcd.setCursor(col, row);
  lcd.print(text);
}

void clearLCD() {
  lcd.clear();
}

void displayData() {
  updateLCD("T:" + String(tmp, 1) + "C ", 0 , 0);
  updateLCD("H:" + String(humi, 1) + "% ", 8, 0);
  updateLCD("S:" + String(soil) + "%", 0, 1);
  updateLCD("L:" + String(lux) + "lux", 8, 1);
}
//Các hàm gọi từ mqtt xuống

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(message);
  if (String(topic) == pumpControl_subcribe) { // bật tắt máy bơm từ website
    if (message == "true") {
      if (pumpState == false) {
        digitalWrite(pumpPin, HIGH);
        pumpState = true;
      }
    } else if (message == "false") {
      if (pumpState == true) {
        digitalWrite(pumpPin, LOW);   
        pumpState = false;
      }
    }
  } 

  if (String(topic) == ledControl_subcribe) { // bật tắt đèn từ website
    if(message == "true"){
      if(ledState == false){
        ledState = true;
        for(int i = 0; i < NUM_LEDS; i++ ) leds[i] = CRGB(235, 235, 235);
        FastLED.show();
      }
    } else if(message == "false"){
        if(ledState == true){
          ledState = false;
          for(int i = 0; i < NUM_LEDS; i++ ) leds[i] = CRGB(0, 0, 0);
          FastLED.show();
        }
    }
  }

  if (String(topic) == doorControl_subcribe) { // đóng mở cửa bằng website
    if(message == "true"){
      if(doorState == false){
        doorState = true;
        myServo.write(150);
        warning = false;
        count = 0;
      }
    } else if(message == "false"){
      if(doorState == true){
        doorState = false;
        myServo.write(50);
      }
    }
  }
  if (String(topic) == warning_subcribe) { // xác nhận từ người dùng có kẻ đột nhập
    if (message == "false") {
      clearLCD();
      displayData();
      digitalWrite(buzzerPin, LOW);
      warning = false;
    } 
  }
}

void setup() {
  Serial.begin(9600);   
  Serial.println("Connecting to Wifi....");
  WiFi.begin(ssid, password);
  WiFi.reconnect();
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected to Wifi: ");
  Serial.print(ssid);
  Serial.println();
  Serial.print("IP Address: ");
  Serial.print(WiFi.localIP());
  Serial.println();
  delay(1000);
  mqtt_client.setServer(sever_mqtt, port_mqtt);
  Serial.println("Connecting to mqtt");
  while(!mqtt_client.connect(mqtt_id)){
    delay(500);
  }
  Serial.println("Connected to mqtt");  

  mqtt_client.setCallback(callback);

  //setup thiết bị
  Wire.begin();
  dht.begin(); 
  lightMeter.begin();
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW);
  pinMode(soilPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  myServo.attach(servoPin);
  myServo.write(50);

  FastLED.addLeds<NEOPIXEL,ledPin>(leds, NUM_LEDS);
  FastLED.show();

  SPI.begin();    
  mfrc522.PCD_Init();

  lcd.init();
  lcd.backlight();
  lcd.clear();
  mqtt_client.subscribe(doorControl_subcribe);
  mqtt_client.subscribe(ledControl_subcribe);
  mqtt_client.subscribe(pumpControl_subcribe);
  mqtt_client.subscribe(warning_subcribe);
}

//Gửi giá trị cảm biến lên
void Data() {
  humi = dht.readHumidity();
  tmp = dht.readTemperature();

  int analog = analogRead(soilPin);
  int phantramao = map(analog, 0, 1023, 0, 100);
  soil = 100 - phantramao;
  lux = lightMeter.readLightLevel();
  Serial.println(tmp);
  Serial.println(humi);


  payload = "{\"temperature\": " + String(tmp) + ", \"humidity\": " + String(humi) + ", \"soil\": " + String(soil) + ", \"brightness_level\": " + String(lux) + ", \"__v\": 0}";
}

void storeData(){
  mqtt_client.publish(dataStore_publish, payload.c_str());
}

void sendData(){
  mqtt_client.publish(dataSend_publish, payload.c_str());
}


// Mở cửa
void unlockDoor(){
  mqtt_client.publish(doorControl_publish, "true");
  doorState = true;
  myServo.write(150);
  digitalWrite(buzzerPin, HIGH);
  delay(100);
  digitalWrite(buzzerPin, LOW);
  warning = false;
  count = 0;
}

//Đóng cửa
void closeDoor(){
  mqtt_client.publish(doorControl_publish, "false");
  doorState = false;
  myServo.write(50);
  digitalWrite(buzzerPin, HIGH);
  delay(100);
  digitalWrite(buzzerPin, LOW);
}

//Sai thẻ
void incorrectTag(){
  digitalWrite(buzzerPin,HIGH);
  delay(100);
  digitalWrite(buzzerPin,LOW);
  delay(100);
  digitalWrite(buzzerPin,HIGH);
  delay(100);
  digitalWrite(buzzerPin,LOW);
  count++;
}

//Mở cửa bằng RFID
void handleRFID() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  for (byte i = 0; i < mfrc522.uid.size; i++) {
    UID[i] = mfrc522.uid.uidByte[i];
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  if(memcmp(UID, ID1, sizeof(UID)) == 0 || memcmp(UID, ID2, sizeof(UID)) == 0){
      if(doorState == false){
        unlockDoor();
      }
      else if(doorState == true){
        closeDoor();
      }
  }
  else {
      incorrectTag();
  }
}

//nút bấm cửa
void buttonDoorPress(){
  int touchState = digitalRead(buttonDoor);
  if(touchState == LOW) flagButtonDoor = true;
  if(flagButtonDoor == true){
    if(touchState == HIGH){
      flagButtonDoor = false;
      if (doorState == false) {
          unlockDoor();
        } else if(doorState == true) {
          closeDoor();
        }
    }
  }
}

//nút bấm led
void buttonLedPress(){
  int touchState = digitalRead(buttonLed);
  if(touchState == LOW) flagButtonLed = true;
  if(flagButtonLed == true){
    if(touchState == HIGH){
      flagButtonLed = false;
      if (ledState == false) {
          ledState = true;
          for(int i = 0; i < NUM_LEDS; i++ ) leds[i] = CRGB(235, 235, 235);
          FastLED.show(); 
          mqtt_client.publish(ledControl_publish, "true");
      } else {
          ledState = false;
          for(int i = 0; i < NUM_LEDS; i++ ) leds[i] = CRGB(0, 0, 0);
          FastLED.show();
          mqtt_client.publish(ledControl_publish,"false");
        }
    }
  }
}

// nút bấm máy bơm
void buttonPumpPress(){
  int touchState = digitalRead(buttonPump);
  if(touchState == LOW) flagButtonPump = true;
  if(flagButtonPump == true){
    if(touchState == HIGH){
      flagButtonPump = false;
      if (pumpState == false) {
          digitalWrite(pumpPin, HIGH);
          pumpState = true;
          mqtt_client.publish(pumpControl_publish, "true");
        } else {
          digitalWrite(pumpPin, LOW);
          pumpState = false;
          mqtt_client.publish(pumpControl_publish, "false");
        }
    }
  }
}

//thông báo có người đột nhập
void systemLock(){
  digitalWrite(buzzerPin, HIGH);
  clearLCD();
  updateLCD("-SYSTEM LOCKED-", 0, 0);
  mqtt_client.publish(warning_publish, "true");
  warning = true;
  count = 0;
}

unsigned long storeDataInterval = 60000;  // Khoảng thời gian giữa các lần gọi hàm storeData (1 phút)
unsigned long sendDataInterval = 2000;  // Khoảng thời gian giữa các lần gọi hàm sendData (2 giây)

unsigned long lastStoreDataTime = 0;
unsigned long lastSendDataTime = 0;

void loop() {
  
  unsigned long currentTime = millis();

  // Kiểm tra và gọi hàm storeData nếu đã đủ thời gian
  if (currentTime - lastStoreDataTime >= storeDataInterval) {
    storeData();
    lastStoreDataTime = currentTime;
  }

  // Kiểm tra và gọi hàm sendData nếu đã đủ thời gian
  if (currentTime - lastSendDataTime >= sendDataInterval) {
    Data();
    sendData();
    if(warning == false){
      displayData();
    }
    lastSendDataTime = currentTime;
  }
  handleRFID();
  buttonDoorPress();
  buttonLedPress();
  buttonPumpPress();
  if(count == 3){
    systemLock();
  }
  mqtt_client.loop();
  delay(100);
}


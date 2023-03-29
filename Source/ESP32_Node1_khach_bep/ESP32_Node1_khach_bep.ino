#include <DHT.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA21, SCL22 pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//khach
#define den_khach 17
#define quat_khach 2 // relay LOW
#define cua_so_khach 4

//bep
#define den_bep 5
#define quat_bep  15//relay low
#define cua_so_bep 16

//cam bien
#define DHT_pin 33
#define DHT_type DHT11

#define MQ2_analog 35
#define MQ2_digital 32

#define flame_sensor 34
//canh bao
#define loa_canh_bao1 18
#define den_canh_bao_1 19

#define WIFI_SSID "wifi"
#define WIFI_PASSWORD "00000000"

#define FIREBASE_HOST "datn-6f71c-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "WoF9zZmOnKZqHopaAU9zWhKFlFk2sfbwetAVqLlr"
 
FirebaseData firebaseData;
FirebaseJson json;

void oled_display (float,float);

DHT dht(DHT_pin, DHT_type);

//bien trang thai
  byte den1_status = 0; 
  byte den2_status = 0; 
  byte quat1_status = 1;  
  byte quat2_status = 1;  
  byte cua_so_khach_status = 0;
  byte cua_so_bep_status = 0;
  
  byte den_canh_bao_1_status = 0;
  byte loa_canh_bao_1_status = 0;

  Servo cuaso1;
  Servo cuaso2;
  
  bool tmp=0;

  float h1;
  float t1;

  byte mq2_status;
  int mq2_value;
  byte flame_status;
  byte mua_status;
  byte tam;

unsigned long t1s = 0;

void setup()
{
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Dang ket noi Wifi");
  while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Serial.println (" ");
  Serial.println ("Da ket noi WiFi!");
  Serial.println ("Dia chi WiFi: ");
  Serial.print(WiFi.localIP());

  pinMode(MQ2_digital, INPUT);
  pinMode(flame_sensor, INPUT);

//PIN OUPUT
  pinMode(den_khach, OUTPUT);
  pinMode(quat_khach, OUTPUT);
  cuaso1.attach(cua_so_khach);

  pinMode(den_bep, OUTPUT);
  pinMode(quat_bep, OUTPUT);
  cuaso2.attach(cua_so_bep);
  
  pinMode(den_canh_bao_1, OUTPUT);
  pinMode(loa_canh_bao1, OUTPUT);

//SET trang thai dau
  digitalWrite(den_khach, den1_status);
  digitalWrite(quat_khach, quat1_status);
  cuaso1.write(0);

  digitalWrite(den_bep, den2_status);
  digitalWrite(quat_bep, quat2_status);
  cuaso2.write(0);

  digitalWrite(den_canh_bao_1, den_canh_bao_1_status);
  digitalWrite(loa_canh_bao1, loa_canh_bao_1_status);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
  {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);

}

void loop()
{
  h1 = dht.readHumidity(); // Đọc giá trị độ ẩm môi trường ở đơn vị %
  t1 = dht.readTemperature(); // Đọc giá trị nhiệt độ môi trường ở đơn vị °C
  mq2_value = analogRead(MQ2_analog);
  mq2_status = digitalRead(MQ2_digital);
  flame_status = digitalRead(flame_sensor);

   if (isnan(mq2_value) ) // Kiểm tra nếu giá trị đọc được từ môi trường có phải 1 số có nghĩa (và kiểm tra lại)
    {
    Serial.println(F("\nKhong the lay gia tri, vui long kiem tra lai cam bien!"));
    return;
    }
  Serial.print(F("\nMQ2 gia tri: "));
  Serial.print(mq2_value);
  Serial.print(F("\nMQ2 trang thai: "));
  Serial.print(mq2_status);
  Serial.print("\n");

  Firebase.setInt (firebaseData, "/Khigasgiatri", mq2_value);
  Firebase.setInt (firebaseData, "/Khigastrangthai", mq2_status);

    if(mq2_status==0) //trạng thái bình thường bằng 1, khi vượt ngưỡng sẽ bằng 0
    {
      den_canh_bao_1_status = 1;
      digitalWrite(den_canh_bao_1, den_canh_bao_1_status);
      Serial.println(F("\nDen canh bao trang thai: "));
      Serial.print(den_canh_bao_1_status);
      Firebase.setInt (firebaseData, "/Cua1", 1);
      Firebase.setInt (firebaseData, "/Cua2", 1);
      Firebase.setInt (firebaseData, "/Quat1", 0);
      Firebase.setInt (firebaseData, "/Quat2", 0);
      digitalWrite(quat_bep, 0);
      digitalWrite(quat_khach, 0);
      cuaso1.write(90);
      cuaso2.write(90);
    }
    if(mq2_status==1)
    {
    digitalWrite(den_canh_bao_1, 0);
    }
 
  if (isnan(h1) || isnan(t1)) // Kiểm tra nếu giá trị đọc được từ môi trường có phải 1 số có nghĩa (và kiểm tra lại)
  {
    Serial.println(F("\nKhong the lay gia tri cam bien DHT11, vui long kiem tra lai cam bien!"));
    return;
  }
  
  Serial.print(F("\nDo am: "));
  Serial.print(h1);
  Serial.print("%\n");
  Serial.print(F("Nhiet do: "));
  Serial.print(t1);
  Serial.print(F("°C "));

  Firebase.setFloat (firebaseData, "/Nhietdo1", t1);
  Firebase.setFloat (firebaseData, "/Doam1", h1);

  if(!flame_status) //trạng thái bình thường bằng 1, khi vượt ngưỡng sẽ bằng 0
  {
    digitalWrite(loa_canh_bao1, HIGH);
    Firebase.setInt (firebaseData, "/Lua", flame_status);
  }
  else
  {
    digitalWrite(loa_canh_bao1, LOW);
    Firebase.setInt (firebaseData, "/Lua", flame_status);
  }

  if(Firebase.getString (firebaseData, "/Den1"))
  {
    den1_status = firebaseData.to<String>().toInt(); 
    digitalWrite(den_khach, den1_status);
  }
 
  if(Firebase.getString (firebaseData, "/Quat1"))
  {
    quat1_status = firebaseData.to<String>().toInt(); 
    digitalWrite(quat_khach, quat1_status);  
  }

  if(Firebase.getString (firebaseData, "/Cua1"))
  {
    cua_so_khach_status = firebaseData.to<String>().toInt(); 
     if (cua_so_khach_status == 1)
        {
          cuaso1.write(90);
        }
        else if (cua_so_khach_status == 0) 
        {
          cuaso1.write(0);
        }
        Serial.print(F("\nTrang thai cua1: "));
        Serial.print(cua_so_khach_status);   
  }

//Bếp
  if(Firebase.getString (firebaseData, "/Den2"))
  {
    den2_status = firebaseData.to<String>().toInt(); 
    digitalWrite(den_bep, den2_status);
  }

  if(Firebase.getString (firebaseData, "/Quat2"))
  {
    quat2_status = firebaseData.to<String>().toInt(); 
    digitalWrite(quat_bep, quat2_status);
  }

  if(Firebase.getString (firebaseData, "/Cua2"))
  {
    cua_so_bep_status = firebaseData.to<String>().toInt(); 

     if (cua_so_bep_status == 1)
        {
          cuaso2.write(90);             
        }
        else if (cua_so_bep_status == 0) 
        {
          cuaso2.write(0);
        }
        Serial.print(F("\nTrang thai cua2: "));
        Serial.print(cua_so_bep_status);   
  }

  if(Firebase.getString (firebaseData, "/Mua"))
  {
    mua_status = firebaseData.to<String>().toInt();
    if (mua_status == 1 && mq2_status != 0)
    {
      Firebase.setInt (firebaseData, "/Cua1", 0);
      Firebase.setInt (firebaseData, "/Cua2", 0);
    }
  }
   oled_display(t1, h1);
}
void oled_display(float t, float h)
{
  // clear display
  display.clearDisplay();
  
  // display temperature
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Nhiet do: ");
  display.setTextSize(2);
  display.setCursor(0,10);
  display.print(t);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(2);
  display.print("C");
  
  // display humidity
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("Do am: ");
  display.setTextSize(2);
  display.setCursor(0, 45);
  display.print(h);
  display.print(" %"); 
  
  display.display(); 
}

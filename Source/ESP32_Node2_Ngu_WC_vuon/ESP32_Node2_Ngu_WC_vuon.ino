#include <DHT.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <ArduinoJson.h>

#define WIFI_SSID "wifi"
#define WIFI_PASSWORD "00000000"

#define FIREBASE_HOST "datn-6f71c-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "WoF9zZmOnKZqHopaAU9zWhKFlFk2sfbwetAVqLlr"

FirebaseData firebaseData;
FirebaseJson json;

#define den_ngu 22
#define quat_ngu 21 //relay LOW

#define den_wc 23

#define bom_nuoc 19 //relay LOW

//cam bien
#define trigPin  33
#define echoPin  25  
#define DHT_pin 26
#define DHT_type DHT11
DHT dht(DHT_pin, DHT_type);
#define analog_dat  35
#define digital_dat 32
#define cam_bien_mua 34 

//khai báo tốc độ âm thanh đơn vị cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701
#define chieu_cao_bon_nuoc 10 // chiều cao bồn đơn vị centimet

//bien trang thai
byte den3_status = 0; 
byte den4_status = 0; 
byte quat3_status = 1;  
byte bom_nuoc_status = 1;  
byte mua_status = 0;

long  thoi_gian;
float khoang_cach; // đơn vị centimet
float muc_nuoc; // đơn vị %
float h2;
float t2; 
float gia_tri_dat;
float phan_tram_dat;
float mua, phan_tram_mua;
int caidat;

int tong_dat = 0;
int tong_mua = 0;
int hien_thi_dat=0;
int hien_thi_mua=0;
int hien_thi_nuoc=0;

unsigned long t2s = 0;
unsigned long t3s = 0;

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

//PIN OUPUT
  pinMode(den_ngu, OUTPUT);
  pinMode(quat_ngu, OUTPUT);
  pinMode(den_wc, OUTPUT);
  pinMode(bom_nuoc, OUTPUT);

//SET trang thai dau
  digitalWrite(den_ngu, den3_status);
  digitalWrite(quat_ngu, quat3_status);
  digitalWrite(den_wc, den4_status);
  digitalWrite(bom_nuoc, bom_nuoc_status);

//cam bien
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT); 
  pinMode(digital_dat, INPUT);
  pinMode(cam_bien_mua, INPUT); 
  pinMode(cam_bien_mua, INPUT); 
}
void loop()
{
  h2 = dht.readHumidity(); // Đọc giá trị độ ẩm môi trường ở đơn vị %
  t2 = dht.readTemperature(); // Đọc giá trị nhiệt độ môi trường ở đơn vị °C

   if (millis() - t3s >= 1000)
  {
    if (isnan(h2) || isnan(t2)) // Kiểm tra nếu giá trị đọc được từ môi trường có phải 1 số có nghĩa (và kiểm tra lại)
    {
      Serial.println(F("\nKhong the lay gia tri cam bien DHT11, vui long kiem tra lai cam bien!"));
      return;
    }

    Firebase.setFloat (firebaseData, "/Nhietdo2", t2);
    Firebase.setFloat (firebaseData, "/Doam2", h2);
    
    Serial.print(F("\nDo am: "));
    Serial.print(h2);
    Serial.println("%");
    Serial.print(F("Nhiet do: "));
    Serial.print(t2);
    Serial.println(F("°C "));  
  t3s=millis();
  }  

if (millis() - t2s >= 2000)
{
    //mua
    for(int i=0; i<=9; i++)
    {
    tong_mua+=analogRead(cam_bien_mua);
    }
    mua = tong_mua / 10;
    phan_tram_mua = 100 - (mua/4095)*100;
    hien_thi_mua = phan_tram_mua;
    if (hien_thi_mua >= 20)
    {
      mua_status = 1;
    }
    else
    mua_status =0;
    Serial.print("Mua: ");
    Serial.println(mua);
    Serial.print("Phan Tram mua ");
    Serial.print(phan_tram_mua);
    Serial.println('%');
    Serial.print("Hien thi mua ");
    Serial.print(hien_thi_mua);
    Serial.println('%');

    Firebase.setInt (firebaseData, "/Mua", mua_status);
    Firebase.setInt (firebaseData, "/Muagiatri", hien_thi_mua);    
    //dat
    for(int i=0;i<=9;i++)
    {
    tong_dat+=analogRead(analog_dat);
    }
    gia_tri_dat=tong_dat/10;
    phan_tram_dat = 100 - (gia_tri_dat/4095)*100;
    hien_thi_dat = phan_tram_dat; 
    Serial.print("Do am dat: ");
    Serial.print(phan_tram_dat);
    Serial.println('%');
    Serial.print("Gia tri analog: ");
    Serial.println(gia_tri_dat);
    Serial.print("Gia tri digital: ");
    Serial.println(digitalRead(digital_dat));
    Serial.print("chan analog: ");
    Serial.println(analogRead(analog_dat));
    Serial.println("\n");

    Firebase.setInt (firebaseData, "/Doamdat", hien_thi_dat);

    delay(2);
    tong_dat = 0;
    tong_mua = 0;
    t2s = millis();
}

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    
    digitalWrite(trigPin, HIGH); // Chuyển trạng thái HiGH trong 10 micro giây
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    thoi_gian = pulseIn(echoPin, HIGH); // Đọc thời gian di chuyển của sóng âm phát ra và nhận về
    khoang_cach = thoi_gian * SOUND_SPEED/2;
    muc_nuoc = 100-(khoang_cach/chieu_cao_bon_nuoc)*100;
    hien_thi_nuoc = muc_nuoc;
    if(hien_thi_nuoc <= 0)
    {
      hien_thi_nuoc=0;
    }
    
    Serial.print("Khoang cach: ");
    Serial.print(khoang_cach);
    Serial.println("cm");
    Serial.print("Muc nuoc: ");
    Serial.print(muc_nuoc);
    Serial.println("%");

    Firebase.setInt (firebaseData, "/Nuoc", hien_thi_nuoc);
 

  if(Firebase.getString (firebaseData, "/Den3"))
  {
    den3_status = firebaseData.to<String>().toInt(); 
    digitalWrite(den_ngu, den3_status);
  }

  if(Firebase.getString (firebaseData, "/Quat3"))
  {
    quat3_status = firebaseData.to<String>().toInt(); 
    digitalWrite(quat_ngu, quat3_status);
  }

  if(Firebase.getString (firebaseData, "/Den4"))
  {
    den4_status = firebaseData.to<String>().toInt(); 
    digitalWrite(den_wc, den4_status);
  }

  if(Firebase.getString (firebaseData, "/Doamcaidat"))
  {
      caidat = firebaseData.to<String>().toInt(); 
      if (hien_thi_dat < caidat)
      {
        Firebase.setInt (firebaseData, "/Pump", 0);
      }
      else
      {
        Firebase.setInt (firebaseData, "/Pump", 1);
      }

      Serial.print("cai dat: ");
      Serial.println(caidat);
  }
   if(Firebase.getString (firebaseData, "/Pump"))
      {
        bom_nuoc_status =  firebaseData.to<String>().toInt(); 
        digitalWrite(bom_nuoc, bom_nuoc_status);
      }
}
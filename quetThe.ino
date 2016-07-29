#include <Wire.h>
#include <SPI.h>
#include <Servo.h>
#include <SD.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

#include "pitch.h"

#define SS_PIN 6
#define RST_PIN 9
#define SERVO_ADDRESS 3
#define LED1 4 // BAO LOI
#define LED2 2 //hien THI MOTOR CHAY MO CUA -mau DUONG -MO COI
#define LED3 5  //ktra pin - xanh la 
#define DOOR_BTN 7
#define BELL 8
#define ESC_BTN 10

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x3f, 16, 4);
MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo servo;

unsigned long uidDec, uidDecTemp;
int posServo;
const int chipSelect = 10;
File logFile;
char fileName[] = "quetThe.csv";

void error(char *str)
{
  Serial.print("Loi: ");
  Serial.println(str);
  
  // red LED indicates error
  digitalWrite(LED1, HIGH);
  digitalWrite(LED3, LOW);
  digitalWrite(LED2, LOW);//TAT DEN XANH LA
  while(1);
}

void setup() {
  Wire.begin();
  // Setup serial va lcd
  Serial.begin(9600);

  // Setup dong ho RTC
  if (!rtc.begin()) {
    Serial.println("Khong the ket noi voi RTC");
    while (true);
  }
  
  if (rtc.lostPower()) {
    Serial.println("Dat thoi gian cho RTC");
    // Dat thoi gian luc chep file nay
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // Setup dau doc the
  SPI.begin();
  mfrc522.PCD_Init();

  // Dong servo
  servo.attach(SERVO_ADDRESS);
  for (posServo = 50; posServo >= 0; posServo--) {
    servo.write(posServo);
    delay(5);
  }
  servo.detach();

  // Setup the SD
  pinMode(chipSelect, OUTPUT);
  if (!SD.begin(chipSelect)) {
    error("Khong co the SD");
  }
  logFile = SD.open(fileName, FILE_WRITE);
  if (!logFile) {
    error("Khong the mo file");
  }
  //logFile.println("sep=,");
  logFile.println("Ngay,Gio,Ten,MS,Truy Cap");
  logFile.flush();
  Serial.println("Da setup xong file SD");
           
  Serial.println("Hay dua the vao");
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Hay dua the vao");
  delay(5500);
  lcd.noBacklight();
  
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(BELL, OUTPUT);
  pinMode(DOOR_BTN, INPUT);

  digitalWrite(LED3, HIGH);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(BELL, HIGH);
  delay(800);
  digitalWrite(BELL, LOW);
}

void loop() {

  kiemTraNutKhanCap();
  // Kiem tra cạy cửa
  kiemTraChuaDongCua();
  
   if ( ! mfrc522.PICC_IsNewCardPresent()) {
    //Serial.println("Ko co the 1");
    return;
  }

  if ( ! mfrc522.PICC_ReadCardSerial()) {
    //Serial.println("Ko co the 2");
    return;
  }

  uidDec = 0;
  Serial.println("----------");

  Serial.println("Da nhan duoc the");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uidDecTemp = mfrc522.uid.uidByte[i];
    uidDec = uidDec * 256 + uidDecTemp;
  }

  Serial.println(uidDec);
  lcd.clear();
  switch(uidDec) {
     case 2664542720:
      theAdmin("Duc-Admin"); //Copy phần này thay đổi số thẻ
      break;
    case 2533030730:
      thePhu("Phuoc-19h00");
      break;
    case 1986395210:
      thePhu("Chau-18h00");
      break;
    case 3680056064:
      thePhu("Thuy-17h40");
      break;
    case 435693326:
      thePhu("TMy-admin");
      break;
    default:
      luuVaoSD(uidDec, "", false);
      printTimeNow();
      theSai();
      break; 
  }
}

void theAdmin(char user[]) {
  Serial.println(user);
  lcd.print(user);
  printTimeNow();
  luuVaoSD(uidDec, user, true);
  theDung();
}

void thePhu(char user[]) {
   Serial.println(user);
      lcd.print(user);
      if (!kiemTraTG()) {
        Serial.println("K hieu luc");
        lcd.backlight();
        lcd.setCursor(3,1);
        lcd.print(" K hieu luc");
        lcd.setCursor(0,3);
   //   lcd.print("6h00 - 18h00");
    //  delay(1000);
    //  lcd.clear();
     // lcd.setCursor(0,1);
     // lcd.print("Quay lai sau nha");
    //  delay(1000);
    //  lcd.clear();
        
    //lcd.print(" K hieu luc");
        
        printTimeNow();
        luuVaoSD(uidDec, user, false);
        digitalWrite(LED1, HIGH);
        digitalWrite(LED2, HIGH);
        digitalWrite(LED3, HIGH);
        //digitalWrite(BELL, HIGH);
        //delay(3000);
            digitalWrite(BELL, HIGH);
            delay(400);
            digitalWrite(BELL, LOW);       
            delay(200);
            digitalWrite(BELL, HIGH);
            delay(400);
            digitalWrite(BELL, LOW);       
            delay(200);
            digitalWrite(BELL, HIGH);
            delay(400);
            digitalWrite(BELL, LOW);       
            delay(200);
            digitalWrite(BELL, HIGH);
            delay(400);
            digitalWrite(BELL, LOW);       
            delay(200);
            digitalWrite(BELL, HIGH);
            delay(400);
            
        digitalWrite(LED1, LOW);
        digitalWrite(LED2, LOW);
        digitalWrite(BELL, LOW);
        lcd.noBacklight();
        return;
      }
       printTimeNow();
      luuVaoSD(uidDec, user, true);
      theDung();
}

void luuVaoSD(unsigned long uidDec, char user[], bool allowed) {
  // Luu truy cap vao file trong the SD
  // Thoi gian hien tai
  DateTime now = rtc.now();
  logFile.print(now.day());
  logFile.print("/");
  logFile.print(now.month());
  logFile.print("/");
  logFile.print(now.year());
  logFile.print(",");
  logFile.print(now.hour());
  logFile.print(":");
  logFile.print(now.minute());
  logFile.print(":");
  logFile.print(now.second());
  logFile.print(",");
  logFile.print(user);
  logFile.print(",");
  logFile.print(uidDec);
  logFile.print(",");
  if (allowed) {
    logFile.println("Cho phep");
  } else {
    logFile.println("Khong cho phep");
  }
  logFile.flush();
}

bool kiemTraTG() {
  DateTime now = rtc.now();
  return (now.hour() >= 6 && now.hour() < 18);
}

void theSai() {
  // The khong co dung
  // Bật LED đỏ (LED1)
  // Tắt LED xanh (LED2)
  // Còi hú
  // Đợi 1 phút
  lcd.clear();
  lcd.setCursor(0,1);    
  lcd.print("the sai  yeu cau");
  lcd.print(" k su dung");
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, LOW);

  
                    lcd.backlight();
  
  Serial.println("The nay khong co");
 // digitalWrite(BELL, HIGH);
 // delay(5000); //sua 60000 thành 30000, chờ test lâu quá.khi ra thực tế thay dổi sau
  //digitalWrite(BELL, LOW);
            digitalWrite(BELL, HIGH);
            delay(300);
            digitalWrite(BELL, LOW);       
            delay(100);
            digitalWrite(BELL, HIGH);
            delay(400);
            digitalWrite(BELL, LOW);       
            delay(300);
            digitalWrite(BELL, HIGH);
            delay(300);
            digitalWrite(BELL, LOW);       
            delay(100);
            digitalWrite(BELL, HIGH);
            delay(300);
            digitalWrite(BELL, LOW);       
            delay(200);
            digitalWrite(BELL, HIGH);
            delay(400);
             digitalWrite(BELL, HIGH);
            delay(300);
            digitalWrite(BELL, LOW);       
            delay(300);
            digitalWrite(BELL, HIGH);
            delay(400);
            digitalWrite(BELL, LOW);       
            delay(300);
            digitalWrite(BELL, HIGH);
            delay(300);
            digitalWrite(BELL, LOW);       
            delay(300);
            digitalWrite(BELL, HIGH);
            delay(400);
            digitalWrite(BELL, LOW);       
            delay(200);
            digitalWrite(BELL, HIGH);
            delay(400);
            digitalWrite(BELL, LOW);  
              
                    lcd.noBacklight();
  
  Serial.println("================================================");
}

void theDung() {
  // The co dung
  // Bật LED xanh (LED2)
  // Tắt LED đỏ (LED1)
  // Mở ổ khoá
  // Còi hú
  // Đóng ổ khoá
  // Đợi 5s
              lcd.backlight();
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, LOW); //MOI THEM

  servo.attach(SERVO_ADDRESS);
  // Mo cua
  for (posServo = 0; posServo <= 157; posServo++) { //thay doi 180 thanh 135
   servo.write(posServo);
    delay(5);
  }

  Serial.println("The nay dung");
  choiNhac();

  // Dong cua
  for (posServo = 135; posServo >=0; posServo--) { //thay doi 180 thanh 135 theo hàng trên nha
    servo.write(posServo);
    delay(5);
    
                    lcd.noBacklight();
  }
  servo.detach();
  
  digitalWrite(LED2, HIGH);
  digitalWrite(LED1, HIGH);
  digitalWrite(LED3, HIGH);
  digitalWrite(BELL, LOW);

  kiemTraChuaDongCua();
  
  Serial.println("================================================");
}

void kiemTraNutKhanCap() {
  int doorStatus = digitalRead(ESC_BTN);
  while (doorStatus == LOW) {
    lcd.backlight();
    lcd.clear();
    lcd.print("Mo Khan Cap");
    theDung();
  }
}

void kiemTraChuaDongCua() {
  int doorStatus = digitalRead(DOOR_BTN);
  while (doorStatus == LOW) {
                  
                    lcd.backlight();
    Serial.println("Chua dong cua");
    lcd.clear();
    lcd.print("Chua dong cua");
    digitalWrite(LED3, LOW);//DOI 1 THANH 3
    digitalWrite(LED1, HIGH); //MOI THEM
    digitalWrite(LED2, LOW);//MOI THEM
    digitalWrite(BELL, HIGH);
    delay(250);
    digitalWrite(BELL, LOW);
    doorStatus = digitalRead(DOOR_BTN);
  }
  digitalWrite(BELL, LOW);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, HIGH);
  
                    lcd.noBacklight();
}

/** In thoi gian hien tai
 *  vao Serial va LCD
 */
void printTimeNow() {
  
  DateTime now = rtc.now();
  
  Serial.print("Thoi Gian hien tai: ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.println(now.second(), DEC);
  
  Serial.print("Ngay thang: ");
  Serial.print(now.day(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.println(now.year(), DEC);

  lcd.setCursor(0,1);
  lcd.print(now.hour());
  lcd.print(":");
  lcd.print(now.minute());
  lcd.print(":");
  lcd.print(now.second());
  lcd.print("   ");
  lcd.print(now.day());
  lcd.print("/");
  lcd.print(now.month());
  //lcd.print("/");
  //lcd.print(now.year());
}

void choiNhac() {
  int notes[] = {NOTE_C4, NOTE_G3,NOTE_G3, NOTE_A3, NOTE_G3,0, NOTE_B3, NOTE_C4};
  int beats[] = {4, 8, 8, 4, 4, 4, 4, 4};

  for (int i = 0; i < 1; i++) {
    for (int thisNote = 0; thisNote < 8; thisNote++) {

    // bây giờ ta đặt một nốt nhạc là 1 giây = 1000 mili giây
    // thì ta chia cho các thành phần noteDurations thì sẽ
    // được thời gian chơi các nốt nhạc
    // ví dụ: 4 => 1000/4; 8 ==> 1000/8 
    int noteDuration = 1000/beats[thisNote];
    tone(BELL, notes[thisNote],noteDuration);

    // để phân biệt các nốt nhạc hãy delay giữa các nốt nhạc
    // một khoảng thời gian vừa phải. Ví dụ sau đây thực hiện tốt
    // điều đó: Ta sẽ cộng 30% và thời lượng của một nốt
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    
    //Ngừng phát nhạc để sau đó chơi nhạc tiếp!
    noTone(BELL);
    }
            digitalWrite(BELL, HIGH);
            delay(800);
            digitalWrite(BELL, LOW);       
            delay(100);
            digitalWrite(BELL, HIGH);
            delay(700);
            digitalWrite(BELL, LOW);       
            delay(100);
            digitalWrite(BELL, HIGH);
            delay(600);
            digitalWrite(BELL, LOW);       
            delay(100);
            digitalWrite(BELL, HIGH);
            delay(550);
            digitalWrite(BELL, LOW);       
            delay(100);
            digitalWrite(BELL, HIGH);
            delay(530);
            digitalWrite(BELL, LOW);       
            delay(100);
            digitalWrite(BELL, HIGH);
            delay(520);
            digitalWrite(BELL, LOW);       
            delay(100);
            digitalWrite(BELL, HIGH);
            delay(500);
            digitalWrite(BELL, LOW);       
            delay(100);
            digitalWrite(BELL, HIGH);
            delay(400);
            digitalWrite(BELL, LOW);       
            delay(100);
            digitalWrite(BELL, HIGH);
            delay(400);
             digitalWrite(BELL, HIGH);
            delay(300);
            digitalWrite(BELL, LOW);       
            delay(100);
            digitalWrite(BELL, HIGH);
            delay(250);
            digitalWrite(BELL, LOW);       
            delay(100);
            digitalWrite(BELL, HIGH);
            delay(200);
             digitalWrite(BELL, HIGH);
            delay(180);
            digitalWrite(BELL, LOW);       
            delay(100);
            digitalWrite(BELL, HIGH);
            delay(160);
            digitalWrite(BELL, LOW);       
            delay(100);
            digitalWrite(BELL, HIGH);
            delay(500);
            digitalWrite(BELL, LOW);
  }
  Serial.println("Choi nhac xong");
}


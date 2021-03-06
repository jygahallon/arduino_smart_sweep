#include <SPI.h>
#include <MFRC522.h>
#include <WiFiEsp.h>
#include <ArduinoJson.h>
#include <DFRobot_sim808.h>
#include <Adafruit_Thermal.h>
#include <SoftwareSerial.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "Logo.h"

#define SS_PIN 53 
#define RST_PIN 5
#define PIN_TX    10
#define PIN_RX    11 
#define TX_PIN 7
#define RX_PIN 6
#define MESSAGE_LENGTH 160
#define GRP_PIN 4
#define GRN_PIN 13
#define RED_PIN 12
SoftwareSerial printerSerial(RX_PIN, TX_PIN); // Declare SoftwareSerial obj first
Adafruit_Thermal printer(&printerSerial);   

SoftwareSerial mySerial(PIN_TX,PIN_RX);
DFRobot_SIM808 sim808(&mySerial);
String message;

String terminal_id="1";


char ssid[] = "apcmhi";     // your network SSID (name)
char pwd[] = "egi12345";  // your network password
String line;
WiFiEspClient client;

String url;
String http ="  HTTP/1.1";
String host="Host: 206.189.209.210";
String connection="Connection: keep-alive";


MFRC522 mfrc522(SS_PIN, RST_PIN);        
MFRC522::MIFARE_Key key;
int block=2;
byte readbackblock[18];
String card_number;

LiquidCrystal_I2C lcd(0x27, 20, 4);

bool discount=false;
bool grp=true;

int ledState = HIGH;         
int buttonState;            
int lastButtonState = LOW;

int grpCount=0;

//TerminalInformation
double fare;
String destination;
String source;
String title;
String queueNumber;

//UserInformation
double balance;
String phone;
double paymentUser;

String grpRep;
String grpNumber;
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50; 

void setup() {
 pinMode(GRN_PIN, OUTPUT);
 pinMode(RED_PIN,OUTPUT);
 digitalWrite(GRN_PIN,HIGH);
 digitalWrite(RED_PIN,HIGH);
 delay(1000);
 digitalWrite(GRN_PIN,LOW);
 digitalWrite(RED_PIN,LOW);
 Serial.begin(19200);
 Serial1.begin(9600);
 mySerial.begin(9600);
 printerSerial.begin(19200);
 
 lcd.begin();
 lcd.backlight();
 lcd.setCursor(1,0);
 lcd.print("Connecting to Wifi");
 WiFi.init(&Serial1);
 while (WiFi.status() != WL_CONNECTED)
 {
   WiFi.begin(ssid, pwd); 
   digitalWrite(GRN_PIN, HIGH);
   digitalWrite(RED_PIN,HIGH);
 }
 digitalWrite(GRN_PIN, LOW);
   digitalWrite(RED_PIN,LOW);
 lcd.clear();
 lcd.setCursor(1,0);
 lcd.print("Connecting to Host");
 clientConnect();
 terminalInformation();
 resetTerminal();
 lcd.clear();
 
 SPI.begin();               
 mfrc522.PCD_Init();

 for (byte i = 0; i < 6; i++) 
 {
  key.keyByte[i] = 0xFF;
 }


 
 pinMode(GRP_PIN,INPUT);
 digitalWrite(GRP_PIN,HIGH);

 while(!sim808.init())
  {
      Serial.print("Sim808 init error\r\n");
      lcd.setCursor(1,0);
      lcd.print("Connecting to SIM");
      digitalWrite(GRN_PIN, HIGH);
      digitalWrite(RED_PIN,HIGH);
      
  }
  lcd.clear(); 
  sms("09202868902","Ready");
  digitalWrite(GRN_PIN, LOW);
  digitalWrite(RED_PIN,LOW);
}

void loop() {
  digitalWrite(GRN_PIN, LOW);
   digitalWrite(RED_PIN,LOW);
  initDisplay();
  buttonCheck();
  if ((!grp)&&(grpCount!=0))
    {
      //insert group to database
      Serial.print("grpRep:");
      Serial.println(card_number);
      Serial.print("grpCount:");
      Serial.println(grpCount);
      grpCount=0;
      printNumber();

    }
  if ( !mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  readBlock(block, readbackblock);//read the block back
  card_number = (char*)readbackblock;
  if (card_number.equals("admin"))
  {
    discount=!discount;
    lcd.clear();  
  }
  
  else
  {
    lcd.setCursor(0,2);
    lcd.print("Reading Card");
    clientConnect();
    if (!userInformation(card_number))
    {
      lcd.setCursor(0,2);
      lcd.print("Invalid Card");
      digitalWrite(RED_PIN, HIGH);
      delay(1000);
      digitalWrite(RED_PIN,LOW);
      lcd.clear();
      return;
    }
    
    if (!checkFare())
    {
      lcd.setCursor(0,2);
      lcd.print("Insufficient Balance");
      digitalWrite(RED_PIN,HIGH);
      delay(1000);
      digitalWrite(RED_PIN,LOW);
      lcd.clear();
      return;
    }
   
    if ((grp)&&(grpCount==0))
    {
        //getRep
      grpRep=card_number;
      grpNumber=phone;
      Serial.println("Group");
      Serial.println(grpRep);
      Serial.println(grpNumber);
      queueTerminal();
    }
    
    lcd.clear();
    initDisplay();  
    if (grp)
    {
      grpCount++;
      lcd.setCursor(0,2);
      digitalWrite(GRN_PIN, HIGH);
      lcd.print("Processing");
     // queueSingle();
      lcd.clear();
      initDisplay();
      lcd.setCursor(0,2);
      lcd.print("Queued");
      delay(1000);
      digitalWrite(GRN_PIN,LOW);
      lcd.clear();
      initDisplay();  
    }
   
    
    
    else if (!grp)
    {
      lcd.setCursor(0,2);
      lcd.print("Processing");
      digitalWrite(GRN_PIN, HIGH);
      queueTerminal();
      digitalWrite(GRN_PIN,LOW);
     // queueSingle();
      lcd.clear();
      initDisplay();
      lcd.setCursor(0,2);
      lcd.print("Queued");
      delay(1000);
      lcd.clear();
      //insert single to database
      Serial.print("Single:");
      Serial.println(card_number);
    }
   
    lcd.clear();
    initDisplay();
    payment();
    if(!grp)
    {
      
      printNumber();
    }
  }
  
  Serial.println(card_number);
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1(); 

  

}

void initDisplay(){
 lcd.setCursor(0,0);
 lcd.print("PASSENGER:");
 lcd.setCursor(10,0);
 if (!discount)
 {
  lcd.print("REGULAR");
 }
 else
 {
  lcd.print("DISCOUNTED");
 }
 lcd.setCursor(0,1);
 if (!grp)
 {
  lcd.print("SINGLE");
 }
 else
 {
  lcd.print("GROUP COUNT:");
  lcd.setCursor(12,1);
  lcd.print(grpCount);
 }
}

void buttonCheck(){
   int reading = digitalRead(GRP_PIN);

  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
  
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == HIGH) {
        grp=!grp;
        lcd.clear();
      }

    }
  }
  lastButtonState = reading;
}

int readBlock(int blockNumber, byte arrayAddress[]) 
{
  int largestModulo4Number=blockNumber/4*4;
  int trailerBlock=largestModulo4Number+3;

  byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
 
  if (status != MFRC522::STATUS_OK) {
         Serial.print("PCD_Authenticate() failed (read): ");
         Serial.println(mfrc522.GetStatusCodeName(status));
         return 3;
  }
      
  byte buffersize = 18;
  status = mfrc522.MIFARE_Read(blockNumber, arrayAddress, &buffersize);
  if (status != MFRC522::STATUS_OK) {
          Serial.print("MIFARE_read() failed: ");
          Serial.println(mfrc522.GetStatusCodeName(status));
          return 4;
  }
  Serial.println("block was read");
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}
bool checkFare()
{
  if (!discount)
  {
    if ((balance-fare)>=0)
    {
      return true;  
    }
    else 
    {
      return false;
    }
  }
  else
  {
    double discounted = fare-(fare*.20);
    if ((balance-discounted)>=0)
    {
      return true;  
    }
    else 
    {
      return false;
    }
  }
}

void payment()
{
  lcd.setCursor(0,2);
  lcd.print("FARE:");
  lcd.setCursor(5,2);
  if (!discount)
  {
    paymentUser=fare;
    lcd.print(fare);
    balance-=fare;
   
  }
  else
  {

    double discounted = fare-(fare*.20);
    paymentUser=discounted;
    lcd.print(discounted);
    balance-=discounted;
  }
  lcd.setCursor(0,3);
  lcd.print("BALANCE:");
  lcd.setCursor(8,3);
  lcd.print(balance);

   url="GET /jt/public/users/queue/"+card_number+"/"+terminal_id+"/"+balance+"/"+paymentUser+" HTTP/1.1";
  Serial.print("URL");
  Serial.println(url);
  clientConnect();
  client.print(url);
  client.println();
  client.println("Host: 206.189.209.210");
  client.println("Connection: close");

  client.println();
  
  
  line = client.readStringUntil("}");
  int start = line.indexOf('{');
  line=line.substring(start,line.length());
  Serial.println(line);
    const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
  DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject& root = jsonBuffer.parseObject(line);
  queueNumber=root["last_queue"].as<char*>();
  client.flush();
  client.stop();
  discount=false;
  sms(phone,queueNumber);
  lcd.clear();
  initDisplay();
}

void information(String card_number){
  
  
}

void getJson(){
  
  //Serial.println(root["id"].as<char*>());
}

void terminalInformation(){
  url="GET /jt/public/terminal/information/"+terminal_id+" HTTP/1.1";
//  Serial.print("URL");
//  Serial.println(url);
////  //client.println(url);


  client.print(url);
  client.println();
  client.println("Host: 206.189.209.210");
  client.println("Connection: close");

  client.println();
  Serial.println("Response");

  line = client.readStringUntil("}");
  int start = line.indexOf('{');
  line=line.substring(start,line.length());
  Serial.println(line);
    const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
  DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject& root = jsonBuffer.parseObject(line);
  destination=root["terminal1"].as<char*>();
  source=root["terminal2"].as<char*>();
  title="From "+source+" to "+destination;
  //title+=" to ";
 // title+=root["terminal2"].as<char*());
 fare=String(root["fare"].as<char*>()).toDouble();
 Serial.print("FARE");
 Serial.println(fare);
 Serial.print("TITLE");
  Serial.println(title);
  client.flush();
  client.stop();
}

bool userInformation(String card_number){
  url="GET /jt/public/users/information/"+card_number+" HTTP/1.1";
  Serial.print("URL");
  Serial.println(url);
////  //client.println(url);


  client.print(url);
  client.println();
  client.println("Host: 206.189.209.210");
  client.println("Connection: close");

  client.println();
  Serial.println("Response");

  line = client.readStringUntil("}");
  client.flush();
  client.stop();
  int start = line.indexOf('{');
  line=line.substring(start,line.length());
  Serial.println(line);
  if((line.equals("{}"))||(line.equals("")))
  {
    Serial.println("empty");
    return false;
  }
    const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
  DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject& root = jsonBuffer.parseObject(line);
  String queue=root["queue"].as<char*>();
  Serial.print("queue");
  Serial.println(queue);
  if (!queue.equals("0"))
  {
      return false;
  }
 balance=root["bal"].as<double>();
 phone=root["phone"].as<char*>();
  
  Serial.print("balance");
  Serial.println(balance);
  Serial.print("phone");
  Serial.println(phone);
  
 return true;

}
void queueTerminal()
{
  clientConnect();
  url="GET /jt/public/terminal/queue/"+terminal_id+" HTTP/1.1";
  Serial.print("URL");
  Serial.println(url);


  client.print(url);
  client.println();
  client.println("Host: 206.189.209.210");
  client.println("Connection: close");

  client.println();
  client.flush();
  client.stop();
}
void queueSingle()
{
  clientConnect();
  url="GET /jt/public/users/queue/"+card_number+"/"+terminal_id+" HTTP/1.1";
  Serial.print("URL");
  Serial.println(url);


  client.print(url);
  client.println();
  client.println("Host: 206.189.209.210");
  client.println("Connection: close");

  client.println();
  client.flush();
  client.stop();
 

}
void resetTerminal()
{
  clientConnect();
  url="GET /jt/public/terminal/reset/"+terminal_id+" HTTP/1.1";
  Serial.print("URL");
  Serial.println(url);


  client.print(url);
  client.println();
  client.println("Host: 206.189.209.210");
  client.println("Connection: close");

  client.println();
  client.flush();
  client.stop();


}
void clientConnect()
{
  while(!client.connected())
  {
      client.connect("206.189.209.210", 80);
      

  }
}
void sms(String phoneNumber,String message)
{
  
       message="Your queue number is "+message;
       Serial.println("Sim808 init success");
       Serial.println("Start to send message ...");
       Serial.println(message);
       Serial.println(phoneNumber);
      sim808.sendSMS(phoneNumber.c_str(),message.c_str());    
}

void printNumber(){
printer.begin();        // Init printer (same regardless of serial type)
  printer.setSize('L');        // Set type size, accepts 'S', 'M', 'L'
printer.justify('C');
  printer.println("Your Queue\nNumber is:");
  //  printer.setLineHeight(50);
  printer.println(queueNumber+"\n");
  //printer.println();
    printer.printBitmap(logo_width, logo_height, logo_data);
printer.println();

//   printer.justify('C');
 // printer.printBitmap(80, 38, lynia);
    printer.feed(2);
   printer.sleep();      // Tell printer to sleep
  delay(3000L);         // Sleep for 3 seconds
  printer.wake();       // MUST wake() before printing again, even if reset
  printer.setDefault(); // Restore printer to defaults
}

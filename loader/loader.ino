#include <SPI.h>
#include <MFRC522.h>
#include <WiFiEsp.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <Wire.h> 
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#define RST_PIN         5          // Configurable, see typical pin layout above
#define SS_PIN          53   
char ssid[] = "PLDTHOMEDSL_EXT";     // your network SSID (name)
char pwd[] = "gahallon01";  // your network password
String line;
String url;


MFRC522 mfrc522(SS_PIN, RST_PIN);        
MFRC522::MIFARE_Key key;
int block=2;
byte readbackblock[18];
String card_number;
String balance;
WiFiEspClient client;

LiquidCrystal_I2C lcd(0x27, 20, 4);
const byte numRows= 4; 
const byte numCols= 4; 
char keymap[numRows][numCols]=
{
  {
    '1', '2', '3', 'A'  }
  ,
  {
    '4', '5', '6', 'B'  }
  ,
  {
    '7', '8', '9', 'C'  }
  ,
  {
    '*', '0', '#', 'D'  }
};


byte rowPins[numRows] = {36,34,32,30}; 
byte colPins[numCols]= {28,26,24,22}; 
Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);

void setup() {
  Serial.begin(19200);
  Serial1.begin(9600);
  lcd.begin();
 lcd.backlight();
 lcd.setCursor(1,0);
 lcd.print("Connecting to Wifi");
 WiFi.init(&Serial1);
 while (WiFi.status() != WL_CONNECTED)
 {
   WiFi.begin(ssid, pwd); 

 }
 lcd.clear();
 lcd.setCursor(1,0);
 lcd.print("Connecting to Host");
 clientConnect();
  lcd.clear();

  
 SPI.begin();               
 mfrc522.PCD_Init();

 for (byte i = 0; i < 6; i++) 
 {
  key.keyByte[i] = 0xFF;
 }




}
void loop()
{
  initDisplay();
  if ( !mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  readBlock(block, readbackblock);//read the block back
  card_number = (char*)readbackblock;
  char keypress = myKeypad.waitForKey();
  if (isDigit(keypress)){
    lcd.clear();
    initDisplay();
    line +=keypress;
    lcd.setCursor(0,3);
    lcd.print(line);
  }
  else if (keypress == 'A'){
    addBalance();
    lcd.clear();
  }
  else if (keypress == 'B'){
    Serial.println("clear");
    line=line.substring(0, line.length()-1);
    
    lcd.clear();
    initDisplay();
    lcd.setCursor(0,3);
    lcd.print(line);
  }

  else{
    //initDisplay();
  }

  
  
  
  
}

void clientConnect()
{
  while(!client.connected())
  {
      client.connect("206.189.209.210", 80);

  }
}
char waitNum()
{
  char keypress;
  
  keypress=myKeypad.waitForKey();
  while (isDigit(keypress)==false){
      keypress=myKeypad.waitForKey();
  }
  return keypress;
}

void initDisplay()
{
  lcd.setCursor(2,0);
  lcd.print("LYNIA - ADD LOAD");
  lcd.setCursor(8,1);
  lcd.print(card_number);
  lcd.setCursor(0,2);
  lcd.print("BALANCE:");
  lcd.setCursor(8,2);
  lcd.print(balance);
 
}
  
void addBalance()
{
  clientConnect();
  url="GET /jt/public/users/addBalance/"+card_number+"/"+line+" HTTP/1.1";
  Serial.print("URL");
  Serial.println(url);


  client.print(url);
  client.println();
  client.println("Host: 206.189.209.210");
  client.println("Connection: close");

  client.println();
  client.flush();
  client.stop();
  line="";

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

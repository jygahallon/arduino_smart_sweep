#include <SPI.h>
#include <MFRC522.h>
#include <WiFiEsp.h>
#include <ArduinoJson.h>

#define SS_PIN 53 
#define RST_PIN 5

char ssid[] = "apcmhi";     // your network SSID (name)
char pwd[] = "egi12345";  // your network password
String cardNumber[100];
String terminal_id="1";
String url;
String line;  
WiFiEspClient client;

MFRC522 mfrc522(SS_PIN, RST_PIN);        
MFRC522::MIFARE_Key key;
int block=2;
byte readbackblock[18];
String card_number;

bool queue=false;
int jeepCapacity=0;
String jeepId;
void setup(){
  Serial.begin(19200);
  Serial1.begin(9600);

  SPI.begin();               
  mfrc522.PCD_Init();
  for (byte i = 0; i < 6; i++) 
  {
    key.keyByte[i] = 0xFF;
    }
 WiFi.init(&Serial1);
 while (WiFi.status() != WL_CONNECTED)
 {
   WiFi.begin(ssid, pwd); 

 }
}

void loop(){
  if(!queue)
  {
    jeepCapacity=trackingJeep();
    if(jeepCapacity!=0)
    {
      Serial.print("capacity");
      Serial.println(jeepCapacity);
      getCardNumber();
      queue=true;
      return;
    }
  }
  else
  {
    //Serial.println("START QUEUE");
  
     if ( !mfrc522.PICC_IsNewCardPresent()) 
     {
      return;
      }
      if ( ! mfrc522.PICC_ReadCardSerial()) 
      {
        return;
      }
      readBlock(block, readbackblock);//read the block back
      card_number = (char*)readbackblock;
      Serial.println(card_number);
      return;
  }
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
void clientConnect()
{
  while(!client.connected())
  {
      client.connect("206.189.209.210", 80);

  }
}
int trackingJeep(){
  clientConnect();
  url="GET /jt/public/jeep/tracking/"+terminal_id+" HTTP/1.1";
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
    return 0;
  }
    
 const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
  DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject& root = jsonBuffer.parseObject(line);
  String on_location=root["on_location"].as<char*>();
  Serial.print("on_location");
  Serial.println(on_location);
  //return false;
  if (on_location.equals("0"))
  {
      return 0;
  }
  else
 {
    jeepId=root["id"].as<char*>();
    return root["capacity"].as<int>();
//    //capacity=(String)cap;
//    Serial.print("Capacity");
//    Serial.println(capacity);
//    return true;
 }
}

void getCardNumber()
{
  clientConnect();
   url="GET /jt/public/terminal/queueRide/"+terminal_id+"/"+jeepId+" HTTP/1.1";
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
   const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
  DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject& root = jsonBuffer.parseObject(line);

  for (int i=0;i<jeepCapacity;i++)
  {
    cardNumber[i]=root[String(i)].as<char*>();
    Serial.print(i);
    Serial.println(cardNumber[i]);
  }
  
  
}


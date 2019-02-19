
#include <SPI.h>//include the SPI bus library
#include <MFRC522.h>//include the RFID reader library

#define SS_PIN 53  //slave select pin
#define RST_PIN 5  //reset pin
MFRC522 mfrc522(SS_PIN, RST_PIN);        // instatiate a MFRC522 reader object.
MFRC522::MIFARE_Key key;//create a MIFARE_Key struct named 'key', which will hold the card information

void setup() 
{
  Serial.begin(9600);   
  SPI.begin();      
  mfrc522.PCD_Init();   
  Serial.println("Approximate your card to the reader...");
  Serial.println();
pinMode (6, OUTPUT);
pinMode (7, OUTPUT);
digitalWrite(7,LOW);
digitalWrite(6, LOW);
}
void loop() 
{
  String content= "";
  
  
  // Scan for cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Selects a card
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  //show UID on serial monitor
  Serial.print("UID tag :");

  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  if ((content.substring(1) == "C7 17 B0 79") || (content.substring(1) == "F0 A4 D0 57") ) //change the codes with the cards codes found after scanning in serial monitor
  {
   
    Serial.println("Authorized access");
    Serial.println();
    digitalWrite (7,HIGH);
    delay(3000);
    digitalWrite (7,LOW);
    delay(3000);
  }
 
 else   {
    Serial.println(" Access denied");
    digitalWrite (6,HIGH);
    delay(3000);
    digitalWrite (6, LOW);
        delay(3000);
  }
} 

#include <DFRobot_sim808.h>
#include <SoftwareSerial.h>
#define MESSAGE_LENGTH 160
//Mobile phone number,need to change
#define PHONE_NUMBER "09202868902"  
 
//The content of messages sent
#define MESSAGE  "hello,world"
char message[MESSAGE_LENGTH];
int messageIndex = 0;
char lat[12];
char lon[12];
char wspeed[12];

char phone[16];
char datetime[24];

#define PIN_TX    10
#define PIN_RX    11
SoftwareSerial mySerial(PIN_TX,PIN_RX);
DFRobot_SIM808 sim808(&mySerial);//Connect RX,TX,PWR,

void setup()
{
  mySerial.begin(9600);
  Serial.begin(9600);

  //******** Initialize sim808 module *************
  while(!sim808.init())
  {
      Serial.print("Sim808 init error\r\n");
      delay(1000);
  }
}

void loop()
{
  //*********** Detecting unread SMS ************************
   messageIndex = sim808.isSMSunread();

   //*********** At least, there is one UNREAD SMS ***********
   if (messageIndex > 0)
   { 
      Serial.print("messageIndex: ");
      Serial.println(messageIndex);
      
      sim808.readSMS(messageIndex, message, MESSAGE_LENGTH, phone, datetime);
                 
      //***********In order not to full SIM Memory, is better to delete it**********
      sim808.deleteSMS(messageIndex);
      Serial.print("From number: ");
      Serial.println(phone);  
      Serial.print("Datetime: ");
      Serial.println(datetime);        
      Serial.print("Recieved Message: ");
      Serial.println(message); 
    
     
      
   }
    
       Serial.println("Sim808 init success");
       Serial.println("Start to send message ...");
       Serial.println(MESSAGE);
       Serial.println(PHONE_NUMBER);
      sim808.sendSMS(PHONE_NUMBER,MESSAGE);      //************* Turn off the GPS power ************
}

#include <SoftwareSerial.h>
#define PIN_TX    10
#define PIN_RX    11
SoftwareSerial mySerial(PIN_TX,PIN_RX);
void setup() {
  //Begin serial comunication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(9600);
  while(!Serial);
   
  //Being serial communication witj Arduino and SIM800
  mySerial.begin(9600);
  delay(1000);
   
  Serial.println("Setup Complete!");
  Serial.println("Sending SMS...");
   
  //Set SMS format to ASCII
  mySerial.write("AT+CMGF=1\r\n");
  delay(1000);
 
  //Send new SMS command and message number
  ////mySerial.write("AT+CMGS=\"07194XXXXX\"\r\n");
  mySerial.print("AT+CMGS=\"");
  mySerial.print("09202868902");
  mySerial.print("\"\r");
   
  //Send SMS content
  mySerial.write("SMS from SIM800l Prueba");
  delay(1000);
   
  //Send Ctrl+Z / ESC to denote SMS message is complete
  mySerial.write((char)26);
  delay(1000);
     
  Serial.println("SMS Sent!");
}
 
void loop() {
}

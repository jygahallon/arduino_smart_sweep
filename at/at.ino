#include <SoftwareSerial.h>

#define PIN_TX    10
#define PIN_RX    11
SoftwareSerial mySerial(PIN_TX,PIN_RX);
void setup()
{
  Serial.begin(9600); // serial port used for debugging
  mySerial.begin(9600);  // your ESP's baud rate might be different
}
 
void loop()
{
  if(mySerial.available())  // check if the ESP is sending a message
  {
    while(mySerial.available())
    {
      int c = mySerial.read(); // read the next character
      Serial.write((char)c);  // writes data to the serial monitor
    }
  }
 
  if(Serial.available())
  {
    // wait to let all the input command in the serial buffer
    delay(10);

    // read the input command in a string
    String cmd = "";
    while(Serial.available())
    {
      cmd += (char)Serial.read();
    }

    // print the command and send it to the ESP
    Serial.println();
    Serial.print(">>>> ");
    Serial.println(cmd);

    // send the read character to the ESP
    mySerial.print(cmd);
  }
}

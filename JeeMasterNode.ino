#include <JeeLib.h>

String recvInput = "";
String serialInput = "";
boolean serialComplete = false;
byte needToSend = 0;
int targetTemp = 160;
int realTemp;

void setup () {
    Serial.begin(9600);
    rf12_initialize(10, RF12_868MHZ, 167);
}

void loop () {
  if (rf12_recvDone() && rf12_crc == 0) {
    for (byte i = 0; i < rf12_len; ++i){
      char inByte = (char)rf12_data[i];
      recvInput += inByte;
     }
     char bufTemp[rf12_len];
     recvInput.toCharArray(bufTemp, rf12_len);
     realTemp = strtod(bufTemp, NULL);
     Serial.println(realTemp/10);
     recvInput = "";
  }
  
  if(needToSend && rf12_canSend()){
    needToSend = 0;
    char rfTemp[4];
    dtostrf(targetTemp, 3, 0, rfTemp);
    rf12_sendStart(0, rfTemp, sizeof rfTemp);
  }
  
  if(serialComplete){  
    int bufLength = serialInput.length();
    char bufTemp[bufLength];
    serialInput.toCharArray(bufTemp, bufLength);
    targetTemp = strtod(bufTemp, NULL) * 10;
    serialInput = "";
    serialComplete = false;
    needToSend = 1;
  }
}
 
 void serialEvent(){
 while(Serial.available()){
    char inByte = (char)Serial.read();
      serialInput += inByte;
      if (inByte == '\n'){
        serialComplete = true;
    }
  }
 
}

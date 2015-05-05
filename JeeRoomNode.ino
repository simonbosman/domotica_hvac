//Rfm12b wireless lib/jeenode
#include <JeeLib.h>

Port portOne(1); //minirelais
DHTxx dht(7); //tempsensor
MilliTimer workTimer;int targetTemp = 160; //chosen temp * 10
int realTemp; //current temp * 10
int hum; //humidity in percentage * 10
int inByte = 0;
byte doWork = 0;
byte hvacOn = 0;
String serialInput = "";        
String rfInput = ""; 
boolean stringComplete = false;  

//Get a fresh reading from the DHT sensor and show some monitoring data
void refreshTempHum() {
  if (dht.reading(realTemp, hum)) {
    Serial.print("temperature = ");
    Serial.println(realTemp/10);
    Serial.print("target temperature = ");
    Serial.println(targetTemp/10);
    Serial.print("humidity = ");
    Serial.println(hum/10);
    Serial.print("hvac = ");
    (hvacOn) ? Serial.println("ON") : Serial.println("OFF");
    Serial.println();
    if(rf12_canSend()) {
      char rfTemp[4];
      dtostrf(realTemp, 3, 0, rfTemp);
      rf12_sendStart(0, rfTemp, sizeof rfTemp);
    }
  }
  else {
    Serial.println("Could not get an temperature reading");
  }
}

//Command the hvac switch on or of
//DHT sensor is only +/-2 degree sensitive 
void commandHvac() {
  if (realTemp < targetTemp){
    portOne.digiWrite2(HIGH);
    hvacOn = 1;  
  }
 
 if (realTemp >= targetTemp){
    portOne.digiWrite2(LOW);
    hvacOn = 0;
   }
}

void setup() {
  portOne.mode2(OUTPUT);
  Serial.begin(9600);
  Serial.println("Hvac control");
  serialInput.reserve(5);
  rfInput.reserve(5);
  rf12_initialize(1, RF12_868MHZ, 167);
}

void loop() {
  if (workTimer.poll(10000))
     doWork = 1;
     
  if (doWork){
     refreshTempHum();
     commandHvac();
     doWork = 0;
  }
 
  if (rf12_recvDone() && rf12_crc == 0) {
    for (int i = 0; i < rf12_len; ++i) {
       char inChar = (char)rf12_data[i];
       rfInput += inChar;
    }
    char bufTemp[rf12_len];
    rfInput.toCharArray(bufTemp, rf12_len);
    targetTemp = strtod(bufTemp, NULL);
    rfInput = ""; 
  }
 
  if (stringComplete) {
    int bufLength = serialInput.length(); 
    char bufTemp[bufLength];
    serialInput.toCharArray(bufTemp, bufLength);
    targetTemp = strtod(bufTemp, NULL) * 10;
    serialInput = "";
    stringComplete = false;
  }
}  
 
void serialEvent() {
    while (Serial.available()) {
      char inChar = (char)Serial.read(); 
      serialInput += inChar;
      if (inChar == '\n') {
        stringComplete = true;
      } 
    }
}


    




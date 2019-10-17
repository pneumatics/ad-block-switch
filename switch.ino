#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include <ArduinoJson.h>

ESP8266WiFiMulti WiFiMulti;
//WiFiClient client;    global is not working correctly 
//HTTPClient http;

unsigned long currentMillis;
 
//each new timer requires similar three lines of code
unsigned long millisUpdateSate;
unsigned long millisWiFiScan;

//boolean       repeat13    = true; //repeat this sequence?
//boolean       flag13      = true; //is this timer enabled?



boolean connectioWasAlive = true;
bool buttonPressed = false;


const int internetBtn = 12;
const int internetLED = 13;
const int adBtn = 4;
const int adLED = 5;


bool internetBtnState = 0;
bool internetLEDState = 0;
bool internetState = 0;

bool adBtnState = 0;
bool adLEDState = 0;
bool adFilterState = 0;

void ICACHE_RAM_ATTR toggleInternet(); //Place ISR in IRAM
void ICACHE_RAM_ATTR toggleAdFilter(); 

void setup() {

  Serial.begin(115200); // serial console set at 115200 baud
  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("ssid", "passwd"); // configure available one or more APs  

  pinMode(internetBtn, INPUT_PULLUP); // set the GPIO pin direction
  attachInterrupt(digitalPinToInterrupt(internetBtn), toggleInternet, FALLING);
  pinMode(adBtn, INPUT_PULLUP); // set the GPIO pin direction
  attachInterrupt(digitalPinToInterrupt(adBtn), toggleAdFilter, FALLING);
  
  pinMode(internetLED, OUTPUT);
  pinMode(adLED, OUTPUT);

}

int adFilterStatus(){

  WiFiClient client;
  HTTPClient http;
  
  String piholeUrl;
  piholeUrl = "http://192.168.0.102/admin/api.php?ad";

  Serial.printf("[HTTP] adFilterStatus(): ");
  
  if (http.begin(client,piholeUrl)) {  // HTTP

      int httpCode = http.GET();
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
          
          String payload = http.getString();
          //Serial.println(payload);
          const size_t capacity = JSON_OBJECT_SIZE(3) + 50;
          DynamicJsonDocument doc(capacity);

          DeserializationError error = deserializeJson(doc, payload);

          // Test if parsing succeeds.
//          if (error) {
//
//            http.end();
//            Serial.printf("0\n");
//            return 0;
//            
//          }
          
          String adStatus = doc["ad"];          
          if ( adStatus == "enabled") {
            
            http.end();
            Serial.printf("1\n");
            return 1;
          
          }         
         
        }
        
      } 
      
      else {
        
        Serial.printf("[HTTP] GET Error: %s\n", http.errorToString(httpCode).c_str());        
        
      }

      http.end();
      Serial.printf("0\n");
      return 0;
      
    }



    
    else {
      Serial.printf("[HTTP} Unable to connect\n");
      return 0;      
    }
}







int internetStatus(){


  WiFiClient client; 
  HTTPClient http;
  
  String internetUrl;
  internetUrl = "http://www.google.com";
  Serial.printf("[HTTP] internetStatus(): ");
  if (http.begin(client,internetUrl)) {  // HTTP

    int httpCode = http.GET();
    if (httpCode > 0) {
      //Serial.printf("[HTTP] GET response code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK) {

        http.end();
        Serial.printf("1\n");
        return 1;
      }
        
      } else {
        Serial.printf("[HTTP] GET Error: %s\n", http.errorToString(httpCode).c_str());        //debug
      }

      http.end();
      Serial.printf("0\n");
      return 0;
      
    }
    
    else {
      Serial.printf("[HTTP} Unable to connect\n");
      return 0;      
    }

    
}

int switchInternet(String power){

  WiFiClient client;
  HTTPClient http;
  
  String piholeUrl;
  piholeUrl = "http://192.168.0.102/admin/api.php?internet=" + power;

  Serial.print("[HTTP] Switching internet...\n");
    if (http.begin(client,piholeUrl)) {  // HTTP

      int httpCode = http.GET();
      if (httpCode > 0) {
        //Serial.printf("[HTTP] GET response code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          //Serial.println(payload);

          const size_t capacity = JSON_OBJECT_SIZE(3) + 50;
          DynamicJsonDocument doc(capacity);

          DeserializationError error = deserializeJson(doc, payload);

          // Test if parsing succeeds.
          if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.c_str());
          }


          String result = doc["result"];
          String internet = doc["internet"];
          //Serial.printf("[API] Result: %s\n",result.c_str());
          
          
          if ( result == "success") {

          //Serial.printf("Internet is %s\n", internet.c_str());
          http.end();
          Serial.printf("[API] Internet switched\n");
          internetState = internetBtnState;
          return 1;
          
          }         
         
        }
        
      } else {
        Serial.printf("\n[HTTP] GET Error: %s\n", http.errorToString(httpCode).c_str());        
      }

      http.end();
      return 0;
      
    }
    else {
      Serial.printf("[HTTP} Unable to connect\n");
      return 0;      
    }

    
}

int switchAdFilter(String power){

  WiFiClient client;
  HTTPClient http;
  String piholeUrl;
  piholeUrl = "http://192.168.0.102/admin/api.php?ad=" + power;

  Serial.print("[HTTP] Switching adFilter...\n");
    if (http.begin(client,piholeUrl)) {  // HTTP

      int httpCode = http.GET();
      if (httpCode > 0) {
        //Serial.printf("[HTTP] GET response code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          //Serial.println(payload);

          const size_t capacity = JSON_OBJECT_SIZE(3) + 50;
          DynamicJsonDocument doc(capacity);

          DeserializationError error = deserializeJson(doc, payload);

          // Test if parsing succeeds.
          if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.c_str());
          }


          String result = doc["result"];
          String ad = doc["ad"];
          //Serial.printf("[API] Result: %s\n",result.c_str());
          //Serial.printf("[API] Ad: %s\n",ad.c_str());
          
          if ( result == "success") {

          Serial.printf("[API] Ad Filter switched\n");
          http.end();
          adFilterState = adBtnState;
          return 1;
          
          }         
         
        }
        
      } else {
        Serial.printf("[HTTP] GET Error: %s\n", http.errorToString(httpCode).c_str());        
      }

      http.end();
      return 0;
      
    }
    else {
      Serial.printf("[HTTP} Unable to connect\n");
      return 0;      
    }

    
}



void toggleInternet(){

  static unsigned long elapsedInterruptTime0 = 0;
  unsigned long interruptTime = millis();
 
  if (interruptTime - elapsedInterruptTime0 > 5000)
  {
      internetBtnState = !internetBtnState;
      buttonPressed = true;
  }
  
  elapsedInterruptTime0 = interruptTime; 
  
}

void toggleAdFilter(){

  static unsigned long elapsedInterruptTime1 = 0;
  unsigned long interruptTime = millis();
 
  if (interruptTime - elapsedInterruptTime1 > 5000)
  {
      adBtnState = !adBtnState;
      buttonPressed = true;
  }
  
  elapsedInterruptTime1 = interruptTime; 
  
}

void updateState(){

//  static unsigned long elapsedInterruptTime2 = 0;
//  unsigned long interruptTime = millis();
// 
//  if ((interruptTime - elapsedInterruptTime2 > 5000) && )
//  {
//
//    Serial.println("updating..HTTP");
//    adFilterState = adFilterStatus();

//    if(adFilterStatus() != 1){
//        
//          adFilterState = 0;
//          
//        }
//      

//    internetState = internetStatus();       
//     
//  }
//  
//  elapsedInterruptTime2 = interruptTime; 

  int currentAdFilterState;
  int currentInternetState;

  if (WiFiMulti.run() == WL_CONNECTED && CheckTime(millisUpdateSate, 30*1000, true) && !buttonPressed)
  {

    Serial.println("\nUpdating Internet and AdFilter status");
    currentAdFilterState = adFilterStatus();
    currentInternetState = internetStatus(); 

    adFilterState = currentAdFilterState;
    adBtnState  = currentAdFilterState;

    internetState = currentInternetState;
    internetBtnState = currentInternetState;
    
  }
  
}

boolean CheckTime(unsigned long &lastMillis, unsigned long wait,boolean restart)
{
  //is the time up for this task?
  if (millis() - lastMillis >= wait)
  {
    //should this start again?
    if(restart)
    {
      //get ready for the next iteration
      lastMillis = millis(); //get ready for the next iteration
    }
    return true;
  }
  return false;
 
}





void monitorWiFi() // Monitor WiFi connection sttaus and reconnect if lost
{
  if(CheckTime(millisWiFiScan, 300, true)){
  
      if (WiFiMulti.run() != WL_CONNECTED )
      {
        if (connectioWasAlive == true)
        {
          connectioWasAlive = false;
          Serial.print("Searching for WiFi Access Point");
        }
        Serial.print(".");
        digitalWrite(internetLED, !digitalRead(internetLED));
        digitalWrite(adLED, !digitalRead(internetLED));
        
      }
      else if (connectioWasAlive == false)
      {
        connectioWasAlive = true;
        Serial.printf("\nConnected to Access Point: %s", WiFi.SSID().c_str());
        Serial.print("\nLocal IP address:");
        Serial.print(WiFi.localIP());
      }
  }
}
  
void updateLED(){
  
 if(connectioWasAlive){
  
  digitalWrite(internetLED, !internetState);
 
  digitalWrite(adLED, adFilterState);
 }
  
  }

  
void loop(){

  monitorWiFi();
  updateState();   
  updateLED();

  if (internetBtnState != internetState && buttonPressed){

      switchInternet(internetBtnState ? "enable" : "disable");
      buttonPressed = false;
    
    }

  if (adBtnState != adFilterState && buttonPressed){

      switchAdFilter(adBtnState ? "enable" : "disable");
      buttonPressed = false;
      
    
    }
 

  
  
  }

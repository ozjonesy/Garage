/*********
Garage Lights Sonoff

Luke Jones - https://github.com/ozjonesy/Garage

Credit - Rui Santos Complete project details at http://randomnerdtutorials.com
*********/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

MDNSResponder mdns;

// Replace with your network credentials
const char* ssid = "network";
const char* password = "password";
const char* boardName = "door";
const char* OTAPassword = "password";

ESP8266WebServer server(80);

int gpio13Led = 13;
int gpio12Relay = 12;
int gpio14Input = 14;
int gpio0Input = 0;
int switchState = HIGH;
WiFiClient client;

// List of other Sonoff's FQDN
const char* Front = "FrontLights.example.com";

void lightOn() { // Turns on the relay
  digitalWrite(gpio12Relay, HIGH);
  //HTTPGet(Front, "on");
}

void lightOnWeb() { //Calls turnOn and sends the web page for on status
  String onPage = "<!DOCTYPE html><html><head><style>body {background-color: #262730;}.button {display: inline-block; height: 300px;width: 300px; background-image: url('http://cdn.mysitemyway.com/etc-mysitemyway/icons/legacy-previews/icons/glossy-black-icons-symbols-shapes/018696-glossy-black-icon-symbols-shapes-power-button.png'); background-size: 300px 300px; cursor: pointer; background-color: #9b9b9b; border: none; border-radius: 50%; box-shadow: 0px 0px 100px #e8e09d; border: 3px solid #262626;} .button:active {box-shadow: 0px 0px 100px 5px #e8e09d; height: 290px;width: 290px;background-size: 290px 290px;}</style></head><body><div style=\"text-align:center;\"><p><a href=\"off\"><br><br><br><button class=\"button\"></button></a></p></div></body>";
  server.send(200, "text/html", onPage);
  lightOn();
}

void lightOff() { //Turns off the relay
  digitalWrite(gpio12Relay, LOW);
  //HTTPGet(Front,"off");
}

void lightOffWeb() { //Calls turnOff and sends the web page for off status
  String offPage = "<!DOCTYPE html><html><head><style>body {background-color: #262730;}.button {display: inline-block; height: 300px;width: 300px; background-image: url('http://cdn.mysitemyway.com/etc-mysitemyway/icons/legacy-previews/icons/glossy-black-icons-symbols-shapes/018696-glossy-black-icon-symbols-shapes-power-button.png'); background-size: 300px 300px; cursor: pointer; background-color: #9b9b9b; border: none; border-radius: 50%; box-shadow: 0px 0px 100px #000; border: 3px solid #262626;} .button:active {box-shadow: 0px 0px 100px 5px #000; height: 290px;width: 290px;background-size: 290px 290px;}</style></head><body><div style=\"text-align:center;\"><p><a href=\"on\"><br><br><br><button class=\"button\"></button></a></p></div></body>";
  server.send(200, "text/html", offPage);
  lightOff();
}

void statusWeb() { //Checks the relay status and sends the correct web page
  if (digitalRead(gpio12Relay) == HIGH) { //if relay is on
    String onPage = "<!DOCTYPE html><html><head><style>body {background-color: #262730;}.button {display: inline-block; height: 300px;width: 300px; background-image: url('http://cdn.mysitemyway.com/etc-mysitemyway/icons/legacy-previews/icons/glossy-black-icons-symbols-shapes/018696-glossy-black-icon-symbols-shapes-power-button.png'); background-size: 300px 300px; cursor: pointer; background-color: #9b9b9b; border: none; border-radius: 50%; box-shadow: 0px 0px 100px #e8e09d; border: 3px solid #262626;} .button:active {box-shadow: 0px 0px 100px 5px #e8e09d; height: 290px;width: 290px;background-size: 290px 290px;}</style></head><body><div style=\"text-align:center;\"><p><a href=\"off\"><br><br><br><button class=\"button\"></button></a></p></div></body>";
    server.send(200, "text/html", onPage);
  } 
  else {
    String offPage = "<!DOCTYPE html><html><head><style>body {background-color: #262730;}.button {display: inline-block; height: 300px;width: 300px; background-image: url('http://cdn.mysitemyway.com/etc-mysitemyway/icons/legacy-previews/icons/glossy-black-icons-symbols-shapes/018696-glossy-black-icon-symbols-shapes-power-button.png'); background-size: 300px 300px; cursor: pointer; background-color: #9b9b9b; border: none; border-radius: 50%; box-shadow: 0px 0px 100px #000; border: 3px solid #262626;} .button:active {box-shadow: 0px 0px 100px 5px #000; height: 290px;width: 290px;background-size: 290px 290px;}</style></head><body><div style=\"text-align:center;\"><p><a href=\"on\"><br><br><br><button class=\"button\"></button></a></p></div></body>";
    server.send(200, "text/html", offPage);
  }
}

void HTTPGet(const char* site, char* action){
  client.connect(site, 80);
    client.print(String("GET /") + action + " HTTP/1.1\r\n" +
             "Host: " + site + "\r\n" +
             "Connection: close\r\n" +
             "\r\n"
            );
    client.flush();
    client.stop();
}

void flashLed(int numFlash) { //flashes the LED numFlash times and returns it to previous state
  for (int count = 0; count < numFlash; count++) {
    digitalWrite(gpio13Led, HIGH); //LED OFF
    delay(200);
    digitalWrite(gpio13Led, LOW); //LED ON
  }
}

void checkSwitch() {
    //Check GPIO14 for external switch
  int currentState = digitalRead(gpio14Input); //read external switch state
  if (switchState != currentState) { // compare this switch state to the last read state, if not the same then do stuff
    if (currentState == HIGH){ // state is HIGH when not grounded ie. external switch off
      lightOff();
      switchState = currentState;
    }
    else {
      lightOn();
      switchState = currentState;
      int count = 0; //counter for number of loops
      int flips = 1; //number of times the switch has been flipped
      bool keepLooping = true;
      int loopState = LOW;
      while (keepLooping) { //Reset loop
        int loopCurrentState = digitalRead(gpio14Input);
        if (flips >= 6){
          flashLed(2);
          lightOff();
          ESP.restart();
        }
        else if (count > 200) {
           keepLooping = false;
           digitalWrite(gpio13Led, HIGH); //LED OFF
           delay(200);
           digitalWrite(gpio13Led, LOW); //LED ON
        }
        else if (loopState != loopCurrentState) {
          flips ++;
          count = 0;
          loopState = loopCurrentState;
        }
        count++;
        delay(5);
      }
    }
  }
}

void checkButton() {
    // Check GPIO0 for internal switch
  int counter = 1;
  if (digitalRead(gpio0Input) == LOW) {
    if (digitalRead(gpio12Relay) == HIGH) {
      lightOff();
    }
    else {
      lightOn();
    }

    while (digitalRead(gpio0Input) == LOW) {
      delay(100);
      counter ++;
    }
  }
  if (counter >= 50) { 
    flashLed(4);
    ESP.restart();
  }
}

void setup(void){
  // preparing GPIOs
  pinMode(gpio13Led, OUTPUT);
  digitalWrite(gpio13Led, HIGH);
  
  pinMode(gpio12Relay, OUTPUT);
  digitalWrite(gpio12Relay, LOW);
  pinMode(gpio14Input, INPUT_PULLUP);
  pinMode(gpio0Input, INPUT_PULLUP);
 
  Serial.begin(115200);
  Serial.println("start setup"); 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  WiFi.hostname(boardName);
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(gpio13Led, LOW);
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //OTA
  ArduinoOTA.setHostname(boardName);
  ArduinoOTA.setPassword(OTAPassword);
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  
  if (mdns.begin(boardName, WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }
  
  Serial.println("Ready");

  server.on("/", statusWeb);
  
  server.on("/on", lightOnWeb);
  
  server.on("/off", lightOffWeb);
  
  server.begin();
  Serial.println("HTTP server started");
}
 
void loop(void){
  ArduinoOTA.handle();
  
  server.handleClient(); //do web server stuff

  checkSwitch();

  checkButton();
} 


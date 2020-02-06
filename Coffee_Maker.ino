#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

WiFiClient homeClient;
PubSubClient client(homeClient);

const char* ssid = "MySpectrumWiFib8-2G";
const char* password = "livelywhale398";

const char* mqtt_server = "192.168.1.241";

const int coffee = D1;

int millisSinceOn = 0;
int currentMillis = 0;
const int milliThreshold = 480000;

void setup(void){
  pinMode(coffee, OUTPUT);
  digitalWrite(coffee, HIGH);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  client.publish("test","connected",true);
}

void loop(void){
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("Coffee_Maker");

  if(digitalRead(coffee) == LOW){
    currentMillis = millis();
    if((currentMillis - millisSinceOn) >= milliThreshold){
      Serial.println("Turning coffee maker off for safety");
      digitalWrite(coffee, HIGH);
      millisSinceOn = 0;
    }  
  }

}

void setup_wifi(){
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

}

void callback(String topic, byte* message, unsigned int length){
  
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.println("Deserializing JSON...");

  StaticJsonDocument<256> jsonDoc;
  
  deserializeJson(jsonDoc, message, length);
  
  if(topic == "/logansroom/coffee_maker"){
    Serial.print("Coffee_Maker Connected...");
    if(jsonDoc["command"] == "on"){
      Serial.println("Brewing a cup o' joe...");
      digitalWrite(coffee, LOW);
      millisSinceOn = millis();
    }
    else if(jsonDoc["command"] == "off"){
      Serial.println("Done Brewing...");
      digitalWrite(coffee,HIGH);
      millisSinceOn = 0;
    }
  }
  
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Coffee_Maker")) {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("/logansroom/coffee_maker");
      client.publish("/logansroom/", "connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

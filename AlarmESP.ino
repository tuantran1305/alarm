#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

#define WIFI_AP "ROOM VAN HOA"
#define WIFI_PASSWORD "roomvanhoa123"

char Thingsboard_Server[] = "54.151.151.78";
#define User_name "0RL3IdhTH6GUWzOtiaY0"
WiFiClient wifiClient;

int status = WL_IDLE_STATUS;

PubSubClient client(wifiClient);

boolean gpioStatus = false;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(10);
  pinMode(D2,OUTPUT);
  digitalWrite(D2,HIGH);
  InitWiFi();
  client.setServer(Thingsboard_Server,1883);
  client.setCallback(callback);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(!client.connected()){
        reconnect();
    }
    client.loop();
}
void callback(const char* topic, byte* payload, unsigned int length){
    //Serial.println("On message");
    char json[length + 1];
    strncpy (json, (char*)payload, length);
    json[length] = '\0';
  
    Serial.println("Topic: ");
    Serial.println(topic);
    
    Serial.println("Message: ");
    
    Serial.println(json);
    
    StaticJsonBuffer<200> jsonBuffer;
    
    JsonObject  &data = jsonBuffer.parseObject((char*)json);
    
    if (!data.success()){
    Serial.println("parseObject() failed");
    return;
    }
    String methodName = String((const char*)data["method"]);
    if(methodName.equals("R")){
      if(data["params"]==true){
          digitalWrite(D2,LOW);
          gpioStatus=true;
          }
      else{
          digitalWrite(D2,HIGH);
          gpioStatus=false;
          }
    }
    client.publish("v1/devices/me/attributes", get_gpio_status().c_str()); 
  }
void reconnect(){
// Loop until we're reconnected
  while (!client.connected()){
    status = WiFi.status();
    if( status != WL_CONNECTED){
        WiFi.begin(WIFI_AP, WIFI_PASSWORD);
        while (WiFi.status() != WL_CONNECTED){
        delay(500);
        Serial.print(".");
      }
    Serial.println("Connected to AP");
    }
  Serial.print("Connecting to Thingsboard node ...");
// Attempt to connect (clientId, username, password)
  if(client.connect("rms00000000",User_name,NULL)){
    Serial.println( "[DONE]" );
    client.subscribe("v1/devices/me/rpc/request/+");//dang ky nhan cas lenh rpc tu cloud
    client.publish("v1/devices/me/attributes", get_gpio_status().c_str());
    } 
  else{
    Serial.print( "[FAILED] [ rc = " );
    Serial.print( client.state());
    Serial.println(" : retrying in 5 seconds]");
    // Wait 5 seconds before retrying
    delay( 5000 );
    }
  }
}
String get_gpio_status(){
  // Prepare gpios JSON payload string
  StaticJsonBuffer<200> jsonBuffer;
  
  JsonObject& data = jsonBuffer.createObject();
  
  data["Alarm"]=gpioStatus ? true : false;
  char payload[256];
  data.printTo(payload, sizeof(payload));
  String strPayload = String(payload);
  Serial.println("Get gpio status: ");
  Serial.println(strPayload);
  return strPayload;
}
void InitWiFi(){
  Serial.println("Connecting to AP ...");
// attempt to connect to WiFi network
  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
    }
  Serial.println("Connected to AP");
}

#include <Arduino.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <Servo.h> 
#include <string>

using namespace std;

WiFiClient client;
PubSubClient mqtt_client(client); 
Servo myservo;

const char* ssid = "Spy";
const char* password = "tamsonam";

const char* server_mqtt = "192.168.156.88";
const int port_mqtt = 1883;
const char* mqtt_id = "esp8266";

const char* host = "www.pushsafer.com";
const int port = 80;
const char* request = "/api?k=XA65jek3QmBl6zDHODeI&d=83274&m=Hello%20t%E1%BB%AB%20device";

const char* topic_subscribe = "to-esp8266";
const char* topic_publish = "from-esp8266";
const double dis = 15;

int statusLed = 0;
int foodAmount = 0;
int flagServo = 0;
int statusFoodInt = 0;
double lastDistance = 0;

const unsigned long interval = 2000; // Khoảng thời gian 5 giây (5000 milliseconds)
unsigned long previousMillis = 0; // Thời gian lần cuối đèn LED được bật/tắt
String lastStatusFood = "0";
String statusFoodStr = "0";

void sendRequest() {
  WiFiClient client;
  while(!client.connect(host, port)) {
    Serial.println("connection fail");
    delay(1000);
  }
  client.print(String("GET ") + request + " HTTP/1.1\r\n"
              + "Host: " + host + "\r\n"
              + "Connection: close\r\n\r\n");
  delay(500);

  while(client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
}

double getPercentFood(){
  digitalWrite(D5, LOW);
  delayMicroseconds(2);
  digitalWrite(D5, HIGH);
  delayMicroseconds(10);
  digitalWrite(D5, LOW);

  double duration = pulseIn(D6, HIGH);
  double distanceCM = duration * 0.034 / 2;

  if(flagServo == 1){
    if(distanceCM > lastDistance) distanceCM = lastDistance;
  }
  if (distanceCM == 0) {
    distanceCM = lastDistance;
    return distanceCM;
  }
  
  lastDistance = distanceCM;
  return distanceCM;
}

void convertPercentFoodToStr(){
  String percentFoodStr = "0";
  double distanceCM = getPercentFood();
  Serial.println(distanceCM);
  if(distanceCM >= dis){
    percentFoodStr = "0";
  }
  else if(distanceCM >= dis - 4.5){
    if(distanceCM <= dis - 3.5){
      double tmp = dis - 3.5 - distanceCM;
      percentFoodStr = String(int(tmp * 25) + 75);
    }
    else if(distanceCM <= dis - 2.5){
      double tmp = dis - 2.5 - distanceCM;
      percentFoodStr = String(int(tmp * 25) + 50);
    }
    else if(distanceCM <= dis - 1.5){
      double tmp = dis - 1.5 - distanceCM;
      percentFoodStr = String(int(tmp * 25) + 25);
    }
    else if(distanceCM <= dis){
      double tmp = dis - distanceCM;
      percentFoodStr = String(int(tmp * 25));
    }
  }
  else if(distanceCM >= 9.5){
    percentFoodStr = "100";
  }
  else{
    percentFoodStr = lastStatusFood; // Trường hợp mèo che cảm biến
  }

  lastStatusFood = percentFoodStr;
  statusFoodStr = percentFoodStr; // Lượng % thức ăn kiểu string
}

void callBack(char* topic, uint8_t* payload, unsigned int length){
  String s = "";
  
  Serial.print("Received from: ");
  Serial.println(topic);
  Serial.print("Message: ");
  for(size_t i = 0; i < length; i++){
    s += (char)payload[i];
  }

  Serial.println(s);
  foodAmount = s.toInt();  // Convert String to int
  flagServo = 1; // Để servo chạy
  Serial.println();
  Serial.println("- - - - - - - - - - - - - - - -");
}

void wifiConnect() {
  WiFi.begin(ssid, password);
  // WiFi.reconnect();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);    
    Serial.print(".");
  }
  Serial.print("Connected to Wifi ");
  Serial.println(ssid);
  Serial.print("IP Address: ");
  Serial.print(WiFi.localIP());
  Serial.println();
  delay(1000);
}

void mqttConnect(){
  mqtt_client.setServer(server_mqtt, port_mqtt);
  mqtt_client.setCallback(callBack);

  Serial.println("Connecting to MQTT");
  while(!mqtt_client.connect(mqtt_id))
    delay(500);
  Serial.println("Connected to MQTT");
  mqtt_client.subscribe(topic_subscribe);
}

void setup() {
  Serial.begin(9600);
  Serial.print("Connecting to WiFi");
  
  pinMode(D5, OUTPUT);  // trig_pin 
	pinMode(D6, INPUT);   // echo_pin
  myservo.attach(D7);


  wifiConnect();
  mqttConnect();
  // sendRequest();
}

void loop() {
  mqtt_client.loop();
  unsigned long currentMillis = millis();
  convertPercentFoodToStr();
  statusFoodInt = atoi(statusFoodStr.c_str());

  Serial.println(statusFoodInt);
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    mqtt_client.publish(topic_publish, statusFoodStr.c_str());
  }

  if(flagServo == 1){
    if(statusFoodInt < foodAmount){
      myservo.write(180);
      delay(500);
      convertPercentFoodToStr();
      statusFoodInt = atoi(statusFoodStr.c_str());
      Serial.println(statusFoodInt);
      mqtt_client.publish(topic_publish, statusFoodStr.c_str());
      myservo.write(0);
      delay(500);
    }
    else flagServo = 0;
  }
}
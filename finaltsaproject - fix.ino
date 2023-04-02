#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

#define pinTrigIn 18
#define pinEchoIn 19

#define pinTrigOut 4
#define pinEchoOut 2

#define pinRelay1 27
#define pinRelay2 23

#define pinBuz 25

#define pinLDR 13

int totalColumns = 16;
int totalRows = 2;

LiquidCrystal_I2C lcd(0x27, totalColumns, totalRows);  

String staticMessage = "Pengunjung";
String scrollingMessage = "Sekarang";

long duration; 
int distance;

int camera = 0;
String StrCamera = "";

// Konfigurasi Access Point
const char* ssid = "JTI-POLINEMA";
const char* password = "jtifast!";

// Konfigurasi Thingsboard
const char* usermqtt = "zwlrpnko:zwlrpnko";
const char* passmqtt = "VhM5RTgFuKVMudvzrAoRZk5ebITarytH";
const char* mqtt_server = "armadillo.rmq.cloudamqp.com";

unsigned long lastMsg = 0;
const char* topic = "jmlPelanggan";

WiFiClient espClient;
PubSubClient client(espClient);
DynamicJsonDocument dataEncode(1024);

float readDistanceIn(){
  digitalWrite(pinTrigIn, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(pinTrigIn, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrigIn, LOW);
  
  int duration = pulseIn(pinEchoIn, HIGH);

  // Calculating the distance
  return duration * 0.0344 / 2;
}

float readDistanceOut(){
  digitalWrite(pinTrigOut, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(pinTrigOut, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrigOut, LOW);
  
  int duration = pulseIn(pinEchoOut, HIGH);

  // Calculating the distance
  return duration * 0.0344 / 2;
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }  

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), usermqtt, passmqtt)) {
      Serial.println("Connected");
      client.subscribe("python/test");
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    StrCamera = StrCamera + (char)payload[i];
  }
  Serial.println();
  camera = StrCamera.toInt();
  StrCamera = "";
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(pinTrigIn, OUTPUT);
  pinMode(pinEchoIn, INPUT);
  pinMode(pinTrigOut, OUTPUT);
  pinMode(pinEchoOut, INPUT);
  pinMode(pinRelay1,OUTPUT);
  pinMode(pinRelay2,OUTPUT);
  pinMode(pinBuz, OUTPUT);
  digitalWrite(pinBuz, LOW);
  lcd.init();                    
  lcd.backlight();

  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float LDR_val = digitalRead(pinLDR);
  Serial.print("Lux :");
  Serial.println(LDR_val);
  
  int jarakIn = readDistanceIn();
  Serial.print("Jarak Masuk :");
  Serial.println(jarakIn);

  int jarakOut = readDistanceOut();
  Serial.print("Jarak Keluar :");
  Serial.println(jarakOut);

  if (jarakIn < 10){
    Serial.println("Masuk!");
    digitalWrite(pinBuz, HIGH);
    delay(600);
    digitalWrite(pinBuz, LOW);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(staticMessage);
    lcd.setCursor(0, 1);
    lcd.print("Masuk!");
    delay(1000);
    lcd.clear();
    client.publish(topic, "Masuk!");
  }

  
  if (jarakOut < 10){
    Serial.println("Keluar!");
    digitalWrite(pinBuz, HIGH);
    delay(600);
    digitalWrite(pinBuz, LOW);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(staticMessage);
    lcd.setCursor(0, 1);
    lcd.print("Keluar!");
    delay(1000);
    lcd.clear();
    client.publish(topic, "Keluar!");
  }

  if (digitalRead(pinLDR) == 1) {
    digitalWrite(pinRelay1, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Lampu 1");
    lcd.setCursor(0, 1);
    lcd.print("Menyala");
    lcd.clear();
    if (camera > 3) {
      digitalWrite(pinRelay2, HIGH);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Lampu 2");
      lcd.setCursor(0, 1);
      lcd.print("Menyala");
      lcd.clear();
    } else if(camera <= 3) {
      digitalWrite(pinRelay2, LOW);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Lampu 2");
      lcd.setCursor(0, 1);
      lcd.print("Menyala");
      lcd.clear();
    }
  } else {
    digitalWrite(pinRelay1, LOW);
    digitalWrite(pinRelay2, LOW);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Lampu");
    lcd.setCursor(0, 1);
    lcd.print("Mati");
    lcd.clear();
  }

  lcd.setCursor(0, 0);
  lcd.print(staticMessage);
  lcd.setCursor(0, 1);
  lcd.print("Sekarang = " + String(camera) + "          ");

  delay(1000);
}

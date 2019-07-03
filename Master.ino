

// EspnowTwoWayController.ino

// a minimal program derived from
//          https://github.com/HarringayMakerSpace/ESP-Now

// This is the program that sends the data and receives the reply. (The Controller)

//=============

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_HDC1000.h>

extern "C" {
#include <espnow.h>
}

// this is the MAC Address of the slave which receives these sensor readings
uint8_t remoteMac[] = {0x36, 0x33, 0x33, 0x33, 0x33, 0x33};
uint8_t mac[] = {0x36, 0x33, 0x33, 0x33, 0x33, 0x33};

#define WIFI_CHANNEL 4
// must match slave struct
struct __attribute__((packed)) DataStruct {
  char text[32];
  unsigned long time;
};

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_HDC1000 hdc = Adafruit_HDC1000();

DataStruct sendingData;

DataStruct receivedData;
// receivedData could use a completely different struct as long as it matches
//   the reply that is sent by the slave
//const char* wifi_ssid = "Garagem";
//const char* wifi_password = "Garagem@2018";
const char* wifi_ssid = "aula-ic3";
const char* wifi_password = "iotic@2019";
#define mqtt_server "iot.eclipse.org"
unsigned long lastSentMillis;
unsigned long sendIntervalMillis = 1000;
unsigned long sentMicros;
unsigned long ackMicros;
unsigned long replyMicros;

unsigned long counter = 0;

byte ledPin = 14;

boolean messageReceived = false;
char dados[32];

//==============

void setup() {
  Serial.begin(115200); Serial.println();
  Serial.println("Starting EspnowTwoWayController.ino");

  WiFi.mode(WIFI_STA); // Station mode for esp-now controller
  wifi_set_macaddr(SOFTAP_IF, &mac[0]);
  WiFi.disconnect();

  Serial.printf("This mac: %s, ", WiFi.macAddress().c_str());
  Serial.printf("target mac: %02x%02x%02x%02x%02x%02x", remoteMac[0], remoteMac[1], remoteMac[2], remoteMac[3], remoteMac[4], remoteMac[5]);
  Serial.printf(", channel: %i\n", WIFI_CHANNEL);

  if (esp_now_init() != 0) {
    Serial.println("*** ESP_Now init failed");
    ESP.restart();
    while (true) {};
  }
  // role set to COMBO so it can send and receive - not sure this is essential
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  esp_now_add_peer(remoteMac, ESP_NOW_ROLE_COMBO, WIFI_CHANNEL, NULL, 0);


  esp_now_register_send_cb(sendCallBackFunction);
  esp_now_register_recv_cb(receiveCallBackFunction);

  strcpy(sendingData.text, "Ola slave, mande os dados!");
  Serial.print("Message "); Serial.println(sendingData.text);

  Serial.println("Setup finished");

}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.printf("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.publish("mc857/topic", dados); 
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//==============

void loop() {
  while (!messageReceived) {
    sendData();
    delay(3000);
  }
  if (!client.connected()) {
    setup_wifi();
    Serial.println("reconnect");
    client.setServer(mqtt_server, 1883);
    reconnect();
  }
  
  client.loop();
  //client.publish("mc857/topic", dados);
  Serial.printf("Dados publicados no MQTT");
  WiFi.disconnect();
  counter = counter + 1;
  delay(3000);
  messageReceived = false;
  ESP.restart();
}

//==============

void sendData() {
  if (millis() - lastSentMillis >= sendIntervalMillis) {
    lastSentMillis += sendIntervalMillis;
    sendingData.time = millis();
    uint8_t byteArray[sizeof(sendingData)];
    memcpy(byteArray, &sendingData, sizeof(sendingData));
    sentMicros = micros();
    esp_now_send(NULL, byteArray, sizeof(sendingData)); // NULL means send to all peers
    Serial.println("Loop sent data");
  }
}

//==============

void sendCallBackFunction(uint8_t* mac, uint8_t sendStatus) {
  ackMicros = micros();
  Serial.print("Trip micros "); Serial.println(ackMicros - sentMicros);
  Serial.printf("Send status = %i", sendStatus);
  Serial.println();
}

//================

void receiveCallBackFunction(uint8_t *senderMac, uint8_t *incomingData, uint8_t len) {
  replyMicros = micros();
  Serial.print("Reply Trip micros "); Serial.println(replyMicros - sentMicros);
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  Serial.print("NewReply ");
  Serial.print("MacAddr ");
  for (byte n = 0; n < 6; n++) {
    Serial.print (senderMac[n], HEX);
  }
//  Serial.print("  MsgLen ");
//  Serial.print(len);
  Serial.print("  Dados ");
  Serial.print(receivedData.text);
  strcpy(dados, receivedData.text);
  
  Serial.print("  Time ");
  Serial.print(receivedData.time);
  Serial.println();
  Serial.println();
  
  messageReceived = true;
}

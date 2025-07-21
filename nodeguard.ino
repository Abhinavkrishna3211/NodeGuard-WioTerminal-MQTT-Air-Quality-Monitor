#include <PubSubClient.h>
#include <rpcWiFi.h>
#include <SensirionI2CSht4x.h>
#include <Wire.h>
#include "sensirion_common.h"
#include "sgp30.h"
#include <TFT_eSPI.h>
#include "seeed_bme680.h"
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
#define IIC_ADDR  uint8_t(0x76)
Seeed_BME680 bme680(IIC_ADDR); 



TFT_eSPI tft;
TFT_eSprite spr = TFT_eSprite(&tft);
SensirionI2CSht4x sht4x;
int randnumber;
const char *ssid = "****";      // your network SSID
const char *password = "******";  // your network password
const char *ID = "node4";               // Name of our device, must be unique
const char *server = "test.mosquitto.org"; // Server URL
WiFiClient wifiClient;
PubSubClient client(wifiClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
float no2 = 0.1;

const float VRefer = 3.3; // Voltage of ADC reference

const int pinAdc = A0;   // Analog pin connected to the Grove Gas Sensor (O2)

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
   // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect 
    if (client.connect(ID)) {
      Serial.println("connected");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() {
  Serial.begin(115200);
   Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

   // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    WiFi.begin(ssid, password);
    // wait 1 second for re-trying
    delay(1000);
  }

  Serial.print("Connected to ");
  Serial.println(ssid);
  delay(500);
  client.setServer(server, 1883);
  client.setCallback(callback);

   pinMode(WIO_BUZZER, OUTPUT);
pinMode(WIO_5S_PRESS, INPUT_PULLUP);

 
      // while (!Serial);
    Serial.println("Serial start!!!");
    delay(100);
    while (!bme680.init()) {
        Serial.println("bme680 init failed ! can't find device!");
        delay(10000);
    }


  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setFreeFont(&FreeSansBoldOblique18pt7b);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Surveillance bot", 30, 10 , 1);
  //Line
  for (int8_t line_index = 0; line_index < 5 ; line_index++)
  {
    tft.drawLine(0, 50 + line_index, tft.width(), 50 + line_index, TFT_GREEN);
  }
  
  tft.drawRoundRect(5, 60, (tft.width() / 2) - 20 , tft.height() - 65 , 10, TFT_WHITE); 
  //tVCO Text
  tft.setFreeFont(&FreeSansBoldOblique12pt7b);
  tft.setTextColor(TFT_RED);
  tft.drawString("tVOC", 7 , 65 , 1);
  tft.setTextColor(TFT_GREEN);
  tft.drawString("ug/m3", 55, 108, 1);
  //CO2e Text
  tft.setFreeFont(&FreeSansBoldOblique12pt7b);
  tft.setTextColor(TFT_RED);
  tft.drawString("CO2e", 7 , 150 , 1);
  tft.setTextColor(TFT_GREEN);
  tft.drawString("ppm", 55, 193, 1);
  // Temp rect
  tft.drawRoundRect((tft.width() / 2) - 10  , 60, (tft.width() / 2) / 2 , (tft.height() - 65) / 2 , 10, TFT_BLUE); 
  tft.setFreeFont(&FreeSansBoldOblique9pt7b);
  tft.setTextColor(TFT_RED) ;
  tft.drawString("Temp", (tft.width() / 2) - 1  , 70 , 1); 
  tft.setTextColor(TFT_GREEN);
  tft.drawString("o", (tft.width() / 2) + 30, 95, 1);
  tft.drawString("C", (tft.width() / 2) + 40, 100, 1);
  //o2 rect
  tft.drawRoundRect(((tft.width() / 2) + (tft.width() / 2) / 2) - 5  , 60, (tft.width() / 2) / 2 , (tft.height() - 65) / 2 , 10, TFT_BLUE); 
  tft.setFreeFont(&FreeSansBoldOblique9pt7b);
  tft.setTextColor(TFT_RED);
  tft.drawString("O2", ((tft.width() / 2) + (tft.width() / 2) / 2)   , 70 , 1); 
  tft.setTextColor(TFT_GREEN);
  tft.drawString("%", ((tft.width() / 2) + (tft.width() / 2) / 2) + 30 , 120, 1);
  //Humi Rect
  tft.drawRoundRect((tft.width() / 2) - 10 , (tft.height() / 2) + 30, (tft.width() / 2) / 2 , (tft.height() - 65) / 2 , 10, TFT_BLUE); 
  tft.setFreeFont(&FreeSansBoldOblique9pt7b);
  tft.setTextColor(TFT_RED) ;
  tft.drawString("Humi", (tft.width() / 2) - 1 , (tft.height() / 2) + 40 , 1); 
  tft.setTextColor(TFT_GREEN);
  tft.drawString("%", (tft.width() / 2) + 30, (tft.height() / 2) + 70, 1);
  //methane Rect
  tft.drawRoundRect(((tft.width() / 2) + (tft.width() / 2) / 2) - 5  , (tft.height() / 2) + 30, (tft.width() / 2) / 2 , (tft.height() - 65) / 2 , 10, TFT_BLUE); 
  tft.setFreeFont(&FreeSansBoldOblique9pt7b);
  tft.setTextColor(TFT_RED) ;
  tft.drawString("Methane", ((tft.width() / 2) + (tft.width() / 2) / 2)   , (tft.height() / 2) + 40 , 1); 
  tft.setTextColor(TFT_GREEN);
  tft.drawString("ppm", ((tft.width() / 2) + (tft.width() / 2) / 2) + 30 , (tft.height() / 2) + 90, 1);

    delay(100);
  
// Wait for Serial to be ready
 
  Wire.begin();
  uint16_t error;
  char errorMessage[256];
  sht4x.begin(Wire);
  uint32_t serialNumber;
  
  s16 err;
  u16 scaled_ethanol_signal, scaled_h2_signal;
  Serial.println("serial start!!");
  while (sgp_probe() != STATUS_OK) {
    Serial.println("SGP failed");
    while (1);
  }
  err = sgp_measure_signals_blocking_read(&scaled_ethanol_signal, &scaled_h2_signal);
  if (err == STATUS_OK) {
    Serial.println("get ram signal!");
  } else {
    Serial.println("error reading signals");
  }
  err = sgp_iaq_init();

  Serial.println("Grove - Gas Sensor Test Code...");
}

void loop() {
  s16 err = 0;
  u16 tvoc_ppb, co2_eq_ppm;
  err = sgp_measure_iaq_blocking_read(&tvoc_ppb, &co2_eq_ppm);
  if (err == STATUS_OK) {
  // Successfully read IAQ values  
  } else {
    Serial.println("error reading IAQ values\n");
  }

  uint16_t error;
  char errorMessage[256];
  float temperature;
  float humidity;
  float randnumbr;
  float concentration;
  float randco;
  float voc;

  char envDataBuf[1000];
  randnumber = random(290, 300);
  randnumbr = random(1.00, 2.00);
  randco = random(2,3);

  Serial.print("tvoc: ");
  Serial.print(tvoc_ppb);
  voc=tvoc_ppb/1000;

bme680.read_sensor_data();
   
 // Read oxygen concentration from Grove Gas Sensor (O2)
  float conc = readConcentration();
   concentration=conc;
 

  // Convert button state to 1 (activated) or 0 (not activated)
  int buttonState = (digitalRead(WIO_5S_PRESS) == LOW) ? 1 : 0;
  int toxic;
  if(co2_eq_ppm>900){
          analogWrite(WIO_BUZZER, 128);
          tft.fillScreen(TFT_RED);
          tft.setFreeFont(&FreeSansBoldOblique24pt7b);
          tft.setTextColor(TFT_WHITE);
          tft.drawString("DANGER", 65, 100 , 1);
          delay(500);
          analogWrite(WIO_BUZZER, 0);
          tft.fillScreen(TFT_BLACK);
          delay(500);
          toxic=1;
          
      }
  else{
    toxic=0;


  tft.setFreeFont(&FreeSansBoldOblique18pt7b);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Surveillance bot", 30, 10 , 1);
  //Line
  for (int8_t line_index = 0; line_index < 5 ; line_index++)
  {
    tft.drawLine(0, 50 + line_index, tft.width(), 50 + line_index, TFT_GREEN);
  }
 
  tft.drawRoundRect(5, 60, (tft.width() / 2) - 20 , tft.height() - 65 , 10, TFT_WHITE); 
 //tVCO Text
  tft.setFreeFont(&FreeSansBoldOblique12pt7b);
  tft.setTextColor(TFT_RED);
  tft.drawString("tVOC", 7 , 65 , 1);
  tft.setTextColor(TFT_GREEN);
  tft.drawString("ug/m3", 55, 108, 1);
 //CO2e Text
  tft.setFreeFont(&FreeSansBoldOblique12pt7b);
  tft.setTextColor(TFT_RED);
  tft.drawString("CO2e", 7 , 150 , 1);
  tft.setTextColor(TFT_GREEN);
  tft.drawString("ppm", 55, 193, 1);
 // Temp rect
  tft.drawRoundRect((tft.width() / 2) - 10  , 60, (tft.width() / 2) / 2 , (tft.height() - 65) / 2 , 10, TFT_BLUE); // s1
  tft.setFreeFont(&FreeSansBoldOblique9pt7b);
  tft.setTextColor(TFT_RED) ;
  tft.drawString("Temp", (tft.width() / 2) - 1  , 70 , 1);
  tft.setTextColor(TFT_GREEN);
  tft.drawString("o", (tft.width() / 2) + 30, 95, 1);
  tft.drawString("C", (tft.width() / 2) + 40, 100, 1);
  //o2 rect
  tft.drawRoundRect(((tft.width() / 2) + (tft.width() / 2) / 2) - 5  , 60, (tft.width() / 2) / 2 , (tft.height() - 65) / 2 , 10, TFT_BLUE); // s2
  tft.setFreeFont(&FreeSansBoldOblique9pt7b);
  tft.setTextColor(TFT_RED);
  tft.drawString("O2", ((tft.width() / 2) + (tft.width() / 2) / 2)   , 70 , 1); 
  tft.setTextColor(TFT_GREEN);
  tft.drawString("%", ((tft.width() / 2) + (tft.width() / 2) / 2) + 30 , 120, 1);
  //Humi Rect
  tft.drawRoundRect((tft.width() / 2) - 10 , (tft.height() / 2) + 30, (tft.width() / 2) / 2 , (tft.height() - 65) / 2 , 10, TFT_BLUE); // s3
  tft.setFreeFont(&FreeSansBoldOblique9pt7b);
  tft.setTextColor(TFT_RED) ;
  tft.drawString("Humi", (tft.width() / 2) - 1 , (tft.height() / 2) + 40 , 1); 
  tft.setTextColor(TFT_GREEN);
  tft.drawString("%", (tft.width() / 2) + 30, (tft.height() / 2) + 70, 1);
  //methane Rect
  tft.drawRoundRect(((tft.width() / 2) + (tft.width() / 2) / 2) - 5  , (tft.height() / 2) + 30, (tft.width() / 2) / 2 , (tft.height() - 65) / 2 , 10, TFT_BLUE); // s4
  tft.setFreeFont(&FreeSansBoldOblique9pt7b);
  tft.setTextColor(TFT_RED) ;
  tft.drawString("Methane", ((tft.width() / 2) + (tft.width() / 2) / 2)   , (tft.height() / 2) + 40 , 1); 
  tft.setTextColor(TFT_GREEN);
  tft.drawString("ppm", ((tft.width() / 2) + (tft.width() / 2) / 2) + 30 , (tft.height() / 2) + 90, 1);
    // tVOC
   
  Serial.print("tVOC: ");
  Serial.print(randnumber);
  Serial.println(" ppm");
  spr.createSprite(40, 30);
  spr.fillSprite(TFT_BLACK);
  spr.setFreeFont(&FreeSansBoldOblique12pt7b);
  spr.setTextColor(TFT_WHITE);
  spr.drawNumber(randnumber, 0, 0, 1);
  spr.pushSprite(15, 100);
  spr.deleteSprite();
 
  //CO2
  Serial.print("CO2e: ");
  Serial.print(co2_eq_ppm);
  Serial.println(" ppm");
  spr.createSprite(40, 30);
  spr.setFreeFont(&FreeSansBoldOblique12pt7b);
  spr.setTextColor(TFT_WHITE);
  spr.drawNumber(co2_eq_ppm, 0, 0, 1);
  spr.setTextColor(TFT_GREEN);
  spr.pushSprite(15, 185);
  spr.deleteSprite();
  
//Temp
  Serial.print("Temperature: ");
  Serial.print(bme680.sensor_result_value.temperature);
  Serial.println( "*C");
  spr.createSprite(30, 30);
  spr.setFreeFont(&FreeSansBoldOblique12pt7b);
  spr.setTextColor(TFT_WHITE);
  spr.drawNumber(bme680.sensor_result_value.temperature, 0, 0, 1);
  spr.setTextColor(TFT_GREEN);
  spr.pushSprite((tft.width() / 2) - 1, 100);
  spr.deleteSprite();
 //O2

  Serial.print("O2: ");
  Serial.print(concentration);
  Serial.println(" %");
  spr.createSprite(45, 30);
  spr.setFreeFont(&FreeSansBoldOblique12pt7b);
  spr.setTextColor(TFT_WHITE);
  spr.drawNumber(concentration, 0, 0, 1);
  spr.pushSprite(((tft.width() / 2) + (tft.width() / 2) / 2), 97);
  spr.deleteSprite();
 //Humidity
  Serial.print("Humidity: ");
  Serial.print(bme680.sensor_result_value.humidity);
  Serial.println( "%");
  spr.createSprite(30, 30);
  spr.setFreeFont(&FreeSansBoldOblique12pt7b);
  spr.setTextColor(TFT_WHITE);
  spr.drawNumber(bme680.sensor_result_value.humidity, 0, 0, 1);
  spr.pushSprite((tft.width() / 2) - 1, (tft.height() / 2) + 67);
  spr.deleteSprite();
 //Methane
  Serial.print("Methane: ");
  Serial.print(randnumbr);
  Serial.println(" ppm");
  spr.createSprite(45, 30);
  spr.setFreeFont(&FreeSansBoldOblique12pt7b);
  spr.setTextColor(TFT_WHITE);
  spr.drawNumber(randnumbr, 0 , 0, 1);
  spr.pushSprite(((tft.width() / 2) + (tft.width() / 2) / 2), (tft.height() / 2) + 67);
  spr.deleteSprite();
  
  }


  sprintf(envDataBuf, "{\"temp\":%f,\n\"hum\":%f,\n\"tVOC_C\":%d,\n\"CO2_C\":%d,\n\"button\":%d,\n\"o2_concentration\":%.2f,\n\"toxic\":%d,\n\"Methane\":%f,\n\"NO2\":%d,\n\"Pressure\":%f,\n\"CO\":%f}", 
          bme680.sensor_result_value.temperature, bme680.sensor_result_value.humidity, randnumber, co2_eq_ppm, buttonState, concentration,toxic,randnumbr,no2,bme680.sensor_result_value.pressure / 1000.0,randco);

  


  



  Serial.println(envDataBuf);
  delay(500);

  if (!client.connected()) {
    reconnect();
  }

  client.loop();
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    client.publish("aTopic", envDataBuf);
  }
}

float readO2Vout() {
  long sum = 0;
  for (int i = 0; i < 32; i++) {
    sum += analogRead(pinAdc);
  }

  sum >>= 5;

  float MeasuredVout = sum * (VRefer / 1023.0);
  return MeasuredVout;
}

float readConcentration() {
  // Vout samples are with reference to 3.3V
  float MeasuredVout = readO2Vout();

  //when its output voltage is 2.0V,
  float Concentration = MeasuredVout * 0.21 / 2.0;
  float Concentration_Percentage = Concentration * 100;
  return Concentration_Percentage;
}
#include <SPI.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// sensor DHT 22 – temperature and humidity
#define DHTPIN D8
#define DHTTYPE DHT22   
DHT dht(DHTPIN, DHTTYPE); 
float hum1;
float temp1; 

// sensor BME 280 – temperature, humidity and atmospheric pressure
#define BME280_ADDRESS (0x76)
Adafruit_BME280 bme;
float hum2;
float temp2;
float press2;

// sensor DS18B20 – temperature
OneWire oneWireDS(4);
DallasTemperature ds(&oneWireDS);
float temp3;

// WiFi client
const char* wifiName = "";
const char* wifiPass = "";

// HTTP client
const char* serverHost = "http://my-meteostation.herokuapp.com/metrics";

// time
int timestamp;
                                
void setup()
{
  // init serial monitor
  Serial.begin(9600);

  // init WiFi
  WiFi.begin(wifiName, wifiPass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("WiFi connecting...");
  }


  // init sensors
  dht.begin();   
  bme.begin(BME280_ADDRESS);
  ds.begin();

}

void loop()
{
  // read values from sensors
  hum1 = dht.readHumidity();  
  temp1 = dht.readTemperature();             

  hum2 = bme.readHumidity();
  temp2 = bme.readTemperature();
  press2 = bme.readPressure() / 100.0F;
  
  ds.requestTemperatures();
  temp3 = ds.getTempCByIndex(0);

  // serialize data to JSON
  DynamicJsonDocument doc(1024);
  doc["t1"] = temp1;
  doc["t2"] = temp2;
  doc["t3"] = temp3;
  doc["h1"] = hum1;
  doc["h2"] = hum2;
  doc["p1"] = press2;
  String metricsJSON;
  serializeJson(doc, metricsJSON);
  Serial.println(metricsJSON);
  Serial.println();

  // send data to server
  if ((WiFi.status() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    http.begin(client, serverHost); //HTTP
    http.addHeader("Content-Type", "application/json");

    Serial.print("[HTTP] POST...\n");
    // start connection and send HTTP header and body
    int httpCode = http.POST(metricsJSON);

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        const String& payload = http.getString();
        Serial.println("received payload:\n<<");
        Serial.println(payload);
        Serial.println(">>");
      }
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }

  delay(10000);
  
  // print values to serial monitor
  Serial.print("T: ");
  Serial.print(temp1);
  Serial.print("/");
  Serial.print(temp2);
  Serial.print("/");
  Serial.print(temp3);
  Serial.print(" ");
  Serial.println("C");

  Serial.print("V: ");
  Serial.print(hum1);
  Serial.print("/");
  Serial.print(hum2);
  Serial.println(" %");
  
  Serial.print("Tl: ");
  Serial.print(press2);
  Serial.println(" hPa.");

}

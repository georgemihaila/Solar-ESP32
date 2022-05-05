#include <ArduinoJson.h>
#include <WiFi.h>
#include <WebServer.h>
#include "ESP32SSDP.h"
#include <DHT.h>

const char *ssid = "";
const char *password = "";

#define VINPIN 32
#define SOLARPIN 33
#define GAS_1 34
#define GAS_2 35

#define DHTPIN 32
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

WebServer server(80);

void setup()
{
  pinMode(VINPIN, INPUT);
  pinMode(SOLARPIN, INPUT);
  pinMode(GAS_1, INPUT);
  pinMode(GAS_2, INPUT);
  dht.begin();

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());

  server.on("/description.xml", handleSSDP);
  server.on("/capabilities", handleListCapabilities);
  server.on("/sensors", handleListSensors);
  server.on("/read", handleReading);
  server.onNotFound(handle_NotFound);
  server.begin();

  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName("ESP32-Sensors-Board");
  SSDP.setSerialNumber("d5696fa3a861feeba4ad4a6364e7cc1d");
  SSDP.setModelName("ESP32 multi sensors board v0.2a");
  SSDP.setModelNumber("ESP32MSB221");
  SSDP.setDeviceType("upnp:rootdevice");
  SSDP.begin();
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }
    Serial.println(WiFi.localIP());
  }
  server.handleClient();
}

void handleListCapabilities()
{
  const size_t capacity = JSON_ARRAY_SIZE(1);
  DynamicJsonDocument doc(capacity);
  doc.add("sensors");
  String result;
  serializeJson(doc, result);
  server.setContentLength(result.length());
  server.send(200, "application/json", result);
}

void handleListSensors()
{
  StaticJsonDocument<96> doc;
  doc.add("Voltage");
  doc.add("Gas");
  doc.add("Temperature");
  doc.add("Humidity");
  String result;
  serializeJson(doc, result);
  server.setContentLength(result.length());
  server.send(200, "application/json", result);
}

double getBatteryVoltage()
{
  return analogRead(VINPIN) / 563.877;
}

double getSolarPanelVoltage()
{
  return analogRead(SOLARPIN) / 199.753;
}

int getGas1Reading()
{
  return analogRead(GAS_1);
}

int getGas2Reading()
{
  return analogRead(GAS_2);
}

void handleReading()
{
  String result;
  StaticJsonDocument<768> doc;

  JsonArray Readings = doc.createNestedArray("Readings");

  JsonObject Readings_0 = Readings.createNestedObject();
  Readings_0["physicalSensor"] = "voltage";

  JsonArray Readings_0_logicalSensors = Readings_0.createNestedArray("logicalSensors");

  JsonObject Readings_0_logicalSensors_0 = Readings_0_logicalSensors.createNestedObject();
  Readings_0_logicalSensors_0["name"] = "BatteryVoltage";
  Readings_0_logicalSensors_0["value"] = getBatteryVoltage();

  JsonObject Readings_0_logicalSensors_1 = Readings_0_logicalSensors.createNestedObject();
  Readings_0_logicalSensors_1["name"] = "SolarPanelVoltage";
  Readings_0_logicalSensors_1["value"] = getSolarPanelVoltage();

  JsonObject Readings_1 = Readings.createNestedObject();
  Readings_1["physicalSensor"] = "gas";

  JsonArray Readings_1_logicalSensors = Readings_1.createNestedArray("logicalSensors");

  JsonObject Readings_1_logicalSensors_0 = Readings_1_logicalSensors.createNestedObject();
  Readings_1_logicalSensors_0["name"] = "gas1";
  Readings_1_logicalSensors_0["value"] = getGas1Reading();

  JsonObject Readings_1_logicalSensors_1 = Readings_1_logicalSensors.createNestedObject();
  Readings_1_logicalSensors_1["name"] = "gas2";
  Readings_1_logicalSensors_1["value"] = getGas2Reading();

  JsonObject Readings_2 = Readings.createNestedObject();
  Readings_2["physicalSensor"] = "dht22";

  JsonArray Readings_2_logicalSensors = Readings_2.createNestedArray("logicalSensors");

  JsonObject Readings_2_logicalSensors_0 = Readings_2_logicalSensors.createNestedObject();
  Readings_2_logicalSensors_0["name"] = "temperature";
  Readings_2_logicalSensors_0["value"] = dht.readTemperature();

  JsonObject Readings_2_logicalSensors_1 = Readings_2_logicalSensors.createNestedObject();
  Readings_2_logicalSensors_1["name"] = "humidity";
  Readings_2_logicalSensors_1["value"] = dht.readHumidity();

  serializeJson(doc, result);
  server.setContentLength(result.length());
  server.send(200, "application/json", result);
}

void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}

void handleSSDP()
{
  SSDP.schema(server.client());
}
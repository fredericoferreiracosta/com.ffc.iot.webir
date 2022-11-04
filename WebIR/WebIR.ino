#include <IRremote.hpp>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#include "Configuration.h"

WiFiClientSecure client;
HTTPClient http;

void setup()
{
    Serial.begin(115200);

    setupWifi();
    setupIR();

    client.setFingerprint(SHA1_FINGERPRINT);
}

void loop()
{
    delay(UPDATE_INTERVAL);
    DynamicJsonDocument commands = getNextCommandForTV();

    for (int i = 0; i < commands.size(); i++)
    {
        String action = commands[i]["action"];
        String id = commands[i]["_id"];

        handleTVCommand(action);
        flushCommand(id);
    }
}

DynamicJsonDocument getNextCommandForTV()
{
    Serial.println("Getting next command for TV");
    http.begin(client, HOST + "/commands?device=tv");

    int responseCode = http.sendRequest("GET");
    if (responseCode == HTTP_CODE_OK)
    {
        String response = http.getString();
        Serial.println(response);

        return deserialize(response);
    }
    else
    {
        Serial.println("Error on HTTP request. Error code: " + String(responseCode));
    }

    http.end();
}

void flushCommand(String id) {
    Serial.println("Flushing command " + id);
    http.begin(client, HOST + "/commands/" + id);

    int responseCode = http.sendRequest("DELETE");
    if (responseCode != HTTP_CODE_OK)
    {
        Serial.println("Error on HTTP request. Error code: " + String(responseCode));
    }

    http.end();
}

void setupWifi()
{
    Serial.println("Connecting to " + String(WIFI_SSID) + ".");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.println("Waiting to connect...");
    }

    Serial.println("Connected! IP address: " + WiFi.localIP().toString() + ".");
}

void setupIR()
{
    IrSender.begin(IR_SEND_PIN, ENABLE_LED_FEEDBACK);
}

void handleTVCommand(String command)
{
    if (command == "off")
    {
        turnOffTV();
    }
}

void turnOffTV()
{
    Serial.println("Turning off TV.");
    IrSender.sendNEC(TURN_OFF_TV, 32);
}

DynamicJsonDocument deserialize(String json)
{
  Serial.println("Deserializing JSON: " + json);
  DynamicJsonDocument doc(JSON_SIZE_LIMIT);

  deserializeJson(doc, json);

  return doc;
}

#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include "driver/adc.h"
#include <esp_wifi.h>
#include <esp_bt.h>

#define SENSOR_PIN 2
#define WIFI_TIMEOUT 10000 // 10seconds in milliseconds
#define RAINING GPIO_INTR_LOW_LEVEL
#define SUNNY GPIO_INTR_HIGH_LEVEL

// Initialize Wifi connection to the router
const char* ssid = "BelongA5465A";     // your network SSID (name)
const char* password = "nvxjh9nrypzd"; // your network key
String iftttEventPath = "http://maker.ifttt.com/trigger/rain_detected/with/key/dSYBSCF4exP6Bk8vmw8ChJ81OM7mw4elKfuSBHjFfrx";
char serverAddress[] = "http://maker.ifttt.com";
bool notificationSent = false;
WiFiClient wifi;

void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_PIN, INPUT);
  
  notificationSent = false;
  setCpuFrequencyMhz(80);
  
  sleepUntil(RAINING);
}

void loop() {
  delay(2000);
  
  Serial.println("Woke up!");  
  
  bool isRaining = getIsRaining();
  
  if(isRaining) {
    Serial.println("It is raining");
  } else {
    Serial.println("It is sunny");
  }

  if (isRaining) {
    sendRainNotification();
    
    Serial.println("Sleep until it is sunny");
    
    sleepUntil(SUNNY);
  } else {
    Serial.println("Sleep until it is raining");

    sleepUntil(RAINING);    
  }
}

bool getIsRaining()
{
  return digitalRead(SENSOR_PIN) == 0;
}

void sendRainNotification() {
  int err = 0;
  int responseCode = 0;
  HTTPClient http;

  connectToWiFi();

  if (WiFi.status() != WL_CONNECTED) {
    return;    
  }

  http.begin(iftttEventPath.c_str());

  Serial.println("Sending notification");
 
  responseCode = http.GET();

  if (responseCode <= 0) {
    Serial.print("Responded with error code ");
    Serial.println(responseCode);      
  }

  Serial.print("Responded with http code ");
  Serial.println(responseCode);
}

void connectToWiFi()
{
  Serial.print("Connecting to WiFi... ");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

	// Keep track of when we started our attempt to get a WiFi connection
  unsigned long startAttemptTime = millis();

  // Keep looping while we're not connected AND haven't reached the timeout
  while (WiFi.status() != WL_CONNECTED && 
          millis() - startAttemptTime < WIFI_TIMEOUT){
    delay(10);
  }

  // Make sure that we're actually connected, otherwise go to deep sleep
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("FAILED");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void sleepUntil(gpio_int_type_t wakeOnLevel)
{
  Serial.println("Going to sleep...");
  Serial.flush();

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  btStop();

  adc_power_off();
  esp_wifi_stop();
  esp_bt_controller_disable();
  
  gpio_wakeup_enable(GPIO_NUM_2, wakeOnLevel);
  esp_sleep_enable_gpio_wakeup();
  
  esp_light_sleep_start(); 
}
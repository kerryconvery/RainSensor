#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "driver/adc.h"
#include <esp_wifi.h>
#include <esp_bt.h>
#include "time.h"

#define SENSOR_PIN 2
#define WIFI_TIMEOUT 10000 // 10seconds in milliseconds
#define RAINING GPIO_INTR_LOW_LEVEL
#define SUNNY GPIO_INTR_HIGH_LEVEL

// Initialize Wifi connection to the router
const char* ssid = "";     // your network SSID (name)
const char* password = ""; // your network key
String iftttEventPath = "";
char serverAddress[] = "http://maker.ifttt.com";
bool notificationSent = false;
WiFiClient wifi;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 39600;
const int   daylightOffset_sec = 39600;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

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
    notifyOfRain();
    
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

void notifyOfRain() {
  Serial.println("nofity of rain");
  
  connectToWiFi();

  if (!isWifiConnected()) {
    Serial.println("Wifi not connected");
    return;    
  }

  if (!isTimeToNotify()) {
    Serial.println("It is not time to notify");
    return;
  }
  
  Serial.println("Will send rain notification");
  
  sendRainNotification();
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

bool isWifiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

bool isTimeToNotify() {
  tm* currentTime = getCurrentTime();

  Serial.printf("The Current time is: %02d:%02d:%02d\n", currentTime->tm_hour, currentTime->tm_min, currentTime->tm_sec);

  Serial.print("The current hour is: ");
  Serial.println(currentTime->tm_hour);

  return (currentTime->tm_hour >= 8 && currentTime->tm_hour <= 18);
}

tm* getCurrentTime() {
  initTimeClient();
  
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }

  time_t currentTime = timeClient.getEpochTime();

  return localtime(&currentTime);
}

void initTimeClient() {
  timeClient.begin();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);  
}

void sendRainNotification() {
  int err = 0;
  int responseCode = 0;
  HTTPClient http;

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
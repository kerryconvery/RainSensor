#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <BlynkSimpleEsp32.h>

#define SENSOR_PIN 2

// Initialize Wifi connection to the router
const char* ssid = "BelongA5465A";     // your network SSID (name)
const char* password = "nvxjh9nrypzd"; // your network key
String iftttEventPath = "http://maker.ifttt.com/trigger/rain_detected/with/key/dSYBSCF4exP6Bk8vmw8ChJ81OM7mw4elKfuSBHjFfrx";
char serverAddress[] = "http://maker.ifttt.com";
bool notificationSent = false;

/*WiFiClientSecure client;*/
WiFiClient wifi;

void setup() {

  Serial.begin(115200);
  pinMode(SENSOR_PIN, INPUT);
  // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  notificationSent = false;
}

void loop() {
  bool isRaining = digitalRead(SENSOR_PIN) == 0;
  
  Serial.print("Is raining ");
  Serial.println(isRaining);

  if (!isRaining) {
    notificationSent = false;
  }

  if (isRaining && !notificationSent) {
    sendRainNotification();
    notificationSent = true;
  }

  delay(1000);  
}

bool sendRainNotification() {
  int err = 0;
  int responseCode = 0;
  HTTPClient http;

  http.begin(iftttEventPath.c_str());

  Serial.println("Sending notification");
 
  responseCode = http.GET();

  if (responseCode <= 0) {
    Serial.print("Responded with error code ");
    Serial.println(responseCode);
    return false;        
  }

  Serial.print("Responded with http code ");
  Serial.println(responseCode);
    
  return true;  
}
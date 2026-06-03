#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);   // 必须先启用 WiFi
  delay(100);

  Serial.print("STA MAC: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
}
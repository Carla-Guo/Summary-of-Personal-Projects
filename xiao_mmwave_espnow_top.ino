#include <WiFi.h>
#include <esp_now.h>
#include <HardwareSerial.h>
#include <mmwave_for_xiao.h>

/*
   Top radar node

   Purpose:
   - Read the upper mmWave radar every 100ms.
   - Send the radar packet to the bottom controller by ESP-NOW.

   Important fix:
   - Do not use UART0 for the radar when Serial is used for debug logs.
   - Do not force enableEngineeringModel() at startup or every loop.
     If the radar UART is wrong or the radar is not ready, that library call can
     wait internally for a long time and trigger a reboot/watchdog reset.
*/


/* =====================================================
   Radar UART
   ===================================================== */
// Seeed's mmwave_for_xiao example uses D2 as RX and D3 as TX.
// If your wiring is different, only change these two pins.
const int RADAR_RX_PIN = D2;
const int RADAR_TX_PIN = D3;
const unsigned long RADAR_POLL_MS = 100;
const unsigned long DEBUG_POLL_MS = 1000;

HardwareSerial COMSerial(1);


/* =====================================================
   Debug Serial
   ===================================================== */
#define ShowSerial Serial


/* =====================================================
   Radar config
   ===================================================== */
Seeed_HSP24 xiao_config(COMSerial);
Seeed_HSP24::RadarStatus radarStatus;

// Arduino .ino 会自动生成函数声明；这里手写一份，避免自定义 enum 参数被提前声明错。
const char* targetStatusToString(Seeed_HSP24::TargetStatus status);


/* =====================================================
   ESP-NOW peer: bottom XIAO MAC address
   ===================================================== */
uint8_t peerAddress[] = {0xE4, 0xB3, 0x23, 0xB5, 0x17, 0x3C};

typedef struct struct_message {
  uint8_t targetStatus;
  uint16_t distance;
  uint8_t radarMode;
  uint8_t photosensitive;
  uint16_t moveGate[9];
  uint16_t staticGate[9];
} struct_message;

struct_message radarPacket;

unsigned long lastRadarPollAt = 0;
unsigned long lastDebugAt = 0;
bool espNowReady = false;


/* =====================================================
   Helpers
   ===================================================== */
bool elapsed(unsigned long now, unsigned long since, unsigned long intervalMs) {
  return now - since >= intervalMs;
}

bool isValidRadarFrame() {
  return radarStatus.targetStatus != Seeed_HSP24::TargetStatus::ErrorFrame;
}

void clearRadarPacket() {
  memset(&radarPacket, 0, sizeof(radarPacket));
  radarPacket.targetStatus = (uint8_t)Seeed_HSP24::TargetStatus::ErrorFrame;
}

void fillRadarPacket() {
  radarPacket.targetStatus = (uint8_t)radarStatus.targetStatus;
  radarPacket.distance = (uint16_t)max(radarStatus.distance, 0);
  radarPacket.radarMode = (uint8_t)max(radarStatus.radarMode, 0);
  radarPacket.photosensitive = (uint8_t)constrain(radarStatus.photosensitive, 0, 255);

  for (int i = 0; i < 9; i++) {
    radarPacket.moveGate[i] = (uint16_t)max(radarStatus.radarMovePower.moveGate[i], 0);
    radarPacket.staticGate[i] = (uint16_t)max(radarStatus.radarStaticPower.staticGate[i], 0);
  }
}

void printRadarDebug(unsigned long now) {
  if (!elapsed(now, lastDebugAt, DEBUG_POLL_MS)) return;
  lastDebugAt = now;

  if (!isValidRadarFrame()) {
    ShowSerial.println("Radar: no valid frame");
    return;
  }

  ShowSerial.print("Radar: ");
  ShowSerial.print(targetStatusToString(radarStatus.targetStatus));
  ShowSerial.print(" distance=");
  ShowSerial.print(radarStatus.distance);
  ShowSerial.print(" mode=");
  ShowSerial.println(radarStatus.radarMode);
}


/* =====================================================
   ESP32 Arduino Core 3.x send callback
   ===================================================== */
void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  ShowSerial.print("ESP-NOW Send: ");
  ShowSerial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}


/* =====================================================
   ESP-NOW init
   ===================================================== */
void initESPNow() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    ShowSerial.println("ESP-NOW Init Failed");
    espNowReady = false;
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    ShowSerial.println("Failed to add peer");
    espNowReady = false;
    return;
  }

  espNowReady = true;
  ShowSerial.println("ESP-NOW Ready");
}


/* =====================================================
   Radar task: run every 100ms
   ===================================================== */
void readAndSendRadar(unsigned long now) {
  if (!elapsed(now, lastRadarPollAt, RADAR_POLL_MS)) return;
  lastRadarPollAt = now;

  radarStatus = xiao_config.getStatus();
  printRadarDebug(now);

  if (!espNowReady || !isValidRadarFrame()) return;

  fillRadarPacket();

  esp_err_t result = esp_now_send(peerAddress, (uint8_t *)&radarPacket, sizeof(radarPacket));
  if (result != ESP_OK) {
    ShowSerial.print("ESP-NOW send call failed: ");
    ShowSerial.println(result);
  }
}


/* =====================================================
   setup
   ===================================================== */
void setup() {
  ShowSerial.begin(9600);
  COMSerial.begin(9600, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN);

  clearRadarPacket();

  ShowSerial.println("Programme Starting!");
  ShowSerial.print("Radar UART RX=");
  ShowSerial.print(RADAR_RX_PIN);
  ShowSerial.print(" TX=");
  ShowSerial.println(RADAR_TX_PIN);

  initESPNow();
}


/* =====================================================
   loop
   ===================================================== */
void loop() {
  unsigned long now = millis();
  readAndSendRadar(now);
}


/* =====================================================
   Status parser
   ===================================================== */
const char* targetStatusToString(Seeed_HSP24::TargetStatus status) {
  switch (status) {
    case Seeed_HSP24::TargetStatus::NoTarget:
      return "NoTarget";
    case Seeed_HSP24::TargetStatus::MovingTarget:
      return "MovingTarget";
    case Seeed_HSP24::TargetStatus::StaticTarget:
      return "StaticTarget";
    case Seeed_HSP24::TargetStatus::BothTargets:
      return "BothTargets";
    case Seeed_HSP24::TargetStatus::ErrorFrame:
      return "ErrorFrame";
    default:
      return "Unknown";
  }
}

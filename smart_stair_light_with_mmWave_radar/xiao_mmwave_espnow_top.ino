#include <WiFi.h>
#include <esp_now.h>
#include <HardwareSerial.h>
#include <mmwave_for_xiao.h>

// =====================
// UART1 replaces SoftwareSerial
// =====================
HardwareSerial COMSerial(1);

// Debug Serial
#define ShowSerial Serial

// Radar config
Seeed_HSP24 xiao_config(COMSerial);
Seeed_HSP24::RadarStatus radarStatus;

// =====================================================
// ESP-NOW settings (fill in receiver XIAO MAC address)
// =====================================================
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

// =====================================================
// [Update 1] ESP32 Arduino Core 3.x callback signature changed
// Previously const uint8_t* mac_addr
// Now must use wifi_tx_info_t*
// =====================================================
void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  ShowSerial.print("ESP-NOW Send: ");
  ShowSerial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

// =====================================================
// Initialize ESP-NOW
// =====================================================
void initESPNow() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    ShowSerial.println("ESP-NOW Init Failed");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    ShowSerial.println("Failed to add peer");
    return;
  }

  ShowSerial.println("ESP-NOW Ready");
}

void setup() {
  COMSerial.begin(9600, SERIAL_8N1, D7, D6);
  ShowSerial.begin(9600);

  delay(500);
  ShowSerial.println("Programme Starting!");

  xiao_config.enableEngineeringModel();

  initESPNow();
}

void loop() {
  int retryCount = 0;
  const int MAX_RETRIES = 10;

  do {
    radarStatus = xiao_config.getStatus();
    retryCount++;
  } while (radarStatus.targetStatus == Seeed_HSP24::TargetStatus::ErrorFrame && retryCount < MAX_RETRIES);

  bool validRadarFrame =
    radarStatus.targetStatus != Seeed_HSP24::TargetStatus::ErrorFrame;

  if (validRadarFrame && radarStatus.radarMode != 1) {
    xiao_config.enableEngineeringModel();
  }

  // Send even if this frame is ErrorFrame, so the bottom unit does not reuse the previous top presence data.
  radarPacket.targetStatus = (uint8_t)radarStatus.targetStatus;

  if (validRadarFrame) {
    // ShowSerial.print("Status: " + String(targetStatusToString(radarStatus.targetStatus)) + " ---- ");
    // ShowSerial.println("Distance: " + String(radarStatus.distance) + " Mode: " + String(radarStatus.radarMode));

    // if (radarStatus.radarMode == 1) {

    //   ShowSerial.print("Move:");
    //   for (int i = 0; i < 9; i++) {
    //     ShowSerial.print(" " + String(radarStatus.radarMovePower.moveGate[i]) + ",");
    //   }
    //   ShowSerial.println();

    //   ShowSerial.print("Static:");
    //   for (int i = 0; i < 9; i++) {
    //     ShowSerial.print(" " + String(radarStatus.radarStaticPower.staticGate[i]) + ",");
    //   }
    //   ShowSerial.println();

    //   ShowSerial.println("Photosensitive: " + String(radarStatus.photosensitive));
    // }

    // =====================================================
    // [Update 2] enum cannot be directly assigned to uint8_t
    // Must cast explicitly
    // =====================================================
    radarPacket.distance = radarStatus.distance;
    radarPacket.radarMode = radarStatus.radarMode;
    radarPacket.photosensitive = radarStatus.photosensitive;

    for (int i = 0; i < 9; i++) {
      radarPacket.moveGate[i] = radarStatus.radarMovePower.moveGate[i];
      radarPacket.staticGate[i] = radarStatus.radarStaticPower.staticGate[i];
    }
  } else {
    radarPacket.distance = 0;
    radarPacket.radarMode = 0;
    radarPacket.photosensitive = 0;

    for (int i = 0; i < 9; i++) {
      radarPacket.moveGate[i] = 0;
      radarPacket.staticGate[i] = 0;
    }
  }

  esp_now_send(peerAddress, (uint8_t *)&radarPacket, sizeof(radarPacket));

  delay(100);
}

// =====================================================
// Status parsing
// =====================================================
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
    default:
      return "Unknown";
  }
}

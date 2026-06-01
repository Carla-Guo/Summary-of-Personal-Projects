#include <WiFi.h>
#include <esp_now.h>
#include <HardwareSerial.h>
#include <mmwave_for_xiao.h>
#include <Adafruit_NeoPixel.h>

/*
   ESP32C6 双雷达楼梯灯系统

   目的保持不变:
   1. 本地下方雷达 + ESP-NOW 远端上方雷达判断通行方向
   2. 先触发一侧预亮 30 颗灯，等待另一侧确认
   3. 确认后按方向播放流水灯，完成后全亮保持
   4. 无人保持一段时间后熄灯
   5. 远端超时视为掉线，避免错误触发

   架构:
   - Radar FSM: 每 100ms 采样一次本地/远端雷达，并输出稳定 presence 快照
   - Passage FSM: 只根据 presence 快照判断方向与通行阶段，并投递 LED 命令
   - LED FSM: 只消费 LED 命令并执行对应显示模式

   读代码顺序建议:
   1. 先看 loop(): 它只是调度器，不处理具体业务。
   2. 再看 Radar FSM: 它负责把雷达原始值变成稳定的“有人/无人”。
   3. 再看 Passage FSM: 它负责判断上楼/下楼/等待/保持。
   4. 最后看 LED FSM: 它负责把 Passage FSM 投递的命令显示出来。

   模块之间的数据流:
   ESP-NOW 回调/本地雷达 -> Radar FSM -> radarSnapshot
   radarSnapshot -> Passage FSM -> ledCommand
   ledCommand -> LED FSM -> pixels 灯带

   注意:
   - Radar FSM 不直接控制 LED。
   - Passage FSM 不直接调用 LED 绘制函数，只投递 LED 命令。
   - LED FSM 不读取雷达，只根据命令和自己的动画状态工作。
   这样做的目的就是解耦，避免所有逻辑堆在 loop() 里。
*/


/* =========================================================
   UART 雷达
   ========================================================= */
// COMSerial 是连接本地毫米波雷达的串口。
// 这里沿用原代码 HardwareSerial(0)，如果你的硬件串口实际接线不同，再改这个对象。
HardwareSerial COMSerial(0);

// Seeed_HSP24 是 mmwave_for_xiao 库提供的雷达对象。
// 后面只在 Radar FSM 里调用 xiao_config.getStatus()。
Seeed_HSP24 xiao_config(COMSerial);
Seeed_HSP24::RadarStatus radarStatus;


/* =========================================================
   LED
   ========================================================= */
// 楼梯灯带输出引脚和灯珠总数，保持原代码目的: 150 颗灯。
#define LED_PIN      A0
#define NUMPIXELS    60

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_RGB + NEO_KHZ800);


/* =========================================================
   ESP-NOW 接收数据
   ========================================================= */
typedef struct struct_message {
  uint8_t targetStatus;
  uint16_t distance;
  uint8_t radarMode;
  uint8_t photosensitive;
  uint16_t moveGate[9];
  uint16_t staticGate[9];
} struct_message;

struct_message remoteRadar;

// 这两个变量在 ESP-NOW 接收回调里更新，在 Radar FSM 里读取。
// volatile 的意思是: 它们可能在主 loop() 之外被回调函数改动。
volatile bool remoteFrameSeen = false;
volatile unsigned long remoteLastSeen = 0;


/* =========================================================
   参数区
   ========================================================= */
// 上电后先等一小段时间，再启用雷达工程模式。
// 这个等待由 Radar FSM 用 millis() 判断，不使用阻塞式等待。
const unsigned long RADAR_BOOT_SETTLE_MS = 500;

// 三个任务的调度周期:
// - 雷达每 100ms 采样一次，符合你的任务要求。
// - 通行判断 10ms 扫一次，足够快，但不堆在 loop()。
// - LED FSM 1ms 扫一次，内部仍按 LED_INTERVAL_MS 推进动画。
const unsigned long RADAR_POLL_MS = 100;
const unsigned long PASSAGE_POLL_MS = 10;
const unsigned long LED_POLL_MS = 1;

// 雷达状态需要稳定 DEBOUNCE_MS 后才算有效，避免误触发。
const unsigned long DEBOUNCE_MS = 250;

// 一侧雷达先触发后，等待另一侧确认的最大时间。
const unsigned long WAIT_TIMEOUT_MS = 3000;

// 流水灯播完后，全亮保持的无人延时。
const unsigned long HOLD_ON_MS = 5000;

// 远端超过这个时间没有 ESP-NOW 包，就认为远端掉线/不可用。
const unsigned long REMOTE_TIMEOUT_MS = 2000;

// 流水灯速度: 每隔 15ms 新增点亮一颗。
const unsigned long LED_INTERVAL_MS = 15;

// 等待另一端确认时，先在触发侧预亮 30 颗。
const int PREVIEW_LED = 10;

// LED 全局亮度，范围 0-255。
// 数值越小越柔和。楼梯灯建议先从 60-100 之间试。
const uint8_t LED_BRIGHTNESS = 70;

// 暖白柔光 RGB。
// 如果想更暖: 降低 B；如果想更白: 提高 G/B。
const uint8_t LED_WARM_R = 255;
const uint8_t LED_WARM_G = 170;
const uint8_t LED_WARM_B = 75;

/* =========================================================
   通用类型
   ========================================================= */
enum Direction {
  DIR_NONE,  // 当前没有方向
  DIR_UP,    // 本地下方先触发，远端上方后触发，理解为上楼方向
  DIR_DOWN   // 远端上方先触发，本地下方后触发，理解为下楼方向
};

// 判断 now 距离 since 是否已经过了 intervalMs。
// unsigned long 这样写可以兼容 millis() 溢出回绕，是 Arduino 常用写法。
bool elapsed(unsigned long now, unsigned long since, unsigned long intervalMs) {
  return now - since >= intervalMs;
}

// loop() 里的调度小工具:
// 到时间就返回 true，并把 lastRun 更新为当前时间。
// 没到时间就直接返回 false，任务不会执行。
bool taskDue(unsigned long now, unsigned long &lastRun, unsigned long intervalMs) {
  if (!elapsed(now, lastRun, intervalMs)) return false;
  lastRun = now;
  return true;
}


/* =========================================================
   Radar FSM
   ========================================================= */
enum RadarFsmState {
  RADAR_WAIT_BOOT, // 上电后短暂等待，避免雷达刚启动时读到不稳定数据
  RADAR_ACTIVE     // 正常工作: 每 100ms 采样并更新稳定 presence
};

// 单个雷达输入的抗抖状态。
// raw: 本次读到的原始有人/无人。
// lastRaw: 上一次原始值，用来判断原始值是否刚发生变化。
// stable: 抗抖后对外公布的稳定值。
// changedAt: 原始值最后一次变化的时间点。
struct PresenceDebouncer {
  bool raw;
  bool lastRaw;
  bool stable;
  unsigned long changedAt;
};

// Radar FSM 对外输出的“稳定快照”。
// 其他模块只看这个快照，不直接读取雷达硬件。
struct RadarSnapshot {
  bool localPresence;   // 本地下方雷达稳定检测到有人
  bool remotePresence;  // 远端上方雷达稳定检测到有人
  bool remoteOnline;    // 远端 ESP-NOW 是否仍在超时时间内
};

// Arduino .ino 会自动生成函数声明，但它不总能正确处理自定义类型。
// 这里手写声明，避免它把 PresenceDebouncer 放到类型定义之前去声明。
void updateDebouncer(PresenceDebouncer &input, bool raw, unsigned long now);

RadarFsmState radarFsmState = RADAR_WAIT_BOOT;
PresenceDebouncer localRadarInput = {false, false, false, 0};
PresenceDebouncer remoteRadarInput = {false, false, false, 0};
RadarSnapshot radarSnapshot = {false, false, false};
unsigned long radarBootAt = 0;

Seeed_HSP24::TargetStatus convertStatus(uint8_t status) {
  return (Seeed_HSP24::TargetStatus)status;
}

// Seeed 雷达会返回移动目标、静止目标、两者都有、错误帧等状态。
// 对楼梯灯来说，只要有移动/静止/两者都有，就统一视为 presence=true。
bool statusIsPresence(Seeed_HSP24::TargetStatus status) {
  return (
    status == Seeed_HSP24::TargetStatus::MovingTarget ||
    status == Seeed_HSP24::TargetStatus::StaticTarget ||
    status == Seeed_HSP24::TargetStatus::BothTargets
  );
}

bool remoteStatusIsPresence(uint8_t status) {
  return statusIsPresence(convertStatus(status));
}

// 抗抖逻辑:
// 1. 原始值 raw 一变化，就重置 changedAt。
// 2. raw 连续保持 DEBOUNCE_MS 后，才把 stable 改成这个值。
// 这样雷达短暂抖动不会立刻影响系统状态机。
void updateDebouncer(PresenceDebouncer &input, bool raw, unsigned long now) {
  input.raw = raw;

  if (raw != input.lastRaw) {
    input.lastRaw = raw;
    input.changedAt = now;
  }

  if (elapsed(now, input.changedAt, DEBOUNCE_MS)) {
    input.stable = input.lastRaw;
  }
}

bool readLocalRadarRaw() {
  radarStatus = xiao_config.getStatus();

  if (radarStatus.targetStatus == Seeed_HSP24::TargetStatus::ErrorFrame) {
    return false;
  }

  return statusIsPresence(radarStatus.targetStatus);
}

// 远端雷达没有本地硬件读取动作，它的数据来自 ESP-NOW 回调。
// 如果超过 REMOTE_TIMEOUT_MS 没收到包，就强制认为远端无人且离线。
bool readRemoteRadarRaw(unsigned long now) {
  bool online = remoteFrameSeen && elapsed(now, remoteLastSeen, REMOTE_TIMEOUT_MS) == false;
  radarSnapshot.remoteOnline = online;

  if (!online) {
    return false;
  }

  return remoteStatusIsPresence(remoteRadar.targetStatus);
}

// Radar FSM 入口:
// 每次被 loop() 调度时只做很少的事，然后马上返回。
// 注意它不关心 LED，也不判断上楼/下楼，只负责产出 radarSnapshot。
void radarRunFsm(unsigned long now) {
  switch (radarFsmState) {
    case RADAR_WAIT_BOOT:
      // 非阻塞等待雷达上电稳定: 没到时间就 return，不占住 CPU。
      if (!elapsed(now, radarBootAt, RADAR_BOOT_SETTLE_MS)) return;

      xiao_config.enableEngineeringModel();
      localRadarInput.changedAt = now;
      remoteRadarInput.changedAt = now;
      radarFsmState = RADAR_ACTIVE;
      return;

    case RADAR_ACTIVE: {
      // 采样本地雷达和远端雷达，然后分别抗抖。
      bool localRaw = readLocalRadarRaw();
      bool remoteRaw = readRemoteRadarRaw(now);

      updateDebouncer(localRadarInput, localRaw, now);
      updateDebouncer(remoteRadarInput, remoteRaw, now);

      // 对外只发布 stable 结果，避免 Passage FSM 看到抖动的 raw 数据。
      radarSnapshot.localPresence = localRadarInput.stable;
      radarSnapshot.remotePresence = remoteRadarInput.stable;
      return;
    }
  }
}


/* =========================================================
   LED command queue + LED FSM
   ========================================================= */
// LED 命令类型。
// Passage FSM 通过 postLedCommand() 投递这些命令，LED FSM 再消费。
enum LedCommandType {
  LED_CMD_NONE,    // 没有命令
  LED_CMD_CLEAR,   // 清灯
  LED_CMD_PREVIEW, // 预亮触发侧 30 颗
  LED_CMD_FLOW,    // 按方向开始流水灯
  LED_CMD_FULL_ON  // 全亮保持
};

// LED 自己的状态。
// 这些状态只描述灯带正在做什么，不描述人从哪里来。
enum LedFsmState {
  LED_OFF,       // 灯带关闭
  LED_PREVIEW,   // 等待确认时，触发侧预亮
  LED_FLOW,      // 正在一颗一颗播放流水灯
  LED_FLOW_DONE, // 流水已经播完，等待 Passage FSM 接收完成事件
  LED_HOLD_ON    // 全亮保持
};

// 简单的“单槽命令队列”。
// 这个项目里 LED 命令不会高频堆积，所以一个 pending 命令足够。
struct LedCommand {
  bool pending;          // true 表示有一条命令等待 LED FSM 执行
  LedCommandType type;   // 命令类型
  Direction direction;   // 对 PREVIEW/FLOW 有意义
};

// LED FSM 的内部上下文。
// currentStep 表示流水灯已经推进到第几颗。
struct LedContext {
  LedFsmState state;       // 当前 LED 状态
  Direction direction;     // 当前动画方向
  int currentStep;         // 下一颗要点亮的序号
  unsigned long lastStepAt;// 上一次流水推进时间
};

// 手写这些声明，是为了避开 .ino 自动原型生成器对 enum/struct 参数的误判。
void postLedCommand(LedCommandType type, Direction direction);
void renderPreview(Direction direction);
void startLedFlow(Direction direction, unsigned long now);

LedCommand ledCommand = {false, LED_CMD_NONE, DIR_NONE};
LedContext ledContext = {LED_OFF, DIR_NONE, 0, 0};

// LED -> Passage 的事件标志。
// LED FSM 播完流水后把它置 true，Passage FSM 下一次调度时消费它。
bool ledFlowFinishedEvent = false;

// Passage FSM 调用这个函数投递 LED 命令。
// 它不直接调用 renderPreview()/renderFullOn()，保持模块解耦。
void postLedCommand(LedCommandType type, Direction direction) {
  ledCommand.pending = true;
  ledCommand.type = type;
  ledCommand.direction = direction;
}

// Passage FSM 用这个函数读取“流水完成事件”。
// 读取后会清掉事件，避免同一个事件被重复处理。
bool consumeLedFlowFinishedEvent() {
  if (!ledFlowFinishedEvent) return false;
  ledFlowFinishedEvent = false;
  return true;
}

uint32_t ledWarmWhite() {
  return pixels.Color(LED_WARM_R, LED_WARM_G, LED_WARM_B);
}

// 防止 PREVIEW_LED 配置大于灯带总数。
int previewLedCount() {
  if (PREVIEW_LED > NUMPIXELS) return NUMPIXELS;
  return PREVIEW_LED;
}

// 下面几个 renderXxx() 是实际“画灯”的函数。
// 它们只由 LED FSM 调用，不由雷达/通行状态机直接调用。
void renderClear() {
  pixels.clear();
  pixels.show();
}

void renderFullOn() {
  uint32_t color = ledWarmWhite();

  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, color);
  }

  pixels.show();
}

void renderPreview(Direction direction) {
  uint32_t color = ledWarmWhite();
  int count = previewLedCount();

  pixels.clear();

  for (int i = 0; i < count; i++) {
    // DIR_UP 从下方第 0 颗开始亮；DIR_DOWN 从上方最后一颗开始亮。
    int index = (direction == DIR_UP) ? i : NUMPIXELS - 1 - i;
    pixels.setPixelColor(index, color);
  }

  pixels.show();
}

void startLedFlow(Direction direction, unsigned long now) {
  // 开始流水前先重新画一次预亮区，确保灯带显示和方向一致。
  renderPreview(direction);

  ledContext.state = LED_FLOW;
  ledContext.direction = direction;

  // 因为前 PREVIEW_LED 颗已经亮了，所以流水从第 PREVIEW_LED 颗继续。
  ledContext.currentStep = previewLedCount();
  ledContext.lastStepAt = now;
  ledFlowFinishedEvent = false;
}

// LED FSM 每轮先检查有没有新命令。
// 有命令就立刻切换 LED 状态并完成对应的一次性显示动作。
void consumeLedCommand(unsigned long now) {
  if (!ledCommand.pending) return;

  LedCommand command = ledCommand;
  ledCommand.pending = false;
  ledCommand.type = LED_CMD_NONE;

  switch (command.type) {
    case LED_CMD_CLEAR:
      // 回到全灭状态。
      renderClear();
      ledContext.state = LED_OFF;
      ledContext.direction = DIR_NONE;
      ledContext.currentStep = 0;
      ledFlowFinishedEvent = false;
      return;

    case LED_CMD_PREVIEW:
      // 只预亮触发侧，不启动流水。
      renderPreview(command.direction);
      ledContext.state = LED_PREVIEW;
      ledContext.direction = command.direction;
      ledContext.currentStep = previewLedCount();
      ledFlowFinishedEvent = false;
      return;

    case LED_CMD_FLOW:
      // 进入流水动画状态，后续由 runLedFlow() 按时间片推进。
      startLedFlow(command.direction, now);
      return;

    case LED_CMD_FULL_ON:
      // 流水结束后全亮保持。
      renderFullOn();
      ledContext.state = LED_HOLD_ON;
      ledContext.direction = DIR_NONE;
      ledContext.currentStep = NUMPIXELS;
      ledFlowFinishedEvent = false;
      return;

    case LED_CMD_NONE:
      return;
  }
}

// 非阻塞流水:
// 每到 LED_INTERVAL_MS，只新增点亮一颗灯，然后立即返回。
// 没到时间就 return，不做任何等待。
void runLedFlow(unsigned long now) {
  if (!elapsed(now, ledContext.lastStepAt, LED_INTERVAL_MS)) return;

  ledContext.lastStepAt = now;

  if (ledContext.currentStep >= NUMPIXELS) {
    // 灯已经全部点亮，发出“流水完成”事件给 Passage FSM。
    ledContext.state = LED_FLOW_DONE;
    ledFlowFinishedEvent = true;
    return;
  }

  int index = (ledContext.direction == DIR_UP)
    ? ledContext.currentStep
    : NUMPIXELS - 1 - ledContext.currentStep;

  pixels.setPixelColor(index, ledWarmWhite());
  pixels.show();
  ledContext.currentStep++;

  if (ledContext.currentStep >= NUMPIXELS) {
    // 点亮最后一颗之后，也立即标记完成。
    ledContext.state = LED_FLOW_DONE;
    ledFlowFinishedEvent = true;
  }
}

// LED FSM 入口:
// 先消费命令，再根据当前状态决定是否推进动画。
void ledRunFsm(unsigned long now) {
  consumeLedCommand(now);

  switch (ledContext.state) {
    case LED_OFF:
    case LED_PREVIEW:
    case LED_FLOW_DONE:
    case LED_HOLD_ON:
      return;

    case LED_FLOW:
      runLedFlow(now);
      return;
  }
}


/* =========================================================
   Passage FSM
   ========================================================= */
// Passage FSM 是“通行逻辑状态机”。
// 它只判断当前是不是在等待另一侧、是不是已经确认方向、是不是该保持/熄灯。
enum PassageFsmState {
  PASSAGE_IDLE,        // 无人，等待任意一侧先触发
  PASSAGE_WAIT_REMOTE, // 本地下方先触发，等待远端上方确认
  PASSAGE_WAIT_LOCAL,  // 远端上方先触发，等待本地下方确认
  PASSAGE_FLOWING,     // 已确认方向，LED 正在流水
  PASSAGE_HOLDING      // 流水完成，灯带全亮保持
};

// Passage FSM 的上下文。
// direction 是本次通行方向，stateStartedAt/holdStartedAt 用来做超时判断。
struct PassageContext {
  PassageFsmState state;      // 当前通行状态
  Direction direction;        // 当前方向
  unsigned long stateStartedAt;// 当前状态开始时间
  unsigned long holdStartedAt; // 全亮保持计时起点
};

// Passage FSM 里有 Direction 参数，同样手写声明给 Arduino 预处理器看。
void enterFlow(Direction direction, unsigned long now);

PassageContext passageContext = {PASSAGE_IDLE, DIR_NONE, 0, 0};

// 进入空闲:
// 清掉方向，并投递清灯命令。
void enterIdle() {
  passageContext.state = PASSAGE_IDLE;
  passageContext.direction = DIR_NONE;
  postLedCommand(LED_CMD_CLEAR, DIR_NONE);
}

// 本地下方先触发:
// 方向暂定为上楼，LED 先从下方预亮 30 颗，等待远端确认。
void enterWaitRemote(unsigned long now) {
  passageContext.state = PASSAGE_WAIT_REMOTE;
  passageContext.direction = DIR_UP;
  passageContext.stateStartedAt = now;
  postLedCommand(LED_CMD_PREVIEW, DIR_UP);
}

// 远端上方先触发:
// 方向暂定为下楼，LED 先从上方预亮 30 颗，等待本地确认。
void enterWaitLocal(unsigned long now) {
  passageContext.state = PASSAGE_WAIT_LOCAL;
  passageContext.direction = DIR_DOWN;
  passageContext.stateStartedAt = now;
  postLedCommand(LED_CMD_PREVIEW, DIR_DOWN);
}

// 另一侧在超时时间内确认:
// 通行方向成立，投递流水灯命令。
void enterFlow(Direction direction, unsigned long now) {
  passageContext.state = PASSAGE_FLOWING;
  passageContext.direction = direction;
  passageContext.stateStartedAt = now;
  postLedCommand(LED_CMD_FLOW, direction);
}

// LED 流水播完:
// 进入全亮保持，由 Passage FSM 根据雷达 presence 决定何时熄灯。
void enterHolding(unsigned long now) {
  passageContext.state = PASSAGE_HOLDING;
  passageContext.holdStartedAt = now;
  postLedCommand(LED_CMD_FULL_ON, DIR_NONE);
}

// Passage FSM 入口:
// 它只读取 radarSnapshot 和 LED 完成事件，不直接读硬件、不直接画灯。
void passageRunFsm(unsigned long now) {
  // 拷贝一份快照，保证本次状态判断使用同一组雷达结果。
  RadarSnapshot snapshot = radarSnapshot;

  switch (passageContext.state) {
    case PASSAGE_IDLE:
      // 谁先稳定触发，就进入对应等待状态。
      // 如果两边同时为 true，这里优先本地侧，保持原逻辑的确定性。
      if (snapshot.localPresence) {
        enterWaitRemote(now);
      } else if (snapshot.remotePresence) {
        enterWaitLocal(now);
      }
      return;

    case PASSAGE_WAIT_REMOTE:
      // 本地先触发后，远端在 WAIT_TIMEOUT_MS 内触发，判定为上楼。
      if (snapshot.remotePresence) {
        enterFlow(DIR_UP, now);
      } else if (elapsed(now, passageContext.stateStartedAt, WAIT_TIMEOUT_MS)) {
        // 超时没有确认，取消本次预亮并回到空闲。
        enterIdle();
      }
      return;

    case PASSAGE_WAIT_LOCAL:
      // 远端先触发后，本地在 WAIT_TIMEOUT_MS 内触发，判定为下楼。
      if (snapshot.localPresence) {
        enterFlow(DIR_DOWN, now);
      } else if (elapsed(now, passageContext.stateStartedAt, WAIT_TIMEOUT_MS)) {
        enterIdle();
      }
      return;

    case PASSAGE_FLOWING:
      // Passage FSM 不知道 LED 具体播到哪一颗。
      // 它只等待 LED FSM 发出的“流水完成事件”。
      if (consumeLedFlowFinishedEvent()) {
        enterHolding(now);
      }
      return;

    case PASSAGE_HOLDING:
      // 只要任意一侧还有人，就刷新保持时间。
      if (snapshot.localPresence || snapshot.remotePresence) {
        passageContext.holdStartedAt = now;
      }

      // 两边都无人并持续 HOLD_ON_MS 后，才熄灯回空闲。
      if (elapsed(now, passageContext.holdStartedAt, HOLD_ON_MS)) {
        enterIdle();
      }
      return;
  }
}


/* =========================================================
   ESP-NOW
   ========================================================= */
void OnDataRecv(const esp_now_recv_info_t *info,
                const uint8_t *incomingData,
                int len) {
  // 回调函数要尽量短: 只拷贝数据、记录时间，不做业务判断。
  if (len != sizeof(remoteRadar)) return;

  memcpy(&remoteRadar, incomingData, sizeof(remoteRadar));
  remoteFrameSeen = true;
  remoteLastSeen = millis();
}

void initESPNow() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // 初始化失败时直接返回，Radar FSM 会因为 remoteFrameSeen=false 而认为远端不可用。
  if (esp_now_init() != ESP_OK) return;

  esp_now_register_recv_cb(OnDataRecv);
}


/* =========================================================
   setup
   ========================================================= */
void setup() {
  COMSerial.begin(9600);

  pixels.begin();
  pixels.setBrightness(LED_BRIGHTNESS);
  renderClear();

  // 记录启动时间，雷达工程模式稍后由 Radar FSM 非阻塞启用。
  radarBootAt = millis();
  initESPNow();
}


/* =========================================================
   loop: 只看时间片并分发任务
   ========================================================= */
unsigned long lastRadarTaskAt = 0;
unsigned long lastPassageTaskAt = 0;
unsigned long lastLedTaskAt = 0;

void loop() {
  unsigned long now = millis();

  // 任务1: 雷达每 100ms 读取一次。
  // 到点才进入 radarRunFsm()，没到点直接跳过。
  if (taskDue(now, lastRadarTaskAt, RADAR_POLL_MS)) {
    radarRunFsm(now);
  }

  // 通行状态机只消费 radarSnapshot，不直接读取雷达或控制灯带。
  if (taskDue(now, lastPassageTaskAt, PASSAGE_POLL_MS)) {
    passageRunFsm(now);
  }

  // LED 状态机只消费 ledCommand，并按自己的节奏推进动画。
  if (taskDue(now, lastLedTaskAt, LED_POLL_MS)) {
    ledRunFsm(now);
  }
}

# Smart Stair Light with Dual mmWave Radar

[English Version](README.md)

一个基于 **Seeed Studio XIAO ESP32-C6 + LD2410C mmWave 雷达 + ESP-NOW** 的双雷达楼梯灯原型。

这个项目的目标是做一个不依赖 Home Assistant、不依赖路由器、不依赖云端的独立楼梯灯系统。上下两端各放置一个 mmWave 雷达节点，系统通过 ESP-NOW 判断人员从哪一端进入楼梯，并按方向播放流水灯效果。

完整开发记录和项目背景可以查看 Hackster 文章：

https://www.hackster.io/carla-guo/building-a-smart-stair-light-with-esp32-c6-and-mmwave-radar-99085f

## 项目特点

- 双 mmWave 雷达检测上下楼方向
- ESP-NOW 点对点无线通信
- 无需 Wi-Fi 路由器
- 无需 Home Assistant
- 支持上楼/下楼方向判断
- 支持入口预亮、流水灯、全亮保持、延时熄灭
- 使用有限状态机拆分雷达、通行判断和 LED 动画逻辑


## 系统结构

本项目由两个 ESP32-C6 节点组成：

| 节点 | 位置 | 作用 | 使用代码 |
| --- | --- | --- | --- |
| Top node | 楼梯上方 | 读取上方雷达，并通过 ESP-NOW 发送雷达数据 | `xiao_mmwave_espnow_top.ino` |
| Bottom node | 楼梯下方 | 读取下方雷达、接收上方雷达数据，并控制 LED 灯带 | `xiao_mmwave_led_espnow_bottom_v3.ino` |

数据流大致如下：

```text
Top radar -> Top ESP32-C6 -> ESP-NOW -> Bottom ESP32-C6 -> LED strip
Bottom radar --------------------------^
```

Bottom 节点是主控节点。它同时处理本地下方雷达、远端上方雷达和 LED 灯带。

## 硬件准备

推荐硬件：

- 2 x Seeed Studio XIAO ESP32-C6
- 2 x LD2410C 或兼容的 mmWave 雷达模块
- 1 x WS2812/NeoPixel 兼容 LED 灯带
- 12V 电源
- 5V 降压供电模块或项目中的 LED Driver Board
- 若干连接线，或使用项目中的 PCB 原型

> 注意：LED 灯带不要直接从 ESP32-C6 取电。请使用独立电源或 LED Driver Board 给灯带供电，并确保 GND 共地。

## 软件准备

建议使用 Arduino IDE 或 PlatformIO。

需要安装的库：

- ESP32 Arduino Core，建议使用支持 ESP32-C6 的 3.x 版本
- `mmwave_for_xiao`
- `Adafruit NeoPixel`

Arduino IDE 中请选择对应的 ESP32-C6 开发板。如果你使用 Seeed XIAO ESP32-C6，请选择 XIAO ESP32-C6 或兼容板卡配置。

## 快速开始

第一次使用时，需要先读取 **bottom 节点 ESP32-C6 的 MAC 地址**，然后把这个 MAC 地址写入 top 节点代码。因为 top 节点需要知道 ESP-NOW 数据应该发给谁。

推荐烧录顺序：

1. 使用 `ESP_MAC.ino` 读取 bottom 节点 MAC 地址
2. 将读取到的 MAC 地址替换进 `xiao_mmwave_espnow_top.ino`
3. 将 top 代码烧录进 top 节点 ESP32-C6
4. 将 bottom 代码烧录进 bottom 节点 ESP32-C6

### 1. 读取 bottom 节点 MAC 地址

先把 **bottom 位置的 ESP32-C6** 连接到电脑。

打开并烧录：

```text
smart_stair_light_with_mmWave_radar/ESP_MAC.ino
```

打开串口监视器，波特率选择：

```text
115200
```

你会看到类似输出：

```text
STA MAC: E4:B3:23:B5:17:3C
```

记下这串 MAC 地址。这个地址属于 bottom 节点。

### 2. 修改 top 节点代码中的 MAC 地址

打开：

```text
smart_stair_light_with_mmWave_radar/xiao_mmwave_espnow_top.ino
```

找到这一行：

```cpp
uint8_t peerAddress[] = {0xE4, 0xB3, 0x23, 0xB5, 0x17, 0x3C};
```

把它替换为你刚才从 bottom 节点读到的 MAC 地址。

例如串口输出是：

```text
STA MAC: AA:BB:CC:11:22:33
```

那么代码中应该写成：

```cpp
uint8_t peerAddress[] = {0xAA, 0xBB, 0xCC, 0x11, 0x22, 0x33};
```

### 3. 烧录 top 节点

将 **top 位置的 ESP32-C6** 连接到电脑。

烧录：

```text
smart_stair_light_with_mmWave_radar/xiao_mmwave_espnow_top.ino
```

烧录完成后，top 节点会读取上方 mmWave 雷达，并持续通过 ESP-NOW 向 bottom 节点发送数据。

### 4. 烧录 bottom 节点

重新连接 **bottom 位置的 ESP32-C6**。

烧录：

```text
smart_stair_light_with_mmWave_radar/xiao_mmwave_led_espnow_bottom_v3.ino
```

烧录完成后，bottom 节点会：

- 读取下方 mmWave 雷达
- 接收 top 节点发来的雷达数据
- 判断上楼或下楼方向
- 控制 LED 灯带播放动画

## 接线说明

当前代码中主要引脚如下。

### Bottom 节点

LED 灯带数据引脚：

```cpp
#define LED_PIN A0
```

LED 数量：

```cpp
#define NUMPIXELS 60
```

如果你的灯带数量不同，请修改 `NUMPIXELS`。

### Top 节点

top 节点使用 UART 连接 mmWave 雷达：

```cpp
COMSerial.begin(9600, SERIAL_8N1, D7, D6);
```

如果你的雷达 TX/RX 接线不同，请根据实际硬件修改这里的引脚。

## 常用参数

在 `xiao_mmwave_led_espnow_bottom_v3.ino` 中可以调整这些参数：

| 参数 | 作用 |
| --- | --- |
| `NUMPIXELS` | LED 灯珠总数 |
| `LED_BRIGHTNESS` | LED 全局亮度 |
| `PREVIEW_LED` | 入口预亮灯珠数量 |
| `LED_INTERVAL_MS` | 流水灯动画速度 |
| `HOLD_ON_MS` | 无人后保持亮灯时间 |
| `WAIT_TIMEOUT_MS` | 一端触发后等待另一端确认的时间 |
| `REMOTE_TIMEOUT_MS` | top 节点掉线判定时间 |
| `DEBOUNCE_MS` | 雷达 presence 抗抖时间 |

## 工作逻辑

系统内部被拆成三个有限状态机：

| 模块 | 职责 |
| --- | --- |
| Radar FSM | 读取本地和远端雷达，并输出稳定 presence 状态 |
| Passage FSM | 根据上下两端触发顺序判断上楼/下楼 |
| LED FSM | 根据通行事件播放预亮、流水、保持和熄灭动画 |

这样做的好处是，雷达读取、方向判断和 LED 动画互不阻塞，系统会比把所有逻辑塞进 `loop()` 更稳定。

## 调试建议

如果系统没有按预期工作，可以按下面顺序检查：

1. 确认 bottom 节点 MAC 地址已经正确写入 top 代码
2. 确认 top 和 bottom 都使用 ESP32-C6，并且都烧录成功
3. 确认两个雷达模块供电正常
4. 确认雷达 TX/RX 没有接反
5. 确认 LED 灯带供电足够，且和 ESP32-C6 共地
6. 确认 `NUMPIXELS` 与实际灯珠数量一致
7. 打开串口监视器查看 ESP-NOW 是否发送成功

## 文件说明

| 文件 | 说明 |
| --- | --- |
| `ESP_MAC.ino` | 用于读取 ESP32-C6 的 STA MAC 地址 |
| `xiao_mmwave_espnow_top.ino` | top 节点代码，负责读取上方雷达并发送数据 |
| `xiao_mmwave_led_espnow_bottom_v3.ino` | bottom 节点代码，负责方向判断和 LED 控制 |
| `hardwave/` | PCB或硬件相关文件 |

## 项目状态

这是一个正在迭代中的原型项目。当前版本已经验证：

- 双雷达 presence 检测
- ESP-NOW 通信
- 上下楼方向判断
- LED 流水动画
- 雷达抗抖
- 无人延时熄灯

未来可以继续扩展：

- Web 配置页面
- OTA 固件更新
- 根据环境光自动调整亮度
- 根据雷达距离门实现更细的位置追踪
- 多段楼梯或多节点扩展

---
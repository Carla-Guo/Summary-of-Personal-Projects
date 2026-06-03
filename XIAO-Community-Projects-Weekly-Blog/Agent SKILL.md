---
name: xiao-weekly-search
description: XIAO 系列硬件热门项目多源搜索与去重（含黑名单机制）。执行 GitHub + YouTube + Instructables 多源搜索，并基于 projects.yaml + blacklist.md 去重后输出最终结果。
---

# xiao-weekly-search

从 GitHub、YouTube、Instructables 三个渠道搜索 XIAO 系列硬件项目，完成去重、信息补全、细筛，并输出可写入候选池。

<!-- Hackster.io 是纯前端渲染（SPA），项目列表由 JavaScript 动态加载，无法通过 curl/fetch 获取原始数据。

实际可用的替代方案：

web_fetch → 返回空/导航内容（已验证）
curl 直接请求 → HTML 无项目数据（已验证）
web_search → API 超时（已验证）
Hackster 的项目数据需要浏览器环境才能渲染，这不是 skill 执行问题，是平台技术限制。

 -->

---

# 🧠 全局执行原则（必须严格遵守）

## ⚠️ STRICT EXECUTION MODE

**This skill runs under ~.openclaw/workspace/EXECUTION_KERNEL.md rules.**

**Every task execution MUST run ~.openclaw/workspace/EXECUTION_SCORE_SYSTEM.md.**

---

## 📂 数据源

### 白名单（已收录项目）

```bash
~/Documents/OSHW-XIAO-Series/projects.yaml
```
<!-- 
### projects.yaml 格式
```yaml
 ========== 添加新项目模板 / Template for New Projects ==========

   - name:
       en: "Project Name"
       zh: "项目名称"
     description:
       en: "Brief description of the project"
       zh: "项目简介"
     board: "XIAO Model"
     category:
       en: "Category"
       zh: "分类"
     year: 
     author:
       en: "Author Name"
       zh: "作者名称"
     author_type: "Community" or "Official"
     link: "Project URL"
     image: "Image URL"
 ========== Board Options / 型号选项 ==========
 XIAO ESP32-S3, XIAO ESP32-S3 Sense, XIAO ESP32-C3, XIAO ESP32-C6, XIAO ESP32-C5
 XIAO nRF52840, XIAO nRF52840 Sense, XIAO nRF54L15 Sense
 XIAO RP2040, XIAO RP2350, XIAO SAMD21, XIAO RA4M1
 XIAO MG24, XIAO MG24 Sense
 ========== Category Options / 分类选项 ==========
 en: Wearables, Robotics, Smart Home, Healthcare, Power Management
     AI Gadget, Tools & Accessories, Telecommunication, Mechanical Keyboard, LED Lighting
 zh: 可穿戴, 机器人, 智能家居, 健康护理, 电源管理, AI Gadget, 工具配件, 通信, 机械键盘, LED 灯光
``` -->

### 黑名单（噪声项目库）

```bash
~/.openclaw/workspace/skills/xiao-weekly-search/blacklist.md
```

<!-- ### blacklist.md 格式

```md
- link: "Project URL"
  reason: "Why filtered"
``` -->

---

### review-notes.md（人工判断记录）

```bash
~/.openclaw/workspace/skills/xiao-weekly-search/review-notes.md
```

---

## PRE-FLIGHT CHECK (MANDATORY)

Before execution starts:

The assistant MUST output:

1. Execution plan (all phases)
2. Tools to be used (explicit list)
3. Data sources (files/scripts/APIs)
4. WAIT FOR USER CONFIRMATION ("CONFIRM")

❗ Execution cannot start without confirmation

# 🚀 Phase 1 — 多源一次性搜索（禁止回查）

## ⚠️ OUTPUT CONTRACT (STRICT):

Phase 1 is a RAW DATA EXTRACTION STAGE ONLY.

FORBIDDEN:
- No analysis of relevance
- No commentary on quality
- No summarization of results
- No interpretation of "XIAO relevance"
- No evaluation of dataset quality
- No suggestions for next steps

Any analytical sentence (including "results are good/bad/low relevance") is INVALID.


## 1a. GitHub 搜索

执行以下 3 个请求（一次性执行完）：

```bash
# 1. XIAO SAMD21 / RA4M1 / Seeeduino XIAO
curl -s "https://api.github.com/search/repositories?q=(\"XIAO+SAMD21\"+OR+\"XIAO+RA4M1\"+OR+\"Seeeduino+XIAO\"+OR+\"Seeed+Studio+XIAO\")+in:readme+pushed:>{7天前的日期}+stars:>30&sort=stars&order=desc&per_page=20"

# 2. XIAO nRF / BLE / MG24 / RP2040 / RP2350
curl -s "https://api.github.com/search/repositories?q=(\"XIAO+nRF52850\"+OR+\"XIAO+nRF54L15\"+OR+\"XIAO+BLE\"+OR+\"XIAO+MG24\"+OR+\"XIAO+RP2040\"+OR+\"XIAO+RP2350\")+in:readme+pushed:>{7天前的日期}+stars:>30&sort=stars&order=desc&per_page=20"

# 3. XIAO ESP32 Series
curl -s "https://api.github.com/search/repositories?q=(\"XIAO+ESP32\"+OR+\"XIAO+ESP32S3\"+OR+\"XIAO+ESP32C3\"+OR+\"XIAO+ESP32C6\"+OR+\"XIAO+ESP32C5\")+in:readme+pushed:>{7天前的日期}+stars:>30&sort=stars&order=desc&per_page=20"
```

将3个结果合并同类项输出Markdown preview：

```md
## GitHub Raw Pool (UNFILTERED)
1. ⭐stars | repo full name | description | url 
...
```

---

## 1b. YouTube 搜索

```bash
~/.openclaw/workspace/skills/xiao-weekly-search/scripts/search_youtube.py
python3 scripts/search_youtube.py
```

输出Markdown preview：

```md
## YouTube Raw Pool (UNFILTERED)
1. duration | title | description | url
...
```

---

## 1c. Instructables 搜索

```bash
~/.openclaw/workspace/skills/xiao-weekly-search/scripts/search_instructables.py
python3 scripts/search_instructables.py
```

输出Markdown preview：

```md
## Instructables Raw Pool (UNFILTERED)
1. title | description | url
...
```

---

# 🧊 Phase 2 — 粗筛（仅去重）

## ⚠️ INFORMATION ISOLATION RULE:

Phase 2 MUST NOT use any enrichment data.
Only URL-based comparison is allowed.
Any classification or content understanding is forbidden here.
此阶段开始禁止任何网络请求，禁止类型判断，仅做 whitelist / blacklist 去重

---

## 2a. 加载去重库（只读一次）

```text

# 解析 projects.yaml：遍历该文件，提取出所有条目下 `link:` 对应的值，存入 whitelist_set
whitelist_set ← projects.yaml (link)

# 解析 blacklist.md：遍历该文件，提取出所有条目下 `- link:` 对应的值，存入 blacklist_set
blacklist_set ← blacklist.md (link)

# 新建粗筛结果缓存空间
rough_candidate_pool = []
```

---

## 2b. 粗筛逻辑（单次执行）

对所有 raw pool 执行：

* ❌ link ∈ whitelist → 已收录
* ❌ link ∈ blacklist → 已过滤历史噪声
* ✅ 其余全部进入 rough_candidate_pool

---

## 2c. 输出粗筛结果

```md
## Rough Dedup Result

- GitHub: X
- YouTube: X
- Instructables: X
- Total: X
- 粗筛列表
```

---

# 🎨 Phase 3 — Unified Enrichment

Use only `rough_candidate_pool`.

<!-- ## Objective

For each candidate, perform one-pass metadata enrichment to gather all information required for:

1. Final screening (Phase 4)
2. projects.yaml writing (Phase 5)
3. Weekly blog generation
4. Future social content reuse

No re-fetch. No re-run.

If a field cannot be determined, return `null`. -->

---

## Unified Output Schema

```text
name
author
image

summary_blog

board
category
year

programming_language
secondary_languages
framework_tool
runtime_platform

key_tech
hardware_stack
use_case

source_evidence
confidence
```

---

## Field Definitions

### Core Fields

* `name` = project title
* `author` = creator / channel / repo owner
* `image` = best available image

### Content Fields

* `summary_blog` = 2-4 sentence description

### YAML Fields

* `board` = detected XIAO model(s)
* `category` = best-fit category
* `year` = publish year / latest update year

### Technical Fields

* `programming_language` = primary coding language
  (C++, Python, MicroPython, Rust, JavaScript...)

* `secondary_languages` = additional meaningful languages

* `framework_tool` = development tools / frameworks
  (Arduino IDE, PlatformIO, MicroPython, LVGL, TensorFlow Lite...)

* `runtime_platform` = system/runtime layer
  (ESP-IDF, Zephyr, Matter, Home Assistant...)

### Feature Fields

* `key_tech` = main technical highlights
  (BLE, Wi-Fi, LoRa, TinyML, MQTT, Voice Recognition...)

* `hardware_stack` = major hardware modules used
  (Camera, OLED, Relay, IMU, Servo, Battery...)

* `use_case` = real application scenario
  (Smart Home, Robotics, Wearable, Monitoring...)

### Validation Fields

* `source_evidence` = where evidence was found
* `confidence` = high / medium / low

---

## 3a. GitHub Enrichment

Extract from repo metadata + README using GitHub token in `~/.openclaw/workspace/.env`:

<!-- * repo title

* owner → author

* README summary

* first valid image

* board mentions

* latest update year

* programming language

* topics

* frameworks/tools mentioned

* runtime platform mentioned

* key tech from README/topics

* use case from README

* hardware modules mentioned -->

---

## 3b. YouTube Enrichment

```bash
curl -s "https://www.youtube.com/oembed?url={video_url}&format=json"

thumbnail → replace with `maxresdefault.jpg`

```

<!-- Extract from title/description/oembed:

* title

* author_name → author

* thumbnail → replace with `maxresdefault.jpg`

* summary_blog

* board mentions

* publish year (if available)

* programming language (only if explicitly stated)

* framework/tool mentions

* runtime platform mentions

* key tech

* hardware stack

* use case -->

---

## 3c. Instructables Enrichment

Extract from page.

<!-- * title

* author

* og:image

* summary

* board mentions

* publish year

* programming language (if stated)

* framework/tool mentions

* key tech

* hardware stack

* use case -->

---

## Output Rule

All candidates must return the same schema.

Missing fields = `null`

Do not guess unsupported claims.

将~/.openclaw/workspace/skills/xiao-weekly-search/enrichment_data.md原有内容覆盖写入新 enrichment 数据，此文件可不批准直接写入。



# 🔍 Phase 4 — 细筛（类型筛选 + 最终候选池）

---

## 4a. 加载人工判断库（只读一次）

遍历review-notes.md文件，学习之前判断失误的经验

## 4b. 类型筛选（仅基于~/.openclaw/workspace/skills/xiao-weekly-search/enrichment_data.md数据）

❌ 排除：
1. SDK/Driver：提供被别人调用的接口库，不是独立项目
2. 收集型/课程型：标题含数字规律或"collection"的合集，不是单一项目
3. 虚假引用：README仅在兼容列表/feature list中提到XIAO，无代码/硬件
4. 噪音：搜索命中但无真实 XIAO 使用

✅ 保留（其余全部）：
- 任何有具体XIAO使用示例的，不管项目多通用
- 任何以XIAO为主控的硬件产品/扩展板/PCB设计/集成产品
- 任何基于XIAO的软件应用（网络协议、无线电、嵌入式Linux风格应用）
- 不管项目是嵌入式/软件/游戏/任何形态
- 不管我认为它是否"配得上"叫XIAO项目
- 只要README有XIAO真实引用，就保留

---

## 4c. 输出去重报告

```md
## Dedup Report

### GitHub
- ❌ reason
- ✅ keep reason

### YouTube
- ❌ reason
- ✅ keep reason

### Instructables
- ❌ reason
- ✅ keep reason
```
---

## 4d. review-notes.md更新（更新人工判断记录）

Carla可以指出误判项目，修正 category / board / author，指出某类项目应保留或应淘汰，更新review-notes.md文件。

~/.openclaw/workspace/skills/xiao-weekly-search/review-notes.md

用途：

* Carla 指出误判项目
* Carla 修正 category / board / author
* Carla 指出某类项目应保留或应淘汰
* Carla 修改筛选标准

输出：

```md
## Review Notes Preview

### YYYY-MM-DD

- Project: xxx
  Link: xxx
  AI Decision: Reject
  Carla Correction: Keep
  Reason: Real XIAO project with valid hardware usage

- Project: xxx
  AI Category: Tools
  Carla Correction: Mechanical Keyboard
```

```text
append manual corrections / Carla decisions
```


---

## 4e. 黑名单新增缓存与最终候选池（锁定）

```md
GitHub: X
YouTube: X
Instructables: X
Total: X
```

```text
# 新增黑名单缓存
new_blacklist_buffer ← 本阶段被淘汰项目

# 新增FINAL CANDIDATE POOL (LOCKED)
final_candidate_pool ← 最终候选池项目
```

⚠️ 从此之后：

❌ 禁止重新筛选
❌ 禁止重新 enrichment
❌ 禁止回查数据

---


# 🧾 Phase 5 — 黑白名单文件写入（需 Carla 批准）

## ⚠️ 未获得 Carla 明确批准前，只输出预览，不得写文件

---
## 5a. blacklist.md（历史噪声库）

输出：

```md
## Blacklist Updates (NEW)
- link: "淘汰项目 URL"
  reason: "淘汰理由简述"
```

执行：

```text
blacklist.md += new_blacklist_buffer
```

---


---
## 5b. projects.yaml（最终候选池写入预览）

基于~/.openclaw/workspace/skills/xiao-weekly-search/enrichment_data.md数据，输出（必须使用 projects.yaml 标准格式）,可包含多个条目。：

```yaml
- name:
    en: ""
    zh: ""
  description:
    en: ""
    zh: ""
  board: ""
  category:
    en: ""
    zh: ""
  year:
  author:
    en: ""
    zh: ""
  author_type: "Community"
  link: ""
  image: ""

# ========== Board Options / 型号选项 ==========
# XIAO ESP32-S3, XIAO ESP32-S3 Sense, XIAO ESP32-C3, XIAO ESP32-C6, XIAO ESP32-C5
# XIAO nRF52840, XIAO nRF52840 Sense, XIAO nRF54L15 Sense
# XIAO RP2040, XIAO RP2350, XIAO SAMD21, XIAO RA4M1
# XIAO MG24, XIAO MG24 Sense


# ========== Category Options / 分类选项 ==========
# en: Wearables, Robotics, Smart Home, Healthcare, Power Management
#     AI Gadget, Tools & Accessories, Telecommunication, Mechanical Keyboard, LED Lighting
# zh: 可穿戴, 机器人, 智能家居, 健康护理, 电源管理, AI Gadget, 工具配件, 通信, 机械键盘, LED 灯光 -->

```

新增位置在开头`projects:` 字段之后, 加上开头：

```yaml
  # ========== YYYY-MM-DD Community Projects ==========
  # Source: xiao-weekly-search(日期)
```

执行：
```text
projects.yaml += final_candidate_pool

```

---

## 执行顺序（批准后）

1. 写入 blacklist.yaml
2. 写入 projects.yaml

---

获得 Carla 明确批准后方可执行写入。

# Phase6 —— blog 文章写作

## ⚠️ 未获得 Carla 明确批准前，只输出预览，不得写文件


基于 ~/.openclaw/workspace/skills/xiao-weekly-search/enrichment_data.md数据，参考~/.openclaw/workspace/skills/xiao-weekly-search/blog_reference.md风格，根据最终候选池名单撰写本周英文博客，内容包括：

**文章结构：**
1. **标题** — `# XIAO Community Projects Weekly`，附项目数量和日期
2. **简介** — 一段话概括本周亮点 + 分类标签
3. **项目列表** — 每个项目独立区块，包含：
   - **项目名 + 作者**（粗体）
   - **项目图片链接**（如果有的话）
   - **项目描述**（2~4 sentence project description with useful technical details, application scenario, or hardware setup.  
Bold keywords such as **XIAO Board**, **Arduino**, **PlatformIO**, **Python**, **BLE**, **Wi-Fi**, **LoRa**, **TinyML** if used.）
   - **Hardware Recommendation**：
      - 每个 Hardware Recommendation 的 XIAO 型号链接，必须来自 XIAO_selection.md 的 Board Selection Table
      - 每个 Expansion 产品链接，必须来自 XIAO_selection.md 的 ## Seeed Studio XIAO Expansion Board Products 区块
      - 每个 Grove 产品链接，必须来自 XIAO_selection.md 的 ## Grove Ecosystem Products 区块
      - 禁止凭记忆/印象填写，必须在写作时逐项对照文件确认
   - **项目链接**
4. **Board Selection Guide** — XIAO 全系列参数对比表格

**每条项目的 Markdown 格式：**
```md
## [项目名] by [作者]

[项目图片链接]（如果有的话）

[描述段落]

**Hardware Recommendation**
- **XIAO:** [名称](链接)
- **Expansion:** [名称](链接) 或 null
- **Grove Ecosystem:** [名称](链接) ，···可推荐多个 或 null

[View project](项目链接)
```

**风格特点：**
- 描述用一句话定位项目亮点，再展开功能
- Hardware 部分固定三项：XIAO + Expansion + Grove（无可填 null）
- 链接统一用 Markdown 短链接格式
- 无需额外评分或排名

---

获得 Carla 明确批准后方可执行覆盖~/.openclaw/workspace/skills/xiao-weekly-search/blog_reference.md原有内容并写入新博客内容。

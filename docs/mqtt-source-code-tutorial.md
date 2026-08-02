# MQTT Source Code Walkthrough — 從原始碼學 MQTT

> M5Stack CoreS3 Diagnostic System · `mqtt-pub` 指令的完整原始碼導讀
> 適合想理解「ESP32 上怎麼寫 MQTT 發布程式」的人，從命令入口一路讀到 MQTT 封包。

## 0. 這份文件怎麼用

這份教學不是使用手冊（請看 `mqtt-pub-guide.md`），而是**逐層拆解原始碼**：

| 章節 | 你會學到 |
|------|----------|
| §1–2 | MQTT 協定核心概念 + 專案裡哪個檔案做什麼 |
| §3 | 命令入口：如何把 CLI 參數變成一次發布流程 |
| §4 | MQTT 客戶端生命週期：連線 → 發布 → 確認 → 清理 |
| §5 | 手刻 JSON：不用 library 怎麼安全組 JSON |
| §6 | 事件驅動 vs 同步等待：ESP-IDF 的非同步 API 怎麼包成「等結果」 |
| §7 | NVS 設定層：`wifi-set mqtt` 的資料流 |
| §8 | 學習點總結（Interview 級重點） |
| §9 | 動手練習：本機 mosquitto + 6 道改寫習題 |

建議閱讀順序：§1 → §3 → §4 → §6 → §5 → §7，然後動手做 §9。

## 1. MQTT 是什麼（30 秒複習）

MQTT 是 IoT 最常用的**發布/訂閱（Pub/Sub）**協定，跑在 TCP 之上（預設 port 1883）。

```
Publisher ──CONNECT──▶ Broker        Subscriber A
           ◀─CONNACK── (中繼站)       Subscriber B
           ──PUBLISH topic=diagtest──▶  （按 topic 轉發）
           ◀─PUBACK────              （收到 → 轉送給 A、B）
```

三個關鍵機制：

1. **Topic** — 訊息的「頻道」，分層結構如 `m5s3_diag/a4cf12345678`。
   發布端往 topic 寫，訂閱端訂閱 topic 或萬用字元（`m5s3_diag/#`）。
2. **QoS（服務品質）** — 傳輸保證等級：
   - **QoS 0**：最多一次，發了不管，像 UDP。
   - **QoS 1**：至少一次，broker 收到要回 **PUBACK**，像 TCP 的 ACK。
   - **QoS 2**：恰好一次，來回 4 個封包，最貴。
3. **保留訊息（retain）** — 發布時設 retain=1，broker 會「記住最後一筆」，
   新訂閱者一上線就收到。本專案用 retain=0（即時性，不需補送）。

本專案的對應：

| MQTT 機制 | 專案用法 |
|-----------|----------|
| QoS | **1** — 需要 broker 真實確認（PUBACK）才算 PASSED |
| retain | 0 — 錯過就錯過，不需要歷史補送 |
| Clean Session | 開啟（`.session.disable_clean_session = false`）— 每次都是全新會話 |
| 安全 | TCP only（無 TLS）— flash budget 考量，見 `docs/diag_function_spec.md` |

## 2. 檔案地圖：MQTT 分散在四個檔案

```
common/src/M5Stack_CoreS3/
├── src/main.c                     §3  CLI 命令入口（Composition Root）
│      └── cmd_mqtt_pub()          ── 編排：拿設定 → 連 Wi-Fi → 發布 → 斷線
├── src/diag_net.c                 §4-6 網路服務層（Interface Adapter）
│      ├── diag_net_publish_mqtt() ── MQTT 客戶端生命週期
│      ├── json_build_report()     ── 手刻 JSON 報告
│      └── mqtt_evt()              ── MQTT 事件處理器
├── include/diag_net.h             ── 服務 API 宣告
└── src/hal/hal_wifi.c             §7  Wi-Fi HAL：連線 + NVS 設定
       └── cfg_get / cfg_set       ── NVS 持久化（diag_wifi namespace）
tools/mqtt_replay.py               ── 主機端重放工具（測試訂閱端用）
```

呼叫鏈（一次 `mqtt-pub` 的完整旅程）：

```
cmd_mqtt_pub (main.c)
  │  hal_wifi_cfg_get_mqtt_url()   ← NVS 讀 broker（無 → Kconfig fallback）
  │  hal_wifi_cfg_get_ssid/pass()  ← NVS 讀 Wi-Fi 憑證
  ├─▶ hal_wifi_connect()           ← 連 Wi-Fi（station mode, DHCP）
  ├─▶ hal_wifi_get_info()          ← 拿 IP/RSSI/channel 塞進報告
  └─▶ diag_net_publish_mqtt()      ← ★ MQTT 主角（§4）
        ├─ json_build_report()     ← 組 JSON payload
        ├─ esp_mqtt_client_init/start()
        ├─ esp_mqtt_client_publish(qos=1)
        └─ 等 PUBACK → PASSED
  └─▶ hal_wifi_deinit()            ← 拆 Wi-Fi（發布完就走，不佔資源）
```

## 3. 第一層：命令入口 `cmd_mqtt_pub`（main.c:818）

「命令」只做**編排**，不做 MQTT 細節。它回答三個問題：

**Q1: 要去哪個 broker？** — NVS 優先，Kconfig 兜底：

```c
// main.c:820-826
char broker[256], ssid[64], pass[64];
hal_wifi_cfg_get_mqtt_url(broker, sizeof(broker));
if (broker[0] == '\0') {
    diag_menu_printf("MQTT publish failed: no broker. Use: wifi-set mqtt "
                     "mqtt://host:1883\r\n");
    return DIAG_FAILED;
}
```

失敗要**看得見**：沒設定 broker 不是靜默失敗，而是印出「怎麼修」的提示。
這是整個專案的鐵律（CLAUDE.md：不允許靜默失敗）。

**Q2: topic 從哪來？**

```c
// main.c:836
const char *topic = (argc >= 2) ? argv[1] : NULL;   // NULL = 預設 m5s3_diag/<mac>
```

topic 預設值在服務層處理（§4 步驟 3），命令層保持「沒指定就傳 NULL」。

**Q3: 失敗了怎麼辦？** — 整個「連線 + 發布」循環重試一次：

```c
// main.c:843-866（重點節錄）
for (int attempt = 1; attempt <= 2; attempt++) {
    if (attempt > 1) {
        diag_menu_printf("MQTT: retry %d/2 in 3 s...\r\n", attempt);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
    if (hal_wifi_connect(ssid, pass, CONFIG_WIFI_CONNECT_TIMEOUT_MS) != DIAG_PASSED) {
        hal_wifi_deinit();
        continue;
    }
    r = diag_net_publish_mqtt(runner, g_diag_err_ctx, broker, topic, &info, &pub_ok);
    hal_wifi_deinit();
    if (r == DIAG_PASSED)
        break;
}
diag_menu_printf("MQTT publish: %s (%s)\r\n",
                 r == DIAG_PASSED ? "PASSED" : "FAILED",
                 pub_ok ? "acknowledged" : "no ack");
```

三個學習點：

- **每次連線用完立刻 `hal_wifi_deinit()`** — 這是「按需連線」設計：不發布就不佔
  無線資源。但也因此每次發布都要重新連 Wi-Fi（約 1–3 秒）。
- **結果用兩個訊號呈現**：`r`（整體成敗）+ `pub_ok`（broker 有沒有回 PUBACK）。
  FAILED 也分兩種：連不上 broker（no ack）vs 根本沒設定（no broker）。
- **重試一次**：`run-all` 剛跑完時 Wi-Fi driver 可能還在穩定中，第一次
  init+connect 偶發失敗，重試一次成本低、回報高。

> 💡 這個模式（拿設定 → 連 Wi-Fi → 做事 → 拆連線 → 印結果）和 `cmd_upload`、
> `cmd_ntp_sync` 完全一樣 — 讀懂一個，三個都會。

## 4. 第二層：MQTT 客戶端生命週期 `diag_net_publish_mqtt`（diag_net.c:420）

這是 MQTT 的核心。把它拆成 12 步：

```c
// diag_net.c:420-504（結構化後的流程）
diag_result_t diag_net_publish_mqtt(...)
{
    // 1. 參數檢查 — 壞輸入直接 FAILED，不 crash
    if (!broker_url || !broker_url[0] || !pub_ok)
        return DIAG_FAILED;
    *pub_ok = false;

    // 2. 組 JSON payload（§5 細講）
    char *json = malloc(JSON_BUF_SIZE);              // 16 KB
    size_t used = json_build_report(json, JSON_BUF_SIZE, runner, err_ctx, wifi);

    // 3. 決定 topic：NULL → "m5s3_diag/<mac>"（讀 MAC 避免撞 topic）
    char def_topic[64];
    if (!topic || !topic[0]) {
        uint8_t mac[6];
        esp_read_mac(mac, ESP_MAC_WIFI_STA);
        snprintf(def_topic, sizeof(def_topic), "m5s3_diag/%02x%02x%02x%02x%02x%02x",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        topic = def_topic;
    }

    // 4. 建立同步用的 semaphore（§6 細講）
    s_mqtt_evt = xSemaphoreCreateBinary();

    // 5. 初始化 MQTT 客戶端 — 只給 broker URI 一行設定
    esp_mqtt_client_config_t cfg = {
        .broker.address.uri = broker_url,            // "mqtt://192.168.1.10:1883"
        .session.disable_clean_session = false,      // 每次全新會話
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&cfg);

    // 6. 註冊事件處理器 — 之後所有 MQTT 事件都在 MQTT task 上呼叫 mqtt_evt
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_evt, NULL);

    // 7. 啟動客戶端 — 它自己開 task、自己連 TCP、自己送 CONNECT
    esp_mqtt_client_start(client);

    // 8. 等 MQTT_EVENT_CONNECTED（10 秒上限）→ 等 broker 回 CONNACK
    if (xSemaphoreTake(s_mqtt_evt, pdMS_TO_TICKS(MQTT_WAIT_MS)) == pdTRUE &&
        s_mqtt_connected) {

        // 9. 發布！QoS 1。msg_id > 0 = 已送進 outbox
        int msg_id = esp_mqtt_client_publish(client, topic, json, used, 1, 0);
        if (msg_id > 0) {
            // 10. 等 MQTT_EVENT_PUBLISHED = broker 的 PUBACK（§6）
            ... 輪詢 s_mqtt_published ...
        }
    }

    // 11. 清理：stop → destroy → 刪 semaphore → free
    esp_mqtt_client_stop(client);
    esp_mqtt_client_destroy(client);
    vSemaphoreDelete(s_mqtt_evt);
    free(json);
    return result;
}
```

### 對照 MQTT 封包 — 每一步在網路上發生什麼

| 程式步驟 | 網路上的封包 | 說明 |
|----------|--------------|------|
| `esp_mqtt_client_init` | — | 建立 handle，準備設定 |
| `esp_mqtt_client_start` | TCP 連線 → **CONNECT** | 開始連 broker:1883 |
| 步驟 8（等 CONNECTED） | ◀ **CONNACK** | broker 接受會話 |
| `esp_mqtt_client_publish` | **PUBLISH** (topic, QoS 1) | 報告上線 |
| 步驟 10（等 PUBLISHED） | ◀ **PUBACK** | broker 確認收到 → 才算 PASSED |

> 注意 `esp_mqtt_client_start()` 是**非同步**的：它啟動 MQTT task 就立刻回傳。
> CONNECT/CONNACK 是 MQTT task 自己慢慢跑的，程式用步驟 8 的 semaphore 等它。

### 為什麼要 QoS 1，不能 QoS 0？（diag_net.c:479 的註解是重點）

```c
/* QoS 1: broker PUBACK is required, msg_id > 0 guaranteed.
   (QoS 0 returns msg_id 0 on success — no ack, no success signal.) */
```

- QoS 0 發布「成功」時 `esp_mqtt_client_publish` 回傳 **msg_id = 0** — 因為沒有
  PUBACK，函式無法確認任何事。`0 > 0` 永遠為 false → 成功路徑是死碼，
  指令永遠顯示 FAILED。
- QoS 1 保證 broker 要回 **PUBACK**，msg_id > 0，且 `MQTT_EVENT_PUBLISHED`
  事件 = PUBACK 已收到 = **真實確認**。

這是整個功能最容易被忽略的坑：**選 QoS 不只是選「可靠性」，而是選「能不能
收到成功訊號」**。

## 5. 第三層：手刻 JSON `json_build_report`（diag_net.c:220）

專案刻意**不引入 JSON library**，用 `snprintf` + 字串拼接。理由是：
省 flash（partition 只有 1MB）、無相依、資料結構固定可控。

組出來的報告：

```json
{
  "app": "m5s3_diag",
  "idf": "v6.0.2-563-g89822381923",
  "mac": "a4:cf:12:34:56:78",
  "wifi": {"ssid": "MyWiFi", "ip": "192.168.1.42", "rssi": -44, "channel": 11},
  "rtc": "2026-08-01 00:26:29",
  "tests":  [{"id": "wifi", "result": "PASSED", "elapsed_ms": 3329, "message": "..."}],
  "errors": [{"component": "WIFI", "location": "MB/WIFI", "message": "...",
              "count": 1, "debug1": "...", "debug2": "..."}],
  "summary": {"total": 14, "passed": 12, "skipped": 1, "failed": 1}
}
```

三個防呆設計（讀程式時注意）：

1. **`json_escape()`（diag_net.c:205）** — 所有字串欄位（SSID、test name、
   message、debug1…）都先 escape。因為 SSID 或錯誤訊息可能含 `"` 或 `\n`，
   不 escape 會產生**壞 JSON**，甚至注入假欄位：

   ```c
   switch (in[i]) {
   case '"':  out[o++] = '\\'; out[o++] = '"';  break;   // " → \"
   case '\\': out[o++] = '\\'; out[o++] = '\\'; break;   // \ → \\
   case '\n': out[o++] = '\\'; out[o++] = 'n';  break;   // 換行 → \n
   }
   ```

2. **`JSON_HEADROOM` 預算守衛（diag_net.c:276）** — 16KB buffer 留下 1KB
   headroom，組到快滿就 `break`，最後 `json[used] = '\0'` 強制截斷。
   手刻 JSON 最怕 buffer overflow，這裡用「預算制」保證永不越界：

   ```c
   if (used > cap - JSON_HEADROOM)
       break;                       /* budget guard */
   ```

3. **`snprintf` 回傳值累進** — 每個欄位都用 `used += n` 追蹤游標位置。
   組完檢查 `used >= cap` 時防禦性截斷（diag_net.c:337-339）。

> 📌 手刻 JSON 的黃金規則：**string 一律 escape、游標永遠從 `snprintf`
> 回傳值累進、結束前檢查是否爆 buffer**。三件事缺一不可。

## 6. 第四層：事件驅動 → 同步等待（diag_net.c:391-418）

這是全檔最有教學價值的地方。**ESP-IDF 的 MQTT API 是事件驅動的**：
所有狀態變化（連上、發布完成、斷線）都透過回呼告訴你，回呼跑在 MQTT task。
但 CLI 指令是**同步的**（一步步往下跑），所以要自己架一座橋。

### 橋的零件

```c
// diag_net.c:391-395 — 一個 semaphore + 三個 volatile 旗標
static SemaphoreHandle_t s_mqtt_evt;   /* signalled by event handler */
/* Written by the MQTT task, polled by the caller task: volatile. */
static volatile bool     s_mqtt_connected;
static volatile bool     s_mqtt_published;
static volatile bool     s_mqtt_failed;
```

### 事件處理器（MQTT task 上執行）

```c
// diag_net.c:397-418
static void mqtt_evt(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    switch ((esp_mqtt_event_id_t)id) {
    case MQTT_EVENT_CONNECTED:      // broker 回 CONNACK
        s_mqtt_connected = true;
        s_mqtt_failed    = false;
        xSemaphoreGive(s_mqtt_evt); // 喚醒等待中的呼叫者
        break;
    case MQTT_EVENT_PUBLISHED:      // broker 回 PUBACK（QoS 1）
        s_mqtt_published = true;
        xSemaphoreGive(s_mqtt_evt);
        break;
    case MQTT_EVENT_DISCONNECTED:
    case MQTT_EVENT_ERROR:
        s_mqtt_failed = true;
        xSemaphoreGive(s_mqtt_evt);
        break;
    }
}
```

### 兩種等待方式，各取所長

| 階段 | 方式 | 原因 |
|------|------|------|
| 等 CONNECTED（步驟 8） | `xSemaphoreTake` **阻塞** | 只等一件事，semaphore 語意乾淨：事件沒來就睡，事件來就醒 |
| 等 PUBLISHED（步驟 10） | 100ms **輪詢**旗標 | PUBLISHED 前可能穿插其他事件（ERROR/DISCONNECTED），純等 semaphore 容易漏判；輪詢 + 檢查三個旗標比較穩健 |

```c
// diag_net.c:484-495 — PUBLISHED 的輪詢迴圈
TickType_t waited = 0;
while (waited < MQTT_WAIT_MS) {
    if (s_mqtt_published) { *pub_ok = true; result = DIAG_PASSED; break; }
    if (s_mqtt_failed)    break;          // 斷線/錯誤要能提早跳出
    vTaskDelay(pdMS_TO_TICKS(100));
    waited += 100;
}
```

### 為什麼旗標要 `volatile`？（面試常考）

`volatile` 告訴編譯器「這個變數會被別人改，不要優化成暫存器複本」。
`mqtt_evt` 跑在 **MQTT task**，主 task 輪詢的是**自己記憶體裡複製的變數** —
沒有 `volatile`，編譯器可能把 `s_mqtt_published` 讀一次塞進暫存器，迴圈永遠
看到舊值。正確做法其實是 atomic 或 mutex，但**二值旗標 + 單寫單讀**的場景
用 volatile 是嵌入式常見的務實取捨（程式註解也說明了這個 trade-off）。

## 7. 第五層：NVS 設定 `wifi-set mqtt`（hal_wifi.c:48-130）

MQTT broker 網址怎麼存？答案是 **NVS（Non-Volatile Storage）** — flash 上的
小型 key-value store，重開機不消失。

資料流：

```
mqtt-pub 指令 (main.c:700)
  └─▶ wifi-set mqtt mqtt://broker.emqx.io:1883
        └─▶ hal_wifi_cfg_set_mqtt_url()  (hal_wifi.c:125)
              └─▶ cfg_set("mqtt", value)  (hal_wifi.c:65)
                    ├─ nvs_open("diag_wifi", NVS_READWRITE)
                    ├─ nvs_set_str → nvs_commit    ← 寫 flash
                    └─ nvs_close

讀取端 (diag_net 的上游 main.c:821)
  └─▶ hal_wifi_cfg_get_mqtt_url()
        └─▶ cfg_get("mqtt", buf, len, CONFIG_WIFI_DIAG_MQTT_URL)
              └─ nvs_get_str → 找到就用 NVS 值
                            → 找不到回退 Kconfig 編譯期預設
```

`cfg_get` 的精髓在 **fallback 參數**（hal_wifi.c:48-63）：

```c
static diag_result_t cfg_get(const char *key, char *buf, size_t len,
                             const char *fallback)
{
    nvs_handle_t h;
    if (nvs_open(CFG_NS, NVS_READONLY, &h) == ESP_OK) {
        size_t need = len;
        esp_err_t e = nvs_get_str(h, key, buf, &need);
        nvs_close(h);
        if (e == ESP_OK && buf[0] != '\0')
            return DIAG_PASSED;          // NVS 有值 → 用 NVS
    }
    snprintf(buf, len, "%s", fallback ? fallback : "");   // 沒有 → Kconfig
    return DIAG_PASSED;
}
```

**NVS 優先、Kconfig 兜底** 的好處：出廠韌體編譯時設預設值，現場不用重刷
韌體，`wifi-set mqtt mqtt://我的broker:1883` 一行就改。同理也用在 ssid、
pass、ntp、url 五個 key（hal_wifi.c:25-30）。

## 8. 學習點總結（這份程式教你的六件事）

1. **分層**：命令層（編排）/ 服務層（協定）/ HAL 層（硬體）。MQTT 細節全在
   `diag_net.c`，換 broker、改 QoS 都不動命令層。
2. **非同步 API 包成同步**：semaphore 等「單一事件」，輪詢旗標等「多事件
   混合」；跨 task 共享變數要 `volatile`（嚴格做法是 atomic）。
3. **QoS 決定「你收不收得到成功訊號」**：QoS 0 沒有 PUBACK，`msg_id = 0`
   無法判成功 — 這是選 QoS 1 的真正理由。
4. **手刻 JSON 三鐵則**：escape 字串、snprintf 累進游標、buffer 預算守衛。
5. **錯誤必須看得見**：每個失敗路徑都印「原因 + 解法」（`no broker. Use:
   wifi-set mqtt ...`），絕不靜默失敗。
6. **資源生命週期**：malloc 一定配對 free、client 一定 stop+destroy、
   semaphore 一定刪除 — 所有 return 路徑都要走到清理。

## 9. 動手練習

### 9.1 本機架一個 broker（10 分鐘）

不用硬體也能玩 — 本機跑 mosquitto，重放工具當發布端：

```bash
# Ubuntu / Debian
sudo apt install mosquitto mosquitto-clients

# 終端機 A：訂閱
mosquitto_sub -h localhost -t 'diagtest' -v

# 終端機 B：發布（用專案的 Python 重放工具）
python3 tools/mqtt_replay.py --broker mqtt://localhost:1883 --topic diagtest
```

終端機 A 應該出現完整的 JSON 報告。這工具（`tools/mqtt_replay.py`）發布的
payload 與設備 `mqtt-pub` 完全同格式 — 訂閱端開發、儀表板 demo 都不用碰硬體。

### 9.2 真機測試 SOP（與 `docs/mqtt-pub-guide.md` §3 相同流程）

```
1. wifi-set ssid "你的SSID" / wifi-set pass "你的密碼"
2. wifi-set mqtt mqtt://<你的broker>:1883
3. 主機端先訂閱：python3 tools/mqtt_replay.py --listen --topic diagtest
4. 設備端：mqtt-pub diagtest
5. 看到 PASSED (acknowledged) + 主機端收到 JSON = 成功
```

### 9.3 六道改寫習題（改完你就真的會了）

每題都在原始碼上改，改完照 9.1/9.2 驗證：

| # | 習題 | 改哪裡 | 驗證點 |
|---|------|--------|--------|
| 1 | 改成 QoS 0 發布 | diag_net.c:481 `publish(..., 1, 0)` → `(..., 0, 0)` | 指令顯示 FAILED (no ack) — 親眼看到 §4 講的坑 |
| 2 | 加上 retain=1 | diag_net.c:481 最後參數 0→1 | 訂閱端晚到也能收到最後一筆 |
| 3 | 發布成功後訂閱 `m5s3_diag/#` 收自己的訊息 | 在 diag_net.c 註冊 `MQTT_EVENT_DATA` 回呼 | 收到自己發的 JSON — 理解事件 ID 對應 |
| 4 | 改用 QoS 2 | 同上 + 理解 `MQTT_EVENT_PUBLISHED` 語意變化 | 仍 PASSED，多花 2 個封包 |
| 5 | 加 broker 帳號密碼 | cfg 加 `.credentials.username/.password` | 連不上會 DISCONNECTED → FAILED |
| 6 | 把 MQTT_WAIT_MS 從 10000 改 1000 | diag_net.c:36 | 慢 broker 下 FAILED — 理解 timeout 的意義 |

> 注意：習題 1 改完會「壞掉」是**預期結果** — 這正是教學目的。
> 改完記得改回來。

### 9.4 觀察封包（進階）

想親眼看到 CONNECT/CONNACK/PUBLISH/PUBACK 四個封包，抓包：

```bash
# broker 那台機器
sudo tcpdump -i any -n port 1883
```

或安裝 Wireshark，filter 用 `tcp.port == 1883`。

## 10. 延伸閱讀

| 資源 | 內容 |
|------|------|
| `docs/mqtt-pub-guide.md` | 功能使用手冊：設定、測試 SOP、排錯、Q&A |
| `docs/diag_function_spec.md` §MQTT Publish Utility | 功能規格（DFS 原始需求） |
| ESP-IDF `esp-mqtt` 官方文件 | `esp_mqtt_client_publish` / 事件 API 完整參考 |
| MQTT 規格書 v3.1.1 | 協定細節：QoS 狀態機、retain、clean session |
| `tools/mqtt_replay.py` | 主機端重放工具原始碼（paho-mqtt 使用範例） |

---

原始碼位置：`common/src/M5Stack_CoreS3/src/main.c`（cmd_mqtt_pub）·
`common/src/M5Stack_CoreS3/src/diag_net.c`（diag_net_publish_mqtt / json_build_report /
mqtt_evt）· `common/src/M5Stack_CoreS3/src/hal/hal_wifi.c`（cfg_get / cfg_set）·
`tools/mqtt_replay.py`

# M5Stack CoreS3 診斷系統

以 ESP-IDF 框架、乾淨架構（Clean Architecture）、UART Console 操作的硬體診斷系統。

## 架構

```
.
├── common/
│   ├── chips/                          # 可重用晶片驅動（無電路板相依）
│   │   ├── screen_GC9A01/
│   │   │   ├── screen_GC9A01.h         # GC9A01 暫存器定義 + 晶片 API
│   │   │   └── screen_GC9A01.c         # SPI 驅動實作
│   │   ├── touch_FT6336/
│   │   │   ├── touch_FT6336.h          # FT6336 暫存器定義 + 晶片 API
│   │   │   └── touch_FT6336.c          # I2C 驅動實作
│   │   ├── rtc_BM8563/
│   │   │   ├── rtc_BM8563.h            # BM8563 暫存器定義 + 晶片 API
│   │   │   └── rtc_BM8563.c            # I2C 驅動實作
│   │   ├── imu_BMI270/
│   │   │   ├── imu_BMI270.h            # BMI270 暫存器定義 + 晶片 API
│   │   │   └── imu_BMI270.c            # I2C 驅動實作
│   │   └── power_AXP2101/
│   │       ├── power_AXP2101.h         # AXP2101 暫存器定義 + 晶片 API
│   │       └── power_AXP2101.c         # I2C 驅動實作
│   └── src/M5Stack_CoreS3/             # CoreS3 診斷專案
│       ├── include/                    # Domain + Interface Adapter 層（純介面）
│       │   ├── diag_core.h             # 核心型別、列舉、測試實體
│       │   ├── diag_config.h           # 系統配置、引腳定義
│       │   ├── diag_menu.h             # UART Console 選單介面
│       │   ├── diag_runner.h           # 測試執行器介面
│       │   └── hal/                    # HAL 抽象介面（電路板級合約）
│       │       ├── hal_screen.h
│       │       ├── hal_touch.h
│       │       ├── hal_rtc.h
│       │       ├── hal_imu.h
│       │       └── hal_power.h
│       ├── src/
│       │   ├── main.c                  # app_main 進入點
│       │   ├── diag_menu.c             # UART 選單實作
│       │   ├── diag_runner.c           # 測試排程實作
│       │   └── hal/                    # CoreS3 電路板配接器
│       │       ├── hal_i2c_helpers.[ch]
│       │       ├── hal_screen.c        # SPI init → 委派 screen_GC9A01
│       │       ├── hal_touch.c         # I2C init → 委派 touch_FT6336
│       │       ├── hal_rtc.c           # I2C init → 委派 rtc_BM8563
│       │       ├── hal_imu.c           # I2C init → 委派 imu_BMI270
│       │       └── hal_power.c         # I2C init → 委派 power_AXP2101
│       └── CMakeLists.txt
└── CMakeLists.txt                      # 頂層 ESP-IDF 專案
```

### Clean Architecture 分層

| 層級 | 目錄 | 內容 |
|------|------|------|
| Domain | `include/diag_core.h` | 測試實體、結果列舉、型別 |
| Interface Adapter | `include/diag_menu.h`, `include/diag_runner.h` | Console presenter/controller、測試編排 |
| Frameworks & Drivers | `include/hal/*.h`, `src/hal/*.c`, `common/chips/*/` | HAL 抽象 + CoreS3 配接器 + 通用晶片驅動 |

## 硬體支援

| 週邊 | 晶片 | 介面 | HAL 檔案 |
|------|------|------|----------|
| 螢幕 | GC9A01 (240×240 圓形 LCD) | SPI | `hal_screen.[ch]` |
| 觸控 | FT6336 電容式觸控 | I2C (0x38) | `hal_touch.[ch]` |
| RTC | BM8563 即時時鐘 | I2C (0x51) | `hal_rtc.[ch]` |
| IMU | BMI270 六軸加速度計/陀螺儀 | I2C (0x69) | `hal_imu.[ch]` |
| 電源 | AXP2101 電源管理單元 | I2C (0x34) | `hal_power.[ch]` |

## 建置

```bash
# 設定 ESP-IDF 環境（請確保已安裝 v5.x）
source ~/esp/esp-idf/export.sh

# 切換到專案根目錄
cd m5stack_diag

# 設定目標晶片
idf.py set-target esp32s3

# 編譯
idf.py build

# 燒錄（請先連接 CoreS3 的 USB 埠）
idf.py -p /dev/ttyACM0 flash

# 監控序列埠
idf.py -p /dev/ttyACM0 monitor
```

## UART 選單指令

| 指令 | 說明 |
|------|------|
| `help` | 顯示說明 |
| `info` | 列出所有測試與狀態 |
| `run <name\|#>` | 執行單一測試 |
| `run-all` | 依序執行全部測試 |
| `status` | 顯示系統狀態總覽 |
| `screen-on` | 開啟螢幕 |
| `screen-off` | 關閉螢幕 |
| `reboot` | 軟體重置 |
| `shutdown` | 系統關機 |
| `reset` | 清除測試結果 |
| `exit` / `quit` | 離開選單 |

## 測試清單

| 編號 | 名稱 | 說明 |
|------|------|------|
| 0 | `i2c-scan` | 掃描 I2C 匯流排，檢查各週邊是否回應 |
| 1 | `screen` | 顯示彩色條、文字、十字線 |
| 2 | `touch` | 讀取 FT6336 觸控狀態與韌體版本 |
| 3 | `rtc` | 讀取 BM8563 即時時鐘時間 |
| 4 | `imu` | 讀取 BMI270 加速度計與陀螺儀數值 |
| 5 | `power` | 讀取 AXP2101 電池電壓、充電狀態、溫度 |

## License

MIT

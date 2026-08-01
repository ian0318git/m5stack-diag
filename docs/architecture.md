參考的code base:example/fugazi_ng_diag/common/src/fugazi/mb_tests.c
common/src/M5Stack_CoreS3/
├── include/
│   ├── diag_config.h       # 系統全域配置與引腳定義
│   ├── diag_menu.h         # Console 選單核心邏輯介面
│   ├── diag_runner.h       # 測試排程與管理核心介面
│   └── hal/                # 硬體抽象層 (Hardware Abstraction Layer) 介面
│       ├── hal_screen.h
│       ├── hal_touch.h
│       ├── hal_rtc.h
│       ├── hal_imu.h
│       └── hal_power.h
├── src/
│   ├── main.c              # 診斷程式進入點 (ESP-IDF app_main)
│   ├── diag_menu.c         # UART 互動選單實作 (參考原 menu.c)
│   ├── diag_runner.c       # 測試流程控制與狀態回報 (參考原 mb_tests.c)
│   └── hal/                # 針對 CoreS3 硬體的專屬驅動實作
│       ├── hal_screen.c    # 螢幕控制 (GC9A01/SPI)
│       ├── hal_touch.c     # 觸控晶片 (FT6336/I2C)
│       ├── hal_rtc.c       # 時鐘晶片 (BM8563/I2C)
│       ├── hal_imu.c       # 六軸感測器 (BMI270/I2C)
│       └── hal_power.c     # 電源管理 (AXP2101/I2C)
└── CMakeLists.txt          # ESP-IDF 組件編譯設定
common/chips/
├── screen_GC9A01/          # 螢幕控制模組
│   ├── screen_GC9A01.h     # 導出 hal_screen_* 通用介面與 GC9A01 暫存器定義
│   └── screen_GC9A01.c     # GC9A01 SPI 驅動與刷色具體實作
├── touch_FT6336/           # 觸控晶片模組
│   ├── touch_FT6336.h      # 導出 hal_touch_* 通用介面與 FT6336 暫存器定義
│   └── touch_FT6336.c      # FT6336 I2C 驅動與座標讀取具體實作
├── rtc_BM8563/             # 時鐘晶片模組
│   ├── rtc_BM8563.h        # 導出 hal_rtc_* 通用介面與 BM8563 暫存器定義
│   └── rtc_BM8563.c        # BM8563 I2C 驅動與時間讀寫具體實作
├── imu_BMI270/             # 六軸感測器模組
│   ├── imu_BMI270.h        # 導出 hal_imu_* 通用介面與 BMI270 暫存器定義
│   └── imu_BMI270.c        # BMI270 I2C 驅動與六軸 raw/物理量讀取實作
└── power_AXP2101/          # 電源管理晶片模組
    ├── power_AXP2101.h     # 導出 hal_power_* 通用介面與 AXP2101 暫存器定義
    └── power_AXP2101.c     # AXP2101 I2C 驅動、電量讀取與電軌控制實作
參考資料:https://docs.m5stack.com/zh_CN/core/CoreS3
esp-bsp:https://github.com/espressif/esp-bsp/tree/master/bsp/m5stack_core_s3

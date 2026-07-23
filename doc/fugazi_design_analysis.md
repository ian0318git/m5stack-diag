# Fugazi NG Diag — 設計模式、HAL 封裝與模組解耦分析

**分析日期:** 2026-07-23  
**來源:** `example/fugazi_ng_diag/`  
**原始框架:** Cisco Systems, Inc. — NG Diagnostic Framework

---

## 概述

Fugazi NG Diag 是一套 Cisco 產線級硬體診斷框架，運行於 Linux userspace，透過 `/dev/mem`、`/dev/i2c-N`、UIO 等介面存取數十種晶片（PHY、FPGA、PMIC、I2C switch、溫度感測器、USB controller 等）。原始架構始於 2002 年，歷經多次平台移植仍然保持穩定。

---

## 1. 核心設計模式

### 1.1 物件模型：OOP in C via Function Vector Table

最核心的設計是 **以 C struct 模擬類別繼承與多型**。基底「類別」是 `dev_object_t`，核心介面定義在 `dev_object_fvt_t`（FVT = Function Vector Table）：

```c
// 基底「類別」
struct dev_object_t_ {
    dev_object_fvt_t *dev_object_fvt;  // vtable pointer
    void    *dev_addr;                 // virtual base address
    uint32  hw_ver_id;
    uint32  dev_flag;
    uint32  dev_state;
    void    *client_context;
    char    *dev_location;
};

// vtable — 所有晶片共用的生命週期介面
typedef struct dev_object_fvt_t_ {
    uint32 (*dev_attach)(dev_object_t *);
    uint32 (*dev_detach)(dev_object_t *);
    uint32 (*dev_reconfig_needed)(dev_object_t *, void *, boolean *);
    uint32 (*dev_restart)(dev_object_t *);
    uint32 (*dev_init)(dev_object_t *);
    uint32 (*dev_oper_enable)(dev_object_t *);
    uint32 (*dev_oper_disable)(dev_object_t *);
    uint32 (*dev_intr_enable)(dev_object_t *);
    uint32 (*dev_intr_disable)(dev_object_t *);
    uint32 (*dev_isr)(dev_object_t *);
    uint32 (*dev_show)(dev_object_t *, print_fn_t, dev_show_cmd);
    dev_error_report_t dev_error_report;
    uint32 (*dev_collect_crashinfo)(dev_object_t *, print_fn_t, dev_show_cmd);
    void (*dev_destroy)(dev_object_t **);
    const char *dev_name;
} dev_object_fvt_t;
```

**深度評估**：這個介面是 **deep** 的典範 — 14 個函式指標涵蓋了任何硬體裝置的完整生命週期（建立→附著→初始化→啟用操作→中斷服務→錯誤回報→crashinfo 收集→銷毀），呼叫者只需學習這一組介面就能操作所有 40+ 種晶片。`dev_name` 放在 vtable 最後一欄，是刻意為將來的擴展保留空間。

### 1.2 繼承模式：Struct Embedding

每種晶片透過 **C struct 嵌入** 來「繼承」基底：

```c
typedef struct dev_pca_object_t {
    dev_object_t    base;              // ← 基底必須是第一欄，保證安全轉型
    pca_callin_fvt_t   *callin_fvt;   // 裝置提供的功能
    pca_callout_fvt_t  *callout_fvt;  // 裝置需要的平台服務
    n2g_i2c_if_t   *i2c_p;
    dev_pca_desc_t *desc_p;
    uchar          *dev_name;
    pca_t           init;
} dev_pca_object_t;
```

使用上就是把 `dev_object_t *` 直接 cast 為具體子型別 — 因為 `base` 在第一欄，兩者的記憶體佈局相同。

### 1.3 Template Method：狀態機生命週期

每個裝置經歷統一的狀態序列：

```
CREATE → ATTACH → INIT → ENABLE_OP
                 ↓
             DETACH → DESTROY
```

預設實作全部指向 `dev_do_nothing()`（**Null Object 模式**），晶片驅動只覆寫需要的方法：

```c
// 在 dev_xxx_create() 中：
init_default_dev_object(dev, dev_fvt);  // 全部指向 do_nothing

// 然後選擇性覆寫：
ppca->base.dev_object_fvt->dev_attach = dev_pca_attach;
ppca->base.dev_object_fvt->dev_init   = dev_pca_init;
ppca->base.dev_object_fvt->dev_show   = dev_pca_show;
```

不支援中斷的晶片，`dev_intr_enable` 就保留為 `dev_do_nothing`。

---

## 2. HAL 封裝方式

### 2.1 雙層架構：Call-in / Call-out

最精巧的解耦設計。每個晶片物件有兩個獨立的 FVT：

```
┌─────────────────────────────────────────────────────┐
│                   晶片驅動程式                         │
│                                                      │
│  callin_fvt  ← 裝置提供的功能 (peek_n_poke, test)     │
│  callout_fvt ← 裝置需要的平台服務 (i2c_open/rd/wr)    │
└─────────────────────────────────────────────────────┘
```

**Callout FVT** 就是依賴注入（Dependency Injection）的 C 語言實作：

```c
typedef struct pca_callout_fvt_t_ {
    uint32_t (*open)(n2g_i2c_if_t *);
    uint32_t (*close)(n2g_i2c_if_t *);
    uint32_t (*rd)(n2g_i2c_if_t *);
    uint32_t (*wr)(n2g_i2c_if_t *);
} pca_callout_fvt_t;
```

晶片驅動完全不知道 I2C 是怎麼實現的 — 是 Linux `i2c-dev`、FPGA 模擬的 I2C、或是透過 USB-I2C 橋接器，驅動程式碼完全不需要修改。這就是 **seam** — 一個可以在不編輯該處的情況下改變行為的位置。

### 2.2 I2C HAL：n2g_i2c_if_t

跨所有晶片共用的 I2C 傳輸介面：

```c
typedef struct n2g_i2c_if_s {
    uint8_t  i2c_bus_type;
    uint32_t i2c_dev;
    char    *buf;
    uint32_t size;
} n2g_i2c_if_t;
```

### 2.3 Platform Abstraction Layer (Goofy ASIC)

`dev_goofy_object_t` 封裝一整個 ASIC 平台，提供：

- Global Register (ASIC ID, revision, feature bits)
- GPIO (48+64 bit GPIO/SGPIO control)
- I2C (5 組 I2C master controller)
- Interrupt (MSI, legacy INTx, GPIO interrupt)
- Reset (watchdog, function block reset, device node reset)
- HSIB (internal bus, LUT routing)
- PCIe (upstream/downstream port enumeration)

### 2.4 Fugazi Log / Time / MMAP HAL

`diag_common.h` 提供跨平台基礎設施：

| 設施 | 說明 |
|------|------|
| **Log** | 6 級日誌（FATAL→VDBG），彩色輸出，自動附加 `file:line:func` |
| **Timer** | `fugazi_getticks()` (ms), `udelay()`/`mdelay()` |
| **MMAP** | `/dev/mem` / UIO 物理記憶體存取封裝 |
| **PCI** | Linux `pci_dev` 查詢封裝 |
| **Hex** | hex dump 與 hex-to-bin 轉換 |

---

## 3. 模組解耦方式

### 3.1 目錄結構反映關注點分離

```
common/
├── chips/                     ← 純晶片驅動（對 menu 一無所知）
│   ├── dev_pca9545a_ti/       ← 每種晶片一個目錄
│   ├── dev_cy7c64215_cypress/
│   ├── dev_88e151x_marvell/
│   └── include/               ← 跨晶片共用的常數
│
└── src/
    └── fugazi/                ← 平台整合層（menu + 測試）
        ├── diag.c             ← 主選單定義（struct mitem 陣列）
        ├── diag_common.c      ← HAL 實作
        ├── linux_main.c       ← entry point
        ├── platform_*.c       ← 平台測試（I2C, LED, fan, PSU...）
        └── i2c_api.c/drv.c   ← I2C 驅動實作
```

**chips/** 完全不知道 **menu/** 的存在 — 它們匯出 `dev_xxx_create()` 函式，被上層呼叫。這是 **Dependency Inversion**。

### 3.2 Menu Engine 作為組合根

選單系統透過靜態陣列組裝所有測試：

```c
static struct mitem reggio_fpga_items[] = {
    {"Platform FPGA Program SPI PROM", 0, 0,
     (type_t(*)())program_reggio_spi_prom_old,  &zero, ...},
    {"FPGA intr test", 0, 0,
     (type_t(*)())platform_intr_test,  &one, MF_CONTINUOUS | MF_DOGRP, ...},
};
```

這是 **shallow 的組合層** — 它不做邏輯，只把測試函式掛到選單上。但它的價值在於讓**新增一個測試只需要加一行陣列項目**，符合 Open/Closed 原則。

### 3.3 錯誤回報的 Callback 鏈

錯誤不是直接 `printf()`，而是透過 `dev_error_report` callback 注入：

```c
// 裝置驅動中：
DEV_ERROR_REPORT(dev, "dev_pca_test: Write failed. rc = %#x", PCA_TEST);

// platform 注入 error_report_fn，可決定要 cterr()、printf() 或兩者並行
```

讓診斷框架可以在 headless 產線模式和互動除錯模式之間切換，而不需要改驅動程式碼。

### 3.4 跨平台的關鍵：兩個 FVT 分離

```
  callin_fvt  (device provides to platform)
       ↑              ↑
  ┌────┴──────────────┴──────────────┐
  │         晶片驅動物件                │
  └────┬──────────────┬──────────────┘
       ↓              ↓
  callout_fvt (device needs from platform)
```

因為 **platform 注入 callout**、**device 註冊 callin**，兩邊可以獨立演進。

---

## 4. 深度評估總結

| 面向 | 評估 |
|------|------|
| **dev_object_t 介面深度** | ✅ **Deep** — 14 個方法涵蓋所有晶片生命週期，呼叫者只學一次 |
| **callout FVT 作為 seam** | ✅ **真實的 seam** — 平台可注入不同的 I2C/GPIO/PCI 實作 |
| **Null Object 模式** | ✅ `dev_do_nothing()` 讓選擇性實作成爲預設行爲 |
| **錯誤回報鏈** | ✅ callback 注入，非直接 printf |
| **目錄結構** | ✅ chips/ 與 platform/ 完全隔離 |
| **跨平台移植** | ✅ 同一晶片驅動在 Linux/VxWorks/RTOS 皆可編譯 |
| **menu 組合層** | ✅ 正確的組合根，Open/Closed |
| **n2g_i2c_if_t** | ⚠️ 較 shallow，但符合嵌入式最小介面原則 |

---

## 5. 對 CoreS3 專案的應用建議

### 5.1 繼承 dev_object 模型

M5Stack CoreS3 DFS 中每個 I2C 裝置都適合包成 `dev_object_t` 子類別：

| 晶片 | I2C Addr | 建議的 dev_xxx_object_t |
|------|----------|------------------------|
| AXP2101 | 0x34 | `dev_axp2101_object_t` |
| AW9523B | 0x58 | `dev_aw9523b_object_t` |
| FT6336U | 0x38 | `dev_ft6336u_object_t` |
| BM8563 | 0x51 | `dev_bm8563_object_t` |
| BMI270 | 0x69 | `dev_bmi270_object_t` |
| AW88298 | 0x36 | `dev_aw88298_object_t` |
| ES7210 | 0x40 | `dev_es7210_object_t` |
| GC0308 | 0x21 | `dev_gc0308_object_t` |
| LTR-553 | 0x23 | `dev_ltr553_object_t` |

### 5.2 Call-in / Call-out 分離

- **Callout FVT**: ESP-IDF 的 `i2c_master_*()` / `spi_device_*()` 透過 callout 注入
- **Callin FVT**: 各晶片的 test function（`axp2101_reg_test()`, `bmi270_reg_test()`）

這樣 CoreS3 專案的晶片驅動可以：
- 在 ESP-IDF 上直接運行
- 未來移植到其他平台（Arduino, Zephyr）時只需換 callout 實作

### 5.3 Menu Engine 直接沿用

使用 `submenu_xtable_t` + `struct mitem` 的靜態表格方式：
- 完全 ROMable（無動態配置）
- 適合嵌入式環境
- 新增測試只需一行

### 5.4 錯誤回報鏈

- 生產線模式：`run-all` → PASS/FAIL summary
- EDVT 模式：詳細 register dump + iteration count
- 兩者透過同一個 `dev_error_report` callback 切換

### 5.5 SPI Bus 獨佔鎖

繼承 fugazi 的 SPI 序列化設計，為 CoreS3 的 SPI2（LCD + microSD 共用）實作 acquire/release 機制。

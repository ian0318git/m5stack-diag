# Docs 索引

M5Stack CoreS3 Diagnostic System 的文件總覽。所有文件在此集中管理 —
按「產品規格 / 工程決策 / 開發流程 / 研究與指南」分類。

## 目錄結構

```
docs/
├── README.md                        # 本索引
├── diag_function_spec.md            # DFS — 診斷功能規格（單一真值來源）
├── architecture.md                  # 架構概覽（歷史參考，以 CONTEXT.md 為準）
├── research_bmi270_config_blob.md   # BMI270 config blob 拒絕根因分析
├── mqtt-pub-guide.md                # mqtt-pub 實作與測試指南
├── mqtt-source-code-tutorial.md     # MQTT 原始碼逐層導讀教學（學習用）
├── adr/                             # Architecture Decision Records
│   ├── 0001-abstract-transport-seam.md
│   ├── 0002-spi2-bus-manager.md
│   └── 0003-static-module-state-in-chip-drivers.md
└── agents/                          # Agent 開發流程指導
    ├── domain.md                    # 領域文件閱讀指引
    ├── issue-tracker.md             # GitHub Issues 操作慣例
    └── triage-labels.md             # 標籤對照表
```

## 文件導覽

### 📋 產品規格

| 文件 | 用途 | 讀者 |
|------|------|------|
| [`diag_function_spec.md`](diag_function_spec.md) | **DFS** — 診斷功能規格：測試覆蓋矩陣、測試案例、Failure Analysis、Debugging Steps。由 HFS 透過 `hfs-to-dfs-writer` skill 產生，**是 25 張 ticket 的單一真值來源** | 開發、EDVT、製造 |

### 🏗️ 架構與決策

| 文件 | 用途 | 讀者 |
|------|------|------|
| [`architecture.md`](architecture.md) | 早期架構概覽。⚠️ **部分過時**（記錄舊目錄結構），以 repo 根 `CONTEXT.md` + `docs/adr/` 為準 | 開發 |
| [`adr/0001-abstract-transport-seam.md`](adr/0001-abstract-transport-seam.md) | Transport Seam 決策：chip driver 只能透過 `diag_i2c_t`/`diag_spi_t` 函式指標表與 ESP-IDF 互動 | 開發 |
| [`adr/0002-spi2-bus-manager.md`](adr/0002-spi2-bus-manager.md) | SPI2 bus 共用生命週期（LCD + SD 卡 ref-counted singleton） | 開發 |
| [`adr/0003-static-module-state-in-chip-drivers.md`](adr/0003-static-module-state-in-chip-drivers.md) | Chip driver 用 module-level static state + refcount，不用 heap instance | 開發 |

### 🤖 開發流程（Agent 指導）

| 文件 | 用途 | 讀者 |
|------|------|------|
| [`agents/domain.md`](agents/domain.md) | 探索 codebase 前應閱讀的領域文件指引（CONTEXT.md、ADR） | Agent |
| [`agents/issue-tracker.md`](agents/issue-tracker.md) | GitHub Issues 操作慣例（`gh` CLI 用法） | Agent |
| [`agents/triage-labels.md`](agents/triage-labels.md) | 標準 triage 角色 ↔ repo 實際標籤的對照表 | Agent |

### 🔬 研究與指南

| 文件 | 用途 | 讀者 |
|------|------|------|
| [`research_bmi270_config_blob.md`](research_bmi270_config_blob.md) | BMI270 拒收 Bosch config blob 的根因分析（I2C 通訊正常但 loader 拒收） | 開發 |
| [`mqtt-pub-guide.md`](mqtt-pub-guide.md) | mqtt-pub 實作架構 + 端到端測試 SOP + 排錯指南 | 開發、測試 |
| [`mqtt-source-code-tutorial.md`](mqtt-source-code-tutorial.md) | MQTT 原始碼逐層導讀：命令層→服務層→JSON→事件同步→NVS，含動手練習 | 想學 MQTT/ESP-IDF 的開發者 |

## 文件優先序（讀文件時）

1. **`CONTEXT.md`**（repo 根）— 領域詞彙表與架構概念，最權威
2. **`docs/adr/`** — 架構決策記錄（為什麼這樣設計）
3. **`docs/diag_function_spec.md`** — 功能規格（做什麼、測什麼）
4. 其餘研究/指南文件按需閱讀

> ⚠️ 若文件內容與 ADR 衝突，以 ADR 為準並回報，不要默默覆寫。

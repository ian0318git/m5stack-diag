# BMI270 Config Blob Rejection — Root Cause Analysis

## Problem Statement

The BMI270 IMU (I2C@0x69) on M5Stack CoreS3 correctly responds with
`chip_id=0x24`, confirming I2C communication is functional. However, the
Bosch firmware configuration blob (`bmi270_config_file`, 8192 bytes) is
rejected by the chip's internal microcontroller loader. The register
`INT_STATUS` (0x21) reads 0x02 after loading, whereas the expected done
bit is 0x01 (`BMI270_INT_STAT_DONE`).

---

## Current Implementation Analysis

### File: `common/chips/imu_BMI270/imu_BMI270.c`

**Init sequence** in `imu_BMI270_init()` (lines 102–132):

```
1. read chip_id (0x00)                         → expect 0x24
2. write CMD_SOFTRESET (0xB6 → 0x7C)           → soft reset
3. delay 10 ms
4. load_config()                                ← FAILS (INT_STAT=0x02)
5. write PWR_CONF = 0x00                        ← advance power save DISABLED here (too late)
6. delay 1 ms
7. write PWR_CTRL = ACC_EN | GYR_EN (0x03 → 0x7E)
8. delay 50 ms
```

**`load_config()` register sequence** (lines 55–96):

```
1. write INIT_CTRL (0x59) = 0x00                 ← prepare for config load
2. LOOP: for each 32-byte chunk of bmi270_config_file[8192]:
     word_addr = byte_index / 2
     write INIT_ADDR_0 (0x5B) = word_addr & 0x0F    ← bits [3:0] of word addr
     write INIT_ADDR_1 (0x5C) = (word_addr >> 4)    ← bits [11:4] of word addr
     burst-write INIT_DATA (0x5E) with 32 bytes of config
3. write INIT_CTRL (0x59) = 0x01                 ← trigger firmware load
4. POLL INT_STATUS (0x21) for bit 0, timeout 300 ms → reads 0x02 instead of 0x01
5. return -1
```

The register address encoding is correct per Bosch API (`bmi2.c`): the
config blob is treated as 8192 16-bit words. The word address
(byte_index / 2) is split across INIT_ADDR_0 (bits 3:0) and INIT_ADDR_1
(bits 11:4). This matches the ESPHome implementation and the Bosch
application note BST-BMI270-AN002-01.

### File: `common/chips/imu_BMI270/imu_BMI270_config.h`

- Array size: **8192 bytes** (8 KB), confirmed by counting hex values.
- Source: Bosch `bmi270.c` from BMI270_SensorAPI repository (base config
  with full feature set including motion detection, orientation, tap,
  step counter, etc.).
- First bytes: `0xc8, 0x2e, 0x00, 0x2e, ...` — matches the standard
  configuration blob in the Bosch API.

---

## Bosch Datasheet / API Findings

### Source files consulted

- **`bmi2_defs.h`** (v2.113.0): Register address definitions
- **`bmi270.c` / `bmi270.h`** (v2.86.1): BMI270-specific config blob and init
- **`bmi2.c` / `bmi2.h`**: Generic BMI2-family init/upload functions
- **BST-BMI270-AN002-01**: Bosch application note "BMI270 Initialization"
- **ESPHome `bmi270.cpp`**: Working reference implementation

### Register Addresses (from `bmi2_defs.h`)

| Constant                | Address | Function                        |
|-------------------------|---------|---------------------------------|
| `BMI2_INIT_CTRL_ADDR`   | 0x59    | Init control (start/stop load)  |
| `BMI2_INIT_ADDR_0`      | 0x5B    | Word address bits [3:0]         |
| `BMI2_INIT_ADDR_1`      | 0x5C    | Word address bits [11:4]        |
| `BMI2_INIT_DATA_ADDR`   | 0x5E    | Init data (burst write port)    |
| `BMI2_INTERNAL_STATUS_ADDR` | 0x21 | Internal status / init status |

### Word Address Encoding

The INIT_ADDR registers encode a **12-bit word address** where each word
is 2 bytes (16-bit):

```
word_address = byte_offset / 2
INIT_ADDR_0 = word_address & 0x0F    // bits [3:0]
INIT_ADDR_1 = (word_address >> 4)    // bits [11:4]
```

This matches the current code's encoding exactly.

### Config File Variants (mutually exclusive)

| Variant                  | Size    | Features                               |
|--------------------------|---------|----------------------------------------|
| `bmi270.c` (base)        | ~8192 B | Full feature set (motion, step, etc.)  |
| `bmi270_legacy.c`        | ~8192 B | BMI160-compatible legacy features      |
| `bmi270_maximum_fifo.c`  | ~328 B  | Max FIFO (6 KB), minimal features      |
| `bmi270_context.c`       | ~8192 B | Activity/context recognition           |
| `bmi270_hc.c`            | ~8192 B | Gesture recognition                    |

The current code uses the **base** 8192-byte config. The `max_fifo`
variant (~328 bytes) is a simpler alternative if the base config
continues to fail after the primary fix below.

---

## INT_STATUS Register (0x21) Bit Definitions

From Bosch API `bmi2.h` `bmi2_get_internal_status()` documentation and
community forum reports:

| Value | Symbol              | Meaning                              |
|-------|---------------------|--------------------------------------|
| 0x00  | BMI2_NOT_INIT       | Not initialized                      |
| 0x01  | BMI2_INIT_OK        | **Config load success** (expected)   |
| 0x02  | BMI2_INIT_ERR       | **Config load failure** (observed)   |
| 0x03  | BMI2_DRV_ERR        | Driver error                         |
| 0x04  | BMI2_SNS_STOP       | Sensor stopped                       |
| 0x05  | BMI2_NVM_ERROR      | NVM error                            |
| 0x06  | BMI2_START_UP_ERROR | Startup error                        |
| 0x07  | BMI2_COMPAT_ERROR   | Compatibility error                  |

The mask `BMI2_CONFIG_LOAD_STATUS_MASK = 0x0F` is used by the Bosch API
to extract the load status from register 0x21. The current code checks
`status & BMI27_INT_STAT_DONE (1 << 0)` which is equivalent to checking
bit 0. The observed value 0x02 means the internal microcontroller
signaled a general initialization error.

---

## Comparison with Working Implementations

### ESPHome `bmi270.cpp` (known-working)

```
1. write PWR_CONF = 0x00          ← disable advance power save FIRST
2. delay 1 ms
3. write INIT_CTRL = 0x00
4. LOOP: write config in bursts    ← (same address encoding as current code)
5. write INIT_CTRL = 0x01
6. delay 20 ms                     ← fixed wait, then read
7. read INT_STATUS, mask 0x0F, check == 0x01
```

### Bosch API `bmi2_sec_init()` flow

```
1. bmi2_soft_reset(dev)            ← soft reset, delay 2 ms
2. bmi2_write_config_file(dev):
   a. adjust chunk size to even
   b. write_config_file(dev)       ← uploads config blob
   c. read INTERNAL_STATUS, mask with 0x0F, check == BMI2_CONFIG_LOAD_SUCCESS (1)
```

### Current (broken) implementation

```
1. soft reset, delay 10 ms
2. load_config():
   a. INIT_CTRL = 0x00
   b. write config in 32-byte chunks    ← address encoding is correct
   c. INIT_CTRL = 0x01
   d. poll INT_STATUS for bit 0         ← FAILS: reads 0x02
3. write PWR_CONF = 0x00                 ← DISABLED AFTER load (TOO LATE)
4. write PWR_CTRL = ACC_EN | GYR_EN
```

---

## Root Cause Analysis

**Primary cause: Advance Power Save (APS) not disabled before config loading.**

The BMI270 powers up with Advance Power Save mode enabled by default.
The config loader port (INIT_DATA, INIT_ADDR, INIT_CTRL) requires APS to
be disabled before the firmware configuration blob can be uploaded. This
is explicitly documented in the Bosch application note AN002-01:

> 1. Disable advanced power save: PWR_CONF.adv_power_save = 0b0
> 2. Wait >= 450 us
> 3. Write INIT_CTRL.init_ctrl = 0x00
> 4. Upload config data

The current code writes `PWR_CONF = 0x00` **after** `load_config()`
returns, which means the entire config upload happens while APS is still
active. The BMI270 internal microcontroller cannot access the firmware
memory while APS is enabled, causing the upload to fail with
`INT_STATUS = 0x02 (INIT_ERR)`.

**The register address encoding is correct.** The split of the word
address across INIT_ADDR_0 (bits 3:0) and INIT_ADDR_1 (bits 11:4)
matches the Bosch API and ESPHome implementations.

**The chunk size is appropriate.** 32-byte chunks fit within the I2C
buffer and match the ESPHome implementation on Arduino platforms.

**Polling vs fixed delay is not the issue.** The 300 ms poll timeout
is more than adequate (the Bosch app note says up to 20 ms). The
problem is that the init never succeeds because APS is on.

---

## Recommended Fix

### Fix 1 (Primary): Disable APS before loading config

In `imu_BMI270_init()`, move the `PWR_CONF = 0x00` write to **before**
`load_config()`:

```c
int imu_BMI270_init(const diag_i2c_t *i2c, void *bus)
{
    // ... chip_id check ...

    /* Soft-reset */
    write_reg(BMI270_REG_CMD, BMI270_CMD_SOFTRESET);
    esp_rom_delay_us(10000);

    /* CRITICAL: Disable advance power save before config loading */
    write_reg(BMI270_REG_PWR_CONF, 0x00);
    esp_rom_delay_us(1000);   /* >=450 us per Bosch app note */

    /* Load firmware config (required for sensor data) */
    if (load_config() != 0) {
        return -1;
    }

    /* Enable sensors (PWR_CONF already 0x00 from above) */
    if (write_reg(BMI270_REG_PWR_CTRL, BMI270_ACC_EN | BMI270_GYR_EN) != 0) {
        return -1;
    }
    esp_rom_delay_us(50000);

    return 0;
}
```

### Fix 2 (Backup): Also try reading INT_STATUS twice

Some community forum reports indicate that the `INT_STATUS` register may
need to be read twice to get a stable value. Add this as a safety
measure if Fix 1 alone does not resolve the issue:

```c
/* Read INT_STATUS twice — workaround for register latch issue */
uint8_t status = 0;
read_reg(BMI270_REG_INT_STATUS, &status);  /* discard first read */
read_reg(BMI270_REG_INT_STATUS, &status);  /* actual status */
```

### Fix 3 (Fallback): Switch to max_fifo config if base config fails

If the 8192-byte base config continues to fail after Fix 1, switch to
the `bmi270_maximum_fifo.c` config file (~328 bytes) from the Bosch
API. This simpler config is used by Betaflight and other projects and
has fewer dependencies:

- Source: `BMI270_SensorAPI/bmi270_maximum_fifo.c`
- Size: ~328 bytes
- Trade-off: No advanced features (motion detect, step counter, etc.),
  but basic accel/gyro data and maximum FIFO (6 KB) are available.

### Fix 4 (Future): Use the Bosch API `bmi2.c` upload function

If possible, integrate the Bosch `bmi2.c` `upload_file()` and
`write_config_file()` functions directly instead of reimplementing the
upload protocol. These handle edge cases (odd-length chunks, multi-page
bursts, retry logic) that the simplified implementation may miss.

---

## Config Blob Source Trace

The current `bmi270_config_file[]` in `imu_BMI270_config.h`:

- **Size**: 8192 bytes (8 KB)
- **Source**: Bosch `BMI270_SensorAPI/bmi270.c` array `bmi270_config_file[]`
- **License**: BSD-3-Clause (compatible with MIT project)
- **Variant**: Standard base config (full feature set)

The first bytes `0xc8, 0x2e, 0x00, 0x2e, 0x80, 0x2e, 0x3d, 0xb1` match
the official Bosch `bmi270.c` source. This is not a legacy or max_fifo
variant.

---

## References

| Source | URL |
|--------|-----|
| Bosch BMI270_SensorAPI | https://github.com/boschsensortec/BMI270_SensorAPI |
| Bosch BMG270 AN002-01 (Init) | https://www.bosch-sensortec.com/media/boschsensortec/downloads/application_notes_1/bst-bmi270-an002-01.pdf |
| Bosch BMG270 AN001-01 (Max FIFO) | https://www.bosch-sensortec.com/media/boschsensortec/downloads/application_notes_1/bst-bmi270-an001.pdf |
| Bosch BMI270 AN005-03 | https://www.bosch-sensortec.com/media/boschsensortec/downloads/application_notes_1/bst-bmi270-an005-03.pdf |
| ESPHome BMI270 driver | https://api-docs-dev.esphome.io/bmi270_8cpp_source |
| ESP Component Registry BMI270 | https://components.espressif.com/components/espressif2022/bmi270/versions/1.0.1~1 |
| M5Stack CoreS3 product page | https://docs.m5stack.com/zh_CN/core/CoreS3 |
| Bosch community: BMI270 init error | https://community.bosch-sensortec.com/mems-sensors-forum-jrmujtaw/post/bmi270-i2c-init-esp32-internal-status-bCiynJ4Y3w65QS1 |

---

## Git History (BMI270-related commits)

```
eb0b7ac BMI270: fix config address encoding per Bosch API
3cb89aa BMI270: skip config blob (INT_STAT=0x02), use ROM defaults
a7e2862 BMI270: update INIT_ADDR per chunk during config loading
1038713 BMI270: fix config completion check — use INTERNAL_STATUS (0x21) not INIT_CTRL
baa8f2c BMI270: disable config blob loading (times out), enable basic sensor output
1a65cfe BMI270: fix register map (was reading wrong addresses)
6d37e20 BMI270: fix config loading order and timeout
aa540c5 BMI270: load firmware config blob (Bosch bmi270_config_file)
```

The commit `3cb89aa` explicitly acknowledges the `INT_STAT=0x02` error
and skips config loading entirely. The subsequent commit `eb0b7ac` fixes
the address encoding but does not address the power save issue, so the
config still fails to load.

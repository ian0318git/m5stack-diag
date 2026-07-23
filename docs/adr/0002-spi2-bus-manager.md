# ADR-0002: Shared SPI2 Bus Manager (hal_spi2_bus)

**Status:** Accepted  
**Date:** 2026-07-23  
**Context:** The ILI9342C LCD (CS=G3) and microSD card slot (CS=G4) share SPI2 (MOSI=G37, MISO=G35, SCK=G36). [CONTEXT.md](../../CONTEXT.md) defines the SPI2 Bus concept.

## Decision

Use a ref-counted singleton module (`hal_spi2_bus`) to manage the SPI2 bus lifecycle. The bus is initialised on first use, shared by LCD and SD card device handles, and freed when the last user releases it.

```c
hal_spi2_bus_init();               // ref++ (first call inits SPI2)
hal_spi2_add_lcd_device(&spi);     // add LCD device to bus
hal_spi2_add_sd_device(&sd_handle);// add SD device via sdspi_host
hal_spi2_remove_lcd_device();      // remove LCD device
hal_spi2_bus_deinit();             // ref-- (last call frees SPI2)
```

## Motivation

The LCD and SD card both use SPI2_HOST but through different driver stacks:
- LCD uses the low-level `spi_device_transmit()` API directly
- SD card uses the `sdspi_host` layer which creates its own device handles

Without a shared manager, each test initialised SPI2 independently, causing double-init failures (`ESP_ERR_INVALID_STATE`) when the LCD test ran before the SD card test.

## Alternatives Considered

| Alternative | Reason Rejected |
|-------------|-----------------|
| Independent bus init per test (status quo ante) | Double-init crashes when LCD test runs before SD card test; no coordination mechanism |
| Global mutex around all SPI2 operations | Tests are already sequential in the current architecture; a mutex adds overhead without solving the double-init problem |
| Dedicated SPI bus for each peripheral | ESP32-S3 has only one SPI controller (SPI2) with multiple CS lines; adding a second bus is not physically possible on this SoC |
| Always init through sdspi_host, never through direct SPI API | sdspi_host is designed for SD card protocol, not for ILI9342C display init sequences that require specific SPI mode and transaction flags |

## Consequences

**Positive:**
- LCD test can run before SD card test without conflict
- SD card test reuses the already-initialised bus instead of re-initiing it
- Ref-count correctly handles the case where LCD is initialised and SD card test is skipped (SD card slot empty)
- The manager was code-reviewed (Finding #2: SPI2 bus leak on LCD init failure was found and fixed)

**Negative:**
- Module-level static state (`s_lcd_spi`, `s_sd_handle`, `s_refcount`) is shared across all callers — a bug in one user can corrupt state for all users
- `sdspi_host_init()` is called as a no-op in IDF v6.0; upgrading to a version where it is not idempotent would break the SD card test

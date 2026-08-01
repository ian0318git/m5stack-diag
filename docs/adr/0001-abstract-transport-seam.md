# ADR-0001: Abstract Transport Seam (diag_i2c_t / diag_spi_t)

**Status:** Accepted  
**Date:** 2026-07-23  
**Context:** [CONTEXT.md](../../CONTEXT.md) defines the Transport Seam concept.

## Decision

Use C function-pointer tables (`diag_i2c_t`, `diag_spi_t`) to decouple chip drivers from the platform-specific I2C and SPI implementations. Chip drivers call through these abstract interfaces; platform code provides concrete adapters.

```c
// diag_transport.h — abstract interface, zero platform dependency
typedef struct {
    int (*write)(void *bus, uint16_t addr, const void *data, size_t len);
    int (*read)(void *bus, uint16_t addr, void *data, size_t len);
    int (*write_then_read)(void *bus, uint16_t addr,
                           const void *wdata, size_t wlen,
                           void *rdata, size_t rlen);
    int (*probe)(void *bus, uint16_t addr);
} diag_i2c_t;
```

## Alternatives Considered

| Alternative | Reason Rejected |
|-------------|-----------------|
| Direct ESP-IDF `i2c_master_transmit()` calls in chip drivers | Chip drivers become ESP-IDF-specific; cannot unit-test or port to another platform without rewriting every driver |
| C++ abstract classes | Project is pure C (ESP-IDF convention); introducing C++ adds toolchain complexity and links C++ runtime |
| Macro-based abstraction (`#ifdef ESP_PLATFORM`) | Preprocessor conditionals scatter platform logic across all drivers; testing requires compiling multiple configurations |
| POSIX-style ioctl with device file | Overkill for embedded bare-metal; adds indirection without corresponding benefit |

## Consequences

**Positive:**
- All 10 chip drivers are platform-agnostic — same binary compiles with ESP-IDF adapter or a mock adapter
- Unit-testable: inject a mock `diag_i2c_t` that reads/writes a memory buffer and verify register access patterns
- New platform port (Zephyr, Arduino, Linux userspace) only needs one new adapter file, not changes to 10 chip drivers
- Each adapter function was code-reviewed and verified

**Negative:**
- Every I2C transaction goes through an indirect function call (3-5 cycles overhead on Xtensa LX7) — acceptable for sub-400 kHz I2C
- Chip drivers carry the I2C address as a `#define` constant and pass it on every transaction; the ESP-IDF adapter ignores it (address is baked into the device handle), creating a latent mismatch risk on non-ESP-IDF platforms

## Related

- The reference codebase (`example/platform_diag/`) uses the same pattern with `callin_fvt_t` / `callout_fvt_t` function vector tables, though with separate tables for device-provided and device-needed functions
- ADR-0002 documents the SPI2 bus manager that builds on this seam

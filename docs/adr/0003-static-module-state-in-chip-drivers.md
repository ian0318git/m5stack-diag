# ADR-0003: Static Module State in Chip Drivers

**Status:** Accepted  
**Date:** 2026-07-23  
**Context:** All chip drivers (`aw9523b.c`, `power_AXP2101.c`, `imu_BMI270.c`, etc.) use module-level `static` variables to store the transport pointer and bus handle. [CONTEXT.md](../../CONTEXT.md) defines the Chip Driver concept.

## Decision

Chip drivers store their state in module-level `static` variables. The transport seam pointer (`const diag_i2c_t *s_i2c`) and opaque bus handle (`void *s_bus`) are set at `_init()` time and used by all subsequent operations.

```c
static const diag_i2c_t *s_i2c = NULL;
static void             *s_bus = NULL;
static int               s_refcount = 0;  // AW9523B only
```

## Motivation

On the CoreS3, each I2C peripheral IC is a singleton — there is exactly one AXP2101, one BMI270, one BM8563, etc. There is no scenario where two instances of the same chip driver are needed simultaneously. Static state avoids the complexity of heap-allocated instance handles while achieving the same result.

## Alternatives Considered

| Alternative | Reason Rejected |
|-------------|-----------------|
| Heap-allocated instance struct returned from `_create()` | Adds malloc/free overhead and error paths; every caller must manage the handle lifetime. Unnecessary for single-instance hardware |
| Caller-allocated struct passed as `void *context` to every function | Clutters every API call with a context parameter; callers would need to store and pass the pointer correctly |
| Global registry of device handles | Overkill for 10 chips; adds lookup overhead and a registration step that can fail |

## Consequences

**Positive:**
- Simple API: `chip_init(i2c, bus)` / `chip_read(&data)` / `chip_deinit()` — no handle management for callers
- No heap fragmentation from repeatedly allocating and freeing device instances
- Zero overhead for the singleton case (direct static access, no indirection)
- Ref-count added to AW9523B after code review found that 4 modules shared it, preventing deinit from breaking other callers

**Negative:**
- Not re-entrant — cannot drive two identical chips simultaneously (not needed on CoreS3)
- Module state persists after `_deinit()` until explicitly overwritten by the next `_init()`
- GC0308 driver was caught without a deinit function during code review (Finding #11); this was fixed

## Related

- The fugazi reference codebase uses the same pattern in all chip drivers
- AW9523B uses an extended version with `s_refcount` to allow safe sharing across 4 board adapters

#ifndef __HIGHRISE_COMMN_UTIL_H__
#define __HIGHRISE_COMMN_UTIL_H__
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>

#define _DRAIN_STDIN() do {                      \
    int flg___ = 0;                              \
    int flg___bak = 0;                           \
    char ch___ = 0;                              \
    flg___    = fcntl(STDIN_FILENO, F_GETFL, 0); \
    flg___bak = flg___;                          \
    flg___    |= O_NONBLOCK;                     \
    fcntl(STDIN_FILENO, F_SETFL, flg___);        \
    while((ch___ = getchar()) != '\n' && ch___ != EOF); \
    fcntl(STDIN_FILENO, F_SETFL, flg___bak);     \
}while(0)

#define BIT_START(MSK____, S____) ({   \
    int i____ = 0;          \
    for(i____ = S____; (1 << i____) <= (MSK____) && !((1 << i____) & (MSK____)); i____++); \
    i____;                  \
})

#define BIT_END(MSK____, S____) ({     \
    int i____ = 0;          \
    for(i____ = S____; (1 << i____) <= (MSK____); i____++); \
    i____;                  \
})

#define BITS_PER_U32        32
#define BITS_SHIFT_U32      5
#define BIT(nr)             (1UL << (nr))
#define BIT_MASK(nr)        (1UL  << ((nr) & (BITS_PER_U32 - 1)))
#define BIT_TO_U32(nr)      ((nr) >> BITS_SHIFT_U32)
#define BIT_IDX_IN_U32(nr)  ((nr) & (BITS_PER_U32 - 1))

/*
 * Desc:
 *  Bits sequence are in any size(32, 64, 128, ...) of multiple of 32.
 *  !!!Caller guarantee the bitmap buffer boundary is valid with the params.
 * Param:
 *  dst      - target bit buf
 *  dst_off  - start bit offset of dst bit buf
 *  src      - source bit buf
 *  src_off  - start bit offset of src bit buf
 *  width    - how many bits would be copied.
 * Return:
 *
 */
static inline int copy_bits(uint32_t *dst, uint32_t dst_off, const uint32_t *src, uint32_t src_off, uint32_t width)
{
    uint32_t soff = src_off;
    uint32_t slen = 0;
    uint32_t doff = dst_off;
    uint32_t dlen = 0;
    int bits_left = width;

    const uint64_t one = 1;


    while(bits_left > 0) {
        slen = BIT_IDX_IN_U32(soff) + bits_left > BITS_PER_U32 ? BITS_PER_U32 - BIT_IDX_IN_U32(soff) : bits_left;
        dlen = BIT_IDX_IN_U32(doff) + bits_left > BITS_PER_U32 ? BITS_PER_U32 - BIT_IDX_IN_U32(doff) : bits_left;
        slen = slen > dlen ? dlen : slen;
        dlen = slen;

        //clear 'bit range in dst' and then OR with 'bit range in src'
        dst[BIT_TO_U32(doff)] = \
            (dst[BIT_TO_U32(doff)] & (~(((one << dlen) - 1) << BIT_IDX_IN_U32(doff))))  | \
          (((src[BIT_TO_U32(soff)] & ( (((one << slen) - 1) << BIT_IDX_IN_U32(soff)))) >> \
                BIT_IDX_IN_U32(soff)) << BIT_IDX_IN_U32(doff));

        soff += slen;
        doff += dlen;

        if (bits_left < slen)
            return -1;

        bits_left -= slen;
    }

    return width;
}

static inline int test_bit(const uint32_t *bitmap, int bitidx)
{
    return (bitmap[BIT_TO_U32(bitidx)] & (1UL << BIT_IDX_IN_U32(bitidx))) != 0;
}

static inline void set_bit(uint32_t *bitmap, int bitidx)
{
    bitmap[BIT_TO_U32(bitidx)] |=   1UL << BIT_IDX_IN_U32(bitidx);
}

static inline int set_bits32(uint32_t *bitmap, uint32_t val, uint32_t off, uint32_t width)
{
    return copy_bits(bitmap, off, &val, 0, width);
}

static inline int get_bits32(uint32_t *bitmap, uint32_t *val, uint32_t off, uint32_t width)
{
    return copy_bits(val, 0, bitmap, off, width);
}

//return 1 or 0
static inline uint32_t get_bit(const uint32_t *bitmap, int bitidx)
{
    return (bitmap[BIT_TO_U32(bitidx)] & (1UL << BIT_IDX_IN_U32(bitidx))) >> BIT_IDX_IN_U32(bitidx);
}

static inline void clr_bit(uint32_t *bitmap, int bitidx)
{
    bitmap[BIT_TO_U32(bitidx)] &= ~(1UL << BIT_IDX_IN_U32(bitidx));
}


#define foreach_set_bit(map__, iter__, iterS__, iterE__) \
for((iter__) = iterS__, ({while(((iter__) < iterE__) && !test_bit(map__,(iter__)))(iter__)++;}); \
    (iter__) < iterE__; \
    (iter__)++,         ({while(((iter__) < iterE__) && !test_bit(map__,(iter__)))(iter__)++;}))

#undef BITS_PER_U32
#undef BITS_SHIFT_U32
#undef BIT
#undef BIT_MASK
#undef BIT_TO_U32
#undef BIT_IDX_IN_U32

/*
 * !!!Do not use 'break' like statement('goto', ...),!!!
 * !!!instead use 'P=NULL' to stop the loop.         !!!
 * p    - char *    the iter ptr
 * str  - char *    the string buffer
 * ch   - char      the dilimiter char
 * t    - char *    the tmp ptr
 */
#define forpart(P, str, ch, t)          \
for(                                    \
    ({                                  \
        (t) = NULL;                     \
        (P) = (str);                    \
        if ((P)) {                      \
            (t) = strchr((P), ch);      \
            if (t) *(t) = 0;            \
        }                               \
    });                                 \
                                        \
    (P);                                \
                                        \
    ({                                  \
        if (t) *(t) = ch;               \
        else (P) = NULL;                \
        if ((P)) {                      \
            if (t) {                    \
                (P) = t + 1;            \
                t = strchr((P), ch);    \
                if (t) *(t) = 0;        \
            }                           \
        }                               \
    })                                  \
)

//delimi by string 'sub'
#define forpart_ext(P, str, sub, t)     \
for(                                    \
    ({                                  \
        (P) = (str);                    \
        if ((P)) {                      \
            (t) = strstr((P), sub);     \
            if (t) *(t) = 0;            \
        }                               \
    });                                 \
                                        \
    (P) && (sub);                       \
                                        \
    ({                                  \
        if (t) *(t) = (sub)[0];         \
        else (P) = NULL;                \
        if ((P)) {                      \
            if (t) {                    \
                (P) = t + strlen(sub);  \
                t = strstr((P), sub);   \
                if (t) *(t) = 0;        \
            }                           \
        }                               \
    })                                  \
)

/*
 * !!! see comment of 'forpart()'
 */
#define forline(L, str, t) forpart(L, str, '\n', t)

extern int _hr_commn_util_log_lvl;
#define _HR_COMMN_UTIL_LOG_LVL_ENV_NAME "HR_COMMN_UTIL_LOG_LVL"
#define LOG_LVL_DBG 4
#define LOG_LVL_INF 3
#define LOG_LVL_WRN 2
#define LOG_LVL_ERR 1
#define LOG_LVL_AWS 0
#define LOG_LVL_LOG(LVL, fmt____, ...) do {                                                \
    if (_hr_commn_util_log_lvl >= (LOG_LVL_##LVL)) {                                        \
        printf("%s:%-24s:%04d:" fmt____, #LVL,                                              \
            ({const char *p____ = strrchr(__FILE__, '/'); p____ ? p____ + 1 : __FILE__;}), \
            __LINE__, ## __VA_ARGS__);                                                     \
    }                                                                                      \
}while(0)

#define LOG_DBG(fmt____, ...) LOG_LVL_LOG(DBG, fmt____, ##__VA_ARGS__)
#define LOG_INF(fmt____, ...) LOG_LVL_LOG(INF, fmt____, ##__VA_ARGS__)
#define LOG_WRN(fmt____, ...) LOG_LVL_LOG(WRN, fmt____, ##__VA_ARGS__)
#define LOG_ERR(fmt____, ...) LOG_LVL_LOG(ERR, fmt____, ##__VA_ARGS__)
#define LOG_AWS(fmt____, ...) LOG_LVL_LOG(AWS, fmt____, ##__VA_ARGS__)
#define LOG_LOG LOG_AWS

static int inline get_env_int(const char *env_name, int *var, int def_val) {
    int   i = 0;
    char *p = NULL;

    *var = def_val;

    p = getenv(env_name);
    if (p) {
        for(i = 0; p[i]; i++) {
            if (!(p[i] >= '0' && p[i] <= '9'))
                return -1;
        }
        return sscanf(p, "%d", var);
    }
    return 0;
}

extern int strcasecmp (const char *s1, const char *s2);
static int inline hr_commn_util_log_lvl_set(char *lvl)
{
    if (lvl == NULL) {
        return get_env_int(_HR_COMMN_UTIL_LOG_LVL_ENV_NAME, &_hr_commn_util_log_lvl, LOG_LVL_ERR);
    }

    if (strcasecmp("debug", lvl) == 0) _hr_commn_util_log_lvl = LOG_LVL_DBG; else
    if (strcasecmp("info" , lvl) == 0) _hr_commn_util_log_lvl = LOG_LVL_INF; else
    if (strcasecmp("warn" , lvl) == 0) _hr_commn_util_log_lvl = LOG_LVL_WRN; else
    if (strcasecmp("error", lvl) == 0) _hr_commn_util_log_lvl = LOG_LVL_ERR; else
                                     _hr_commn_util_log_lvl = LOG_LVL_ERR;
    return 0;
}

#define URET_COND(cond____, retv____, t____, fmt____, ...)  \
do{                                                         \
    if (cond____) {                                         \
        if (fmt____)                                        \
            printf("%s:%-20s:%d:" fmt____, t____,              \
                    ({const char *p____ = strrchr(__FILE__, '/'); p____ ? p____ + 1 : __FILE__;}), \
                    __LINE__, ## __VA_ARGS__);              \
        uret = retv____;                                    \
        goto _EXIT_POINT;                                   \
    }                                                       \
}while(0)

#define RET_COND(cond____, retv____, t____, fmt____, ...)   \
do{                                                         \
    if (cond____) {                                         \
        if (fmt____)                                        \
            printf("%s:%-20s:%d:" fmt____, t____,              \
                    ({const char *p____ = strrchr(__FILE__, '/'); p____ ? p____ + 1 : __FILE__;}), \
                    __LINE__, ## __VA_ARGS__);              \
        return retv____;                                    \
    }                                                       \
}while(0)

#define ERR_URET_COND(cond____, retv____, fmt____, ...) \
    URET_COND(cond____, retv____, "ERR", fmt____, ##__VA_ARGS__)

#define ERR_RET_COND(cond____, retv____, fmt____, ...) \
    RET_COND(cond____, retv____, "ERR", fmt____, ##__VA_ARGS__)

#define TERR_RET_COND(cond____, retv____, fmt____, ...)                                           \
do{                                                                                               \
    if (cond____) {                                                                               \
        if (fmt____)                                                                              \
            cterr('f', '0', "ERR:%-20s:%d:" fmt____,                                              \
                    ({const char *p____ = strrchr(__FILE__, '/'); p____ ? p____ + 1 : __FILE__;}),\
                    __LINE__, ## __VA_ARGS__);                                                    \
        return retv____;                                                                          \
    }                                                                                             \
}while(0)

#define TERR_URET_COND(cond____, retv____, fmt____, ...)                                          \
do{                                                                                               \
    if (cond____) {                                                                               \
        uret = retv____;                                                                          \
        if (fmt____)                                                                              \
            cterr('f', '0', "ERR:%-20s:%d:" fmt____,                                              \
                    ({const char *p____ = strrchr(__FILE__, '/'); p____ ? p____ + 1 : __FILE__;}),\
                    __LINE__, ## __VA_ARGS__);                                                    \
        goto _EXIT_POINT;                                                                         \
    }                                                                                             \
}while(0)

static inline void *mmap_reg_space(unsigned int paddr, unsigned int size)
{
    void     *vaddr = NULL;
    int       fd    = 0;
    long      pgsz  = 0;
    unsigned int page  = 0;

    LOG_DBG("Enter '%s'\n", __func__);
    fd = open("/dev/mem", O_RDWR|O_SYNC);
    ERR_RET_COND(!(fd > 0), MAP_FAILED, "Open '/dev/mem' failed, err:%s\n", strerror(errno));

    pgsz  = sysconf(_SC_PAGESIZE);
    page  = (paddr & (~(pgsz - 1)));
    size  = ((paddr + size + pgsz - 1) & (~(pgsz - 1))) - page;
    vaddr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, page);

    ERR_RET_COND(vaddr == MAP_FAILED || close(fd), MAP_FAILED, "Map reg space failed.\n");

    close(fd);

    return vaddr + (paddr - page);
}

static inline int munmap_reg_space(void *vaddr, unsigned int paddr, unsigned int size)
{
    long         pgsz = 0;
    unsigned int page = 0;

    LOG_DBG("Enter '%s'\n", __func__);
    pgsz  = sysconf(_SC_PAGESIZE);
    page  = (paddr & (~(pgsz - 1)));
    size  = ((paddr + size + pgsz - 1) & (~(pgsz - 1))) - page;

    ERR_RET_COND(0 > munmap(vaddr - (paddr - page), size), -(__LINE__), "Unmap 0x%p failed.\n", vaddr);

    return 0;
}
#endif

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "linux_gpio.h"
#include "hr_commn_util.h"

#define GPIO_BASE                0xF2440000
#define GPIO_SPACE_SIZE          0x1000

static unsigned int TCK_GPIO = 26;
static unsigned int TDI_GPIO = 22;
static unsigned int TMS_GPIO = 21;
static unsigned int TDO_GPIO = 23;

#define PRG_ENB_GPIO 25
#define PRG_ENB_VAL  0x0

#define INPUT   0
#define OUTPUT  1
#define LOW     0
#define HIGH    1

static volatile unsigned char *gpio_base = NULL;

static int _gpio_space_init()
{
    void *p = NULL;
    ERR_RET_COND(gpio_base != NULL, -(__LINE__), "GPIO space has already been initilized.\n");
    p = mmap_reg_space(GPIO_BASE, GPIO_SPACE_SIZE);
    ERR_RET_COND(p == MAP_FAILED, -(__LINE__), "Failed map gpio space\n");
    gpio_base = (volatile unsigned char *)p;
    return 0;
}

static int _gpio_space_free()
{
    ERR_RET_COND(gpio_base == NULL, -(__LINE__), "GPIO space has not been mapped yet.\n");
    ERR_RET_COND(munmap_reg_space((void*)gpio_base, GPIO_BASE, GPIO_SPACE_SIZE) < 0,
                -(__LINE__), "Unmape GPIO space failed.\n");
    gpio_base = NULL;
    return 0;
}

//MPP_Control_<n*8>_<n*8+7> Register
#define _GPIO_FUN_SEL(PIN, SEL) do {                                       \
    static volatile uint32_t *regp = NULL;                                 \
    static uint32_t  boff = 0;                                             \
    assert(gpio_base != NULL);                                             \
    regp  = (volatile uint32_t *)((((PIN) >> 3) << 2) + gpio_base);        \
    boff  = ((PIN) & 0x7) << 2;                                            \
    *regp = ((*regp) & (~(0xf << boff))) | ((SEL) << boff);                \
}while(0)

//GPIO <n> DataOut Register
#define _GPIO_OUT_SET(PIN, VAL) do {                                       \
    static volatile uint32_t *regp = NULL;                                 \
    static uint32_t  boff = 0;                                             \
    assert(gpio_base != NULL);                                             \
    regp = (volatile uint32_t *)((((PIN) >> 5) << 6) + gpio_base + 0x100); \
    boff = (PIN) & 0x1f;                                                   \
    *regp= ((*regp) & (~(1 << boff))) | ((VAL) << boff);                   \
}while(0)

//GPIO <n> Control Set / Clear Register
#define _GPIO_DIR_SET(PIN, DIR) do {                                       \
    static volatile uint32_t *regp = NULL;                                 \
    static uint32_t  boff = 0;                                             \
    assert(gpio_base != NULL);                                             \
    regp = (volatile uint32_t *)(((DIR) == OUTPUT) ?                       \
        (((PIN) >> 5) << 6) + gpio_base + 0x12c                            \
        :                                                                  \
        (((PIN) >> 5) << 6) + gpio_base + 0x128);                          \
    boff = (PIN) & 0x1f;                                                   \
    *regp= 1 << boff;                                                      \
}while(0)

//GPIO <n> Blink Enable Register
#define _GPIO_BLK_ENB(PIN, ENB) do {                                       \
    static volatile uint32_t *regp = NULL;                                 \
    static uint32_t  boff = 0;                                             \
    assert(gpio_base != NULL);                                             \
    regp = (volatile uint32_t *)((((PIN) >> 5) << 6) + gpio_base + 0x108); \
    boff = (PIN) & 0x1f;                                                   \
    *regp= ((*regp) & (~(1 << boff))) | ((ENB) << boff);                   \
}while(0)

//GPIO <n> Data In Polarity Register
#define _GPIO_POL_SET(PIN, VAL) do {                                       \
    static volatile uint32_t *regp = NULL;                                 \
    static uint32_t  boff = 0;                                             \
    assert(gpio_base != NULL);                                             \
    regp = (volatile uint32_t *)((((PIN) >> 5) << 6) + gpio_base + 0x10c); \
    boff = (PIN) & 0x1f;                                                   \
    *regp= ((*regp) & (~(1 << boff))) | ((VAL) << boff);                   \
}while(0)

//GPIO <n> DataOut Register
#define _GPIO_STA_GET(PIN) ({                                              \
    volatile uint32_t *regp = NULL;                                        \
    uint32_t  boff = 0;                                                    \
    assert(gpio_base != NULL);                                             \
    regp = (volatile uint32_t *)((((PIN) >> 5) << 6) + gpio_base + 0x110); \
    boff = (PIN) & 0x1f;                                                   \
    (*(regp + 1)) & (1 << boff) ? ((*regp & (1 << boff)) ? 0 : 1) : ((*regp & (1 << boff)) ? 1 : 0); \
})

static int _gpio_dump(unsigned int pin, char *pin_name)
{
    volatile uint32_t *regp = NULL;
    uint32_t  boff = 0;
    uint32_t  sel = 0;
    uint32_t  vout= 0;
    uint32_t  dir = 0;
    uint32_t  blnk= 0;
    uint32_t  pol = 0;
    uint32_t  vin = 0;

    assert(gpio_base != NULL);

    regp = (volatile uint32_t *)(((pin >> 3) << 2) + gpio_base);
    boff = (pin & 0x7) << 2;
    sel  = ((*regp) >> boff) & 0x0f;

    regp = (volatile uint32_t *)(((pin >> 5) << 6) + gpio_base + 0x100);
    boff = pin & 0x1f;
    vout = ((*regp) >> boff) & 0x01;

    regp = (volatile uint32_t *)(((pin >> 5) << 6) + gpio_base + 0x104);
    boff = (pin) & 0x1f;
    dir  = ((*regp) >> boff) & 0x01;

    regp = (volatile uint32_t *)(((pin >> 5) << 6) + gpio_base + 0x108);
    boff = pin & 0x1f;
    blnk = ((*regp) >> boff) & 0x01;

    regp = (volatile uint32_t *)(((pin >> 5) << 6) + gpio_base + 0x10c);
    boff = pin & 0x1f;
    pol  = ((*regp) >> boff) & 0x01;

    regp = (volatile uint32_t *)((((pin) >> 5) << 6) + gpio_base + 0x110);
    boff = (pin) & 0x1f;
    vin  = pol ? ((*regp & (1 << boff)) ? 0 : 1) : ((*regp & (1 << boff)) ? 1 : 0);

    printf("GPIO-%u-%s:\n"
           "    func sel :%u\n"
           "    direction:%s\n"
           "    value    :%s(Already inverted if Input if needed)\n"
           "    in ploar :%s\n"
           "    blink    :%s\n",
           pin, pin_name,
           sel,
           dir == 0 ? "output" : "input",
           dir == 0 ? (vout == 0 ? "Low" : "High") : (vin == 0 ? "Low" : "High"),
           pol == 0 ? "Not invert" : "Invert",
           blnk== 0 ? "Not" : "Yes"
        );

    return 0;
}

static int _gpio_init(unsigned int pin, unsigned int dir)
{
    _GPIO_FUN_SEL(pin, 0);
    _GPIO_OUT_SET(pin, 0);
    _GPIO_DIR_SET(pin, dir);
    _GPIO_BLK_ENB(pin, 0);
    _GPIO_POL_SET(pin, 0);
    return 0;
}

static int invert_tdo_tdi(void) {
    char *p = NULL;
    unsigned int tmp = 0;

    p = getenv("JAMPL_GPIO_EXCHANGE");
    if (p && (strcasecmp(p, "Y") == 0 ||
              strcasecmp(p, "yes") == 0 ||
              strcmp(p, "1") == 0)) {

        tmp = TDI_GPIO;
        TDI_GPIO = TDO_GPIO;
        TDO_GPIO = tmp;
        printf("Set TDO to GPIO-%u, TDI to GPIO-%u\n", TDO_GPIO, TDI_GPIO);
    }

    return 0;
}

int gpio_init_jtag(void)
{
    invert_tdo_tdi();
    printf("TDI-%u, TDO-%u, TMS-%u, TCK-%u\n", TDI_GPIO, TDO_GPIO, TMS_GPIO, TCK_GPIO);

    ERR_RET_COND(_gpio_space_init() < 0, -(__LINE__), "Failed.\n");

    _gpio_init(TCK_GPIO, OUTPUT);
    _gpio_init(TDI_GPIO, OUTPUT);
    _gpio_init(TMS_GPIO, OUTPUT);
    _gpio_init(TDO_GPIO, INPUT);
    _GPIO_POL_SET(TDO_GPIO, 1);

    _gpio_init(PRG_ENB_GPIO, OUTPUT);
    _GPIO_OUT_SET(PRG_ENB_GPIO, PRG_ENB_VAL == 0 ? 0 : 1);

    _gpio_dump(TCK_GPIO    , "jtag_tck");
    _gpio_dump(TDI_GPIO    , "jtag_tdi");
    _gpio_dump(TMS_GPIO    , "jtag_tms");
    _gpio_dump(TDO_GPIO    , "jtag_tdo");
    _gpio_dump(PRG_ENB_GPIO, "prog_enb");

    return 0;
}

int gpio_free_jtag(void)
{
    _GPIO_OUT_SET(PRG_ENB_GPIO, PRG_ENB_VAL == 0 ? 1 : 0);
    _GPIO_DIR_SET(PRG_ENB_GPIO, INPUT);

    _GPIO_OUT_SET(TCK_GPIO, LOW);
    _GPIO_DIR_SET(TCK_GPIO, INPUT);

    _GPIO_OUT_SET(TDI_GPIO, LOW);
    _GPIO_DIR_SET(TDI_GPIO, INPUT);

    _GPIO_OUT_SET(TMS_GPIO, LOW);
    _GPIO_DIR_SET(TMS_GPIO, INPUT);

    _GPIO_OUT_SET(TDO_GPIO, LOW);
    _GPIO_DIR_SET(TDO_GPIO, INPUT);

    ERR_RET_COND(_gpio_space_free() < 0, -(__LINE__), "Failed.\n");
    return 0;
}

void gpio_set_tdi(int tdi)
{
    _GPIO_OUT_SET(TDI_GPIO, tdi == 0 ? LOW : HIGH);
}

void gpio_set_tms(int tms)
{
    _GPIO_OUT_SET(TMS_GPIO, tms == 0 ? LOW : HIGH);
}

void gpio_set_tck(int tck)
{
    _GPIO_OUT_SET(TCK_GPIO, tck == 0 ? LOW : HIGH);
}

unsigned int gpio_get_tdo()
{
    return _GPIO_STA_GET(TDO_GPIO);
}


#ifdef JAMPL_GPIO_TEST_MAIN
int main(int argc, char *argv[])
{
    int i = 0;
    hr_commn_util_log_lvl_set(NULL);
    invert_tdo_tdi();

    printf("Test only GPIO TCK to check gpion API workable.\n\n");
    printf("TDI-%u, TDO-%u, TMS-%u, TCK-%u\n", TDI_GPIO, TDO_GPIO, TMS_GPIO, TCK_GPIO);

    ERR_RET_COND(_gpio_space_init() < 0, -(__LINE__), "Failed.\n");

    while(i < 2) {
        printf("########### %d ##############\n", i);
        printf("\nSet to output\n");
        _gpio_init(TCK_GPIO, OUTPUT);
        _gpio_dump(TCK_GPIO, "jtag_tck");
        printf("\nSet to LOW\n");
        gpio_set_tck(0);
        _gpio_dump(TCK_GPIO, "jtag_tck");

        printf("\nSet to HIGH\n");
        gpio_set_tck(1);
        _gpio_dump(TCK_GPIO, "jtag_tck");

        printf("\nSet to LOW\n");
        gpio_set_tck(0);
        _gpio_dump(TCK_GPIO, "jtag_tck");

        printf("\n----------------------\n");

        printf("\nSet to input\n");
        _gpio_init(TCK_GPIO, INPUT);
        _gpio_dump(TCK_GPIO, "jtag_tck");

        printf("\nSet to LOW\n");
        gpio_set_tck(0);
        _gpio_dump(TCK_GPIO, "jtag_tck");

        i++;
    }
    return 0;
}
#endif

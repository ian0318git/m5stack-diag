/*------------------------------------------------------------------
 * tam_stubs.c -- dummy TAM/EPDM/PCI function stubs for build linking
 *
 * This file provides empty implementations of TAM SDK, Broadcom EPDM,
 * and PCI utility functions to satisfy linker dependencies when the
 * real SDK libraries are not available.
 *
 * All functions return success (0 or TAM_RC_OK) to allow the binary
 * to link.  This is NOT functional — diagnostics requiring TAM/EPDM/PCI
 * hardware will fail at runtime.
 *------------------------------------------------------------------
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "tam_library.h"
#include "tam_lib_manufacturing.h"
#include "tam_lib_object_manager.h"
#include "epdm.h"
#include "epdm_sec.h"

/*========================================================================
 * TAM Core API stubs
 *========================================================================*/

tam_lib_status_t
tam_lib_get_library_version(tam_library_version_t *tam_version)
{
    memset(tam_version, 0, sizeof(*tam_version));
    return TAM_RC_OK;
}

void tam_lib_display_library_version(void) {}

tam_lib_status_t
tam_lib_device_open(void *platform_opaque_handle,
                    uint16_t platform_max_bus_length,
                    void **tam_handle)
{
    *tam_handle = (void *)0x1; /* dummy non-NULL handle */
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_device_close(void **tam_handle)
{
    *tam_handle = NULL;
    return TAM_RC_OK;
}

uint8_t tam_lib_check_mode(void *tam_handle) { return BUS_MODE_SIMPLE; }

tam_lib_status_t
tam_lib_set_simple(void *tam_handle) { return TAM_RC_OK; }

tam_lib_status_t
tam_lib_get_chip_serial_number(void *tam_handle, uint8_t *chip_serial_number)
{
    memset(chip_serial_number, 0, CHIP_SERIAL_NUMBER_LENGTH);
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_get_chip_info(void *tam_handle, tam_lib_chip_info_t *chip_info)
{
    memset(chip_info, 0, sizeof(*chip_info));
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_scc_read_id(void *tam_handle, tam_lib_scc_id_t *scc_id)
{
    memset(scc_id, 0, sizeof(*scc_id));
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_scc_read_eeprom(void *tam_handle, uint8_t *dest_buffer,
                        uint16_t length, uint16_t src)
{
    memset(dest_buffer, 0, length);
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_scc_write_eeprom(void *tam_handle, uint8_t *src_buffer,
                         uint16_t length, uint16_t dest)
{
    return TAM_RC_OK;
}

char *tam_lib_rc2string(uint8_t status)
{
    static char buf[32];
    snprintf(buf, sizeof(buf), "RC:0x%02x", status);
    return buf;
}

char *tam_lib_object_type2string(uint8_t object_type)
{
    static char buf[32];
    snprintf(buf, sizeof(buf), "OBJ:0x%02x", object_type);
    return buf;
}

char *tam_lib_storage_location2string(uint8_t location)
{
    static char buf[32];
    snprintf(buf, sizeof(buf), "LOC:0x%02x", location);
    return buf;
}

char *tam_lib_storage_mode2string(uint8_t storage)
{
    static char buf[32];
    snprintf(buf, sizeof(buf), "MODE:0x%02x", storage);
    return buf;
}

char *tam_lib_predefined2string(uint8_t predefined_oid)
{
    static char buf[32];
    snprintf(buf, sizeof(buf), "OID:0x%02x", predefined_oid);
    return buf;
}

char *tam_lib_symm_algo2string(uint8_t symm_algo)
{
    static char buf[32];
    snprintf(buf, sizeof(buf), "ALGO:0x%02x", symm_algo);
    return buf;
}

char *tam_lib_symm_mode2string(uint8_t symm_mode)
{
    static char buf[32];
    snprintf(buf, sizeof(buf), "MODE:0x%02x", symm_mode);
    return buf;
}

/*========================================================================
 * TAM Authentication stubs
 *========================================================================*/

tam_lib_status_t
tam_lib_authentication(void *tam_handle) { return TAM_RC_OK; }

tam_lib_status_t
tam_lib_authentication_udi(void *tam_handle, char *pid, char *sn,
                           uint8_t cert_type) { return TAM_RC_OK; }

tam_lib_status_t
tam_lib_aik_validate_certchain_udi(void *tam_handle,
                                   uint32_t session_id,
                                   char *prod_name,
                                   char *prod_sn) { return TAM_RC_OK; }

/*========================================================================
 * TAM Admin / Session stubs
 *========================================================================*/

tam_lib_status_t
tam_lib_generate_admin_pin(void *tam_handle, uint8_t *admin_pin)
{
    memset(admin_pin, 0, TAM_PIN_LENGTH);
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_session_init(void *tam_handle, uint8_t user_id, uint8_t *pin,
                     uint32_t *session_id)
{
    *session_id = 1;
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_session_end(void *tam_handle, uint32_t session_id) { return TAM_RC_OK; }

tam_lib_status_t
tam_lib_user_add(void *tam_handle, uint32_t admin_session_id,
                 uint8_t user_id, uint8_t *pin) { return TAM_RC_OK; }

tam_lib_status_t
tam_lib_user_delete(void *tam_handle, uint32_t admin_session_id,
                    uint8_t user_id) { return TAM_RC_OK; }

tam_lib_status_t
tam_lib_admin_login(void *tam_handle, uint32_t *admin_session_id)
{
    *admin_session_id = 1;
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_admin_logout(void *tam_handle, uint32_t admin_session_id)
{
    return TAM_RC_OK;
}

/*========================================================================
 * TAM Object stubs
 *========================================================================*/

tam_lib_status_t
tam_lib_object_create(void *tam_handle, uint32_t session_id,
                      uint8_t object_type, uint16_t object_size,
                      uint8_t csp_flag, uint8_t memory_flag,
                      uint8_t encryption_flag, uint32_t *object_id)
{
    *object_id = 1;
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_object_delete(void *tam_handle, uint32_t session_id,
                      uint32_t object_id) { return TAM_RC_OK; }

tam_lib_status_t
tam_lib_object_read(void *tam_handle, uint32_t session_id,
                    uint32_t object_id, uint8_t *buffer, uint16_t *length)
{
    memset(buffer, 0, *length);
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_object_write(void *tam_handle, uint32_t session_id,
                     uint32_t object_id, uint8_t *buffer, uint16_t length)
{
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_object_enumerate(void *tam_handle, uint32_t session_id,
                         uint8_t *num_objects,
                         tam_lib_object_enum_couplet_t *object_list)
{
    *num_objects = 0;
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_object_list(void *tam_handle, uint32_t session_id,
                    uint16_t index_start, uint8_t couplet_max,
                    uint16_t *num_objects, uint8_t *couplet_count,
                    tam_lib_object_enum_couplet_t *object_list)
{
    *num_objects = 0;
    *couplet_count = 0;
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_object_readinfo(void *tam_handle, uint32_t session_id,
                        uint32_t object_id, uint8_t *object_type,
                        uint16_t *object_size)
{
    *object_type = TAM_INVALID_OBJECT;
    *object_size = 0;
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_object_attributes(void *tam_handle, uint32_t session_id,
                          uint32_t object_id,
                          tam_lib_object_attributes_t *attributes)
{
    memset(attributes, 0, sizeof(*attributes));
    return TAM_RC_OK;
}

/*========================================================================
 * TAM TRNG stubs
 *========================================================================*/

tam_lib_status_t
tam_lib_trand_check(void *tam_handle, uint32_t admin_session_id)
{
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_trand_read(void *tam_handle, uint16_t length, uint8_t *buffer)
{
    memset(buffer, 0, length);
    return TAM_RC_OK;
}

/*========================================================================
 * TAM Manufacturing stubs
 *========================================================================*/

tam_lib_status_t
tam_lib_mfg_ecskmp_generate(void *tam_handle, uint8_t *already_clliped)
{
    *already_clliped = 0;
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_mfg_ecskmp_read(void *tam_handle, uint8_t *buffer, uint16_t *length)
{
    memset(buffer, 0, *length);
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_mfg_login_init(void *tam_handle, uint16_t credentials_length,
                       uint8_t *mfg_nonce)
{
    memset(mfg_nonce, 0, credentials_length);
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_mfg_login_credentials(void *tam_handle, uint16_t length,
                              uint8_t *nonce_credentials)
{
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_mfg_login_signature(void *tam_handle, uint16_t length,
                            uint8_t *signature, uint32_t *session_id)
{
    *session_id = 1;
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_mfg_cliip_install(void *tam_handle, uint32_t mfg_session_id,
                          uint16_t length, uint8_t *cliip_data)
{
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_mfg_create_sudi_request(void *tam_handle, uint32_t session_id,
                                sudi_info_t *sudi_info,
                                uint8_t **sudi_request,
                                uint16_t *request_length)
{
    *sudi_request = NULL;
    *request_length = 0;
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_mfg_install_cert_and_chain(void *tam_handle, uint32_t session_id,
                                   uint16_t ca_len, uint8_t *ca_data,
                                   uint16_t leaf_len, uint8_t *leaf_data,
                                   uint8_t cert_type)
{
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_mfg_session_end(void *tam_handle, uint32_t session_id)
{
    return TAM_RC_OK;
}

void tam_lib_mfg_errinfo_clear(void) {}
void tam_lib_mfg_errinfo_print(void) {}

tam_lib_mfg_errinfo_t tam_lib_mfg_errinfo;

/*========================================================================
 * TAM eSPI stubs
 *========================================================================*/

tam_lib_status_t
tam_lib_espi_get_mfg_id(void *tam_handle, uint16_t *length, uint8_t *data)
{
    *length = 0;
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_espi_get_dev_id(void *tam_handle, uint16_t *length, uint8_t *data)
{
    *length = 0;
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_espi_read(void *tam_handle, uint32_t addr, uint16_t length,
                  uint8_t *data)
{
    memset(data, 0xFF, length);
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_espi_write(void *tam_handle, uint32_t addr, uint16_t length,
                   uint8_t *data)
{
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_espi_erase_64k(void *tam_handle, uint32_t sector)
{
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_espi_erase_4k(void *tam_handle, uint32_t sector)
{
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_espi_bitstream_upgrade(void *tam_handle, uint8_t upgrade_type,
                               uint16_t data_len, uint8_t *data,
                               uint32_t addr_offset)
{
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_espi_authenticate(void *tam_handle, uint8_t image_type,
                          uint8_t *fpga_status, uint8_t *fw_status)
{
    *fpga_status = TAM_RC_OK;
    *fw_status = TAM_RC_OK;
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_soft_reset(void *tam_handle, uint32_t param1, uint32_t param2)
{
    return TAM_RC_OK;
}

/*========================================================================
 * TAM Object Manager stubs
 *========================================================================*/

tam_lib_status_t
tam_lib_omgr_enable_object_naming(void *tam_handle, uint32_t session_id)
{
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_omgr_set_endianness(void *tam_handle, uint8_t endianness)
{
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_omgr_get_oid_from_name(void *tam_handle, uint32_t session_id,
                               const char *name, uint32_t *oid)
{
    *oid = 0;
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_omgr_get_name_from_oid(void *tam_handle, uint32_t session_id,
                               uint32_t oid, char *name)
{
    strcpy(name, "unknown");
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_omgr_set_oid_name(void *tam_handle, uint32_t session_id,
                          uint32_t oid, const char *name)
{
    return TAM_RC_OK;
}

tam_lib_status_t
tam_lib_omgr_delete_by_oid(void *tam_handle, uint32_t session_id,
                           uint32_t oid)
{
    return TAM_RC_OK;
}

/*========================================================================
 * Broadcom EPDM dispatch table stubs
 *========================================================================*/

__plp__dispatch__t__ plp_miura_dispatch;
__plp__sec__dispatch__t__ plp_miura_sec_dispatch;

/*========================================================================
 * PCI utility stubs (libpci)
 *========================================================================*/

/* Minimal pci_access stub */
struct pci_access {
    int dummy;
};

struct pci_access *pci_alloc(void)
{
    return (struct pci_access *)calloc(1, sizeof(struct pci_access));
}

void pci_init(struct pci_access *pacc) { (void)pacc; }
void pci_scan_bus(struct pci_access *pacc) { (void)pacc; }

int pci_fill_info(struct pci_access *pacc, int flags)
{
    (void)pacc; (void)flags;
    return 0;
}

int pci_read_byte(struct pci_access *pacc, int reg)
{
    (void)pacc; (void)reg;
    return 0;
}

void pci_cleanup(struct pci_access *pacc)
{
    free(pacc);
}

/*------------------------------------------------------------------
 * tam_lib_object_manager.h -- dummy stub for build dependency
 *
 * This is a placeholder to satisfy the build dependency.
 * The real header is part of the TAM Aikido SDK.
 *------------------------------------------------------------------
 */
#ifndef __TAM_LIB_OBJECT_MANAGER_H__
#define __TAM_LIB_OBJECT_MANAGER_H__

#include "tam_library.h"

/* Object manager (omgr) API functions */
extern tam_lib_status_t
tam_lib_omgr_enable_object_naming(void *tam_handle,
                                  uint32_t session_id);

extern tam_lib_status_t
tam_lib_omgr_set_endianness(void *tam_handle,
                            uint8_t endianness);

extern tam_lib_status_t
tam_lib_omgr_get_oid_from_name(void *tam_handle,
                               uint32_t session_id,
                               const char *name,
                               uint32_t *oid);

extern tam_lib_status_t
tam_lib_omgr_get_name_from_oid(void *tam_handle,
                               uint32_t session_id,
                               uint32_t oid,
                               char *name);

extern tam_lib_status_t
tam_lib_omgr_set_oid_name(void *tam_handle,
                          uint32_t session_id,
                          uint32_t oid,
                          const char *name);

extern tam_lib_status_t
tam_lib_omgr_delete_by_oid(void *tam_handle,
                           uint32_t session_id,
                           uint32_t oid);

#endif /* __TAM_LIB_OBJECT_MANAGER_H__ */

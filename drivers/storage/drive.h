#include "ahci.h"
#include <stdint.h>

typedef enum {
    DISK_NULL = 0,
    DISK_AHCI_DEV_NULL = 0,
    DISK_AHCI_DEV_SATA,
    DISK_AHCI_DEV_SEMB,
    DISK_AHCI_DEV_PM,
    DISK_AHCI_DEV_SATAPI,
} disk_type_t;

typedef struct {
    disk_type_t type;
    // If in future NVMe will be added, then
    // it won't depend on AHCI implementation.
    union {
        HBA_PORT *ahci_hba_port;
    } control;
} disk_entry_t;

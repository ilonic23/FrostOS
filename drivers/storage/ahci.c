#include "ahci.h"
#include <stdint.h>
#include "../pci.h"
#include "../../cpu/timer.h"

HBA_MEM *ahci_get_abar(uint8_t bus, uint8_t slot, uint8_t func) {
    pci_standard_device_header_t ahci_ctrl_header = pci_get_standard_device_header(bus,
            slot, func);
    if (ahci_ctrl_header.base_header.vendor == 0xFFFF) return 0;

    if (ahci_ctrl_header.base_header.class_code == 0x01 &&
        ahci_ctrl_header.base_header.subclass == 0x06 &&
        ahci_ctrl_header.base_header.progif == 0x01) {
        // Assuming your pci_write_word takes (bus, slot, func, offset, value)
        uint16_t cmd = pci_config_read_word(bus, slot, func, 0x04);
        cmd |= (1 << 1); // Enable Memory Space
        cmd |= (1 << 2); // Enable Bus Mastering
        pci_config_write_word(bus, slot, func, 0x04, cmd);
        HBA_MEM *abar = (HBA_MEM *)(uintptr_t)(ahci_ctrl_header.bar5 & ~0x0FFFU); // clear flags from BAR5
        return abar;
    }
    return 0;
}

// Performs BIOS/OS handoff if needed and enables AHCI mode
// Returns 1 on success, 0 on timeout exceeded
int ahci_init(HBA_MEM *abar) {
    int timeout_suc = 1;;
    if (abar->cap2 & 1) { // BIOS/OS Handoff supported
        timeout_suc = 0;
        // Set OS Owned Semaphore
        abar->bohc |= (1u << 1);

        // Wait for BIOS to clear BIOS Owned Semaphore bit
        for (int i = 1000; i > 0; --i) {
            if (!( (abar->bohc & 1) || (abar->bohc & (1u << 4) ))) {
                timeout_suc = 1;
                break;
            }
            sleep_ms(10);
        }
    }

    // Enable AHCI mode
    abar->ghc |= (1u << 31);

    // Full HBA reset
    abar->ghc |= 1;
    
    // Wait for the hardware to clear the reset bit
    timeout_suc = 0;
    for (int i = 1000; i > 0; --i) {
        if (!( abar->ghc & 1 )) {
            timeout_suc = 1;
            break;
        }
        sleep_ms(10);
    }

    // Re-enable AHCI mode after reset
    abar->ghc |= (1u << 31);
    return timeout_suc;
}

// Took from OSDEV

#define	SATA_SIG_ATA	0x00000101	// SATA drive
#define	SATA_SIG_ATAPI	0xEB140101	// SATAPI drive
#define	SATA_SIG_SEMB	0xC33C0101	// Enclosure management bridge
#define	SATA_SIG_PM	0x96690101	// Port multiplier

#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3

// Check device type and wake up if needed
static int check_type(HBA_MEM *abar, HBA_PORT *port, int port_num)
{
    uint32_t ssts = port->ssts;
    uint8_t ipm = (ssts >> 8) & 0x0F;
    uint8_t det = ssts & 0x0F;

    if (det != HBA_PORT_DET_PRESENT)
        return AHCI_DEV_NULL;
        
    switch (ipm) {
        case 1: // Device active
            break;
        case 2: // Partial - device is sleeping but wake up is fast
            if(!ahci_wakeup_port(abar, port)) return AHCI_DEV_WAKEUP_FAIL;
            break;
        case 6: // Slumber - device is sleeping, waking up is slower
            if(!ahci_wakeup_port(abar, port)) return AHCI_DEV_WAKEUP_FAIL;
            break;
        case 8: // DevSleep - COMRESET
            if(!ahci_comreset_port(port)) return AHCI_DEV_WAKEUP_FAIL;
            break;
        default:
            return AHCI_DEV_NULL;
    }

	switch (port->sig)
	{
        case SATA_SIG_ATAPI:
            return AHCI_DEV_SATAPI;
        case SATA_SIG_SEMB:
            return AHCI_DEV_SEMB;
        case SATA_SIG_PM:
            return AHCI_DEV_PM;
        default:
            return AHCI_DEV_SATA;
    }
}

int *ahci_probe_ports(HBA_MEM *abar)
{
    static int ports[32] = {0};
	// Search disk in implemented ports
	uint32_t pi = abar->pi;
	int i = 0;
	while (i<32)
	{
        // Port i implemented
		if (pi & 1)
		{
			int dt = check_type(abar, &abar->ports[i], i);
            ports[i] = dt;
		} else {
            ports[i] = AHCI_DEV_NULL;
        }

		pi >>= 1;
		i ++;
	}
    return ports;
}

// ----------------------------------------

// Returns 1 on success, 0 on timeout exceeded
int ahci_comreset_port(HBA_PORT *port) {
    // According to Intel's Serial ATA AHCI 1.3.1 Specification (p. 30)
    // we shall first set the DET (bits 0-3) value to 0x1 in the PxSCTL
    port->sctl = (port->sctl & ~0x0F) | 1;

    // Wait 1 millisecond, but 5 will make us sure.
    sleep_ms(5);

    // Clear COMRESET by writing 0 to DET
    port->sctl = (port->sctl & ~0x0F);

    // Wait for the port to re-establish PHY link (DET == 3)
    for (int i = 100; i > 0; --i) {
        if ((port->ssts & 0x0F) == HBA_PORT_DET_PRESENT) {
            // Clear ERR
            port->serr |= 0xFFFFFFFF;
            return 1;
        }
        sleep_ms(10);
    }
    return 0;
}

// Returns 1 on success, returns 0 on timeout exceeded
int ahci_wakeup_port(HBA_MEM *abar, HBA_PORT *port) {
    // Disable power-management state transitions
    // Bit 8 - partial and bit 9 - slumber when set to 1 disables them.
    port->sctl |= (3 << 8);

    // Force port state to active
    port->cmd &= ~(0x0FU << 28); // Clear ICC bits
    port->cmd |=  (1U << 28);    // Set ICC to 1 - active (p. 26)
    
    // Set POD and SUD (power-on-device, spin-up-device) to 1 if allowed
    if (port->cmd & (1u << 20)) {
        // If CPD is set, we can write to POD
        port->cmd |= (1u << 2);
    }
    if (abar->cap & (1u << 27)) {
        // If CAP.SSS is set, we can spin up drives
        port->cmd |= (1u << 1);
    }

    // Wait for wake up
    for (int i = 100; i > 0; --i) {
        uint8_t current_ipm = (port->ssts >> 8) & 0x0F;
        uint8_t current_det = port->ssts & 0x0F;

        if (current_det == HBA_PORT_DET_PRESENT && current_ipm == HBA_PORT_IPM_ACTIVE) {
            // Clear ERR
            port->serr |= 0xFFFFFFFF;
            return 1;
        }
        sleep_ms(10);
    }
    return 0;
}

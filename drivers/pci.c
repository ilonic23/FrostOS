#include <stdint.h>
#include "../cpu/ports.h"
#include "pci.h"

// Took from OSDEV: https://wiki.osdev.org/PCI#Configuration_Space_Access_Mechanism_#1
// offset is in bytes - just a note for me ;)
uint16_t pci_config_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint16_t tmp = 0;
  
    // Create configuration address as per Figure 1
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
  
    // Write out the address
    port_dword_out(0xCF8, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    tmp = (uint16_t)((port_dword_in(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
    return tmp;
}

uint32_t pci_config_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (1u << 31)
                     | ((uint32_t)bus  << 16)
                     | ((uint32_t)slot << 11)
                     | ((uint32_t)func <<  8)
                     | (offset & 0xFC);

    port_dword_out(0xCF8, address);
    return port_dword_in(0xCFC);
}

// 0xFFFF - invalid vendor - non-existent device
uint16_t pci_get_vendor(uint8_t bus, uint8_t slot) {
    return pci_config_read_word(bus, slot, 0, 0);
}

uint16_t pci_get_device(uint8_t bus, uint8_t slot) {
    uint32_t vendor_device = pci_config_read_dword(bus, slot, 0, 0);
    uint16_t vendor = (uint16_t)(vendor_device & 0xFFFF);
    if (vendor != 0xFFFF) {
       return (uint16_t)((vendor_device >> 16) & 0xFFFF);
    }
    return vendor;
}

// Doesn't check for invalid vendor!
uint8_t pci_get_class_code(uint8_t bus, uint8_t slot) {
    return (uint8_t)((pci_config_read_word(bus, slot, 0, 10) >> 8) & 0xFF);
}

// Doesn't check for invalid vendor!
uint8_t pci_get_subclass(uint8_t bus, uint8_t slot) {
    return (uint8_t)(pci_config_read_word(bus, slot, 0, 10) & 0xFF);
}

// Doesn't check for invalid vendor!
uint8_t pci_get_progif(uint8_t bus, uint8_t slot) {
    return (uint8_t)((pci_config_read_word(bus, slot, 0, 8) >> 8) & 0xFF);
}

// Doesn't check for invalid vendor!
uint8_t pci_get_revision(uint8_t bus, uint8_t slot) {
    return (uint8_t)(pci_config_read_word(bus, slot, 0, 8) & 0xFF);
}

pci_base_device_header_t pci_get_base_device_header(uint8_t bus, uint8_t slot, uint8_t func) {
    if (pci_get_vendor(bus, slot) == 0xFFFF)
        return (pci_base_device_header_t){.vendor = 0xFFFF};

    uint32_t result[4];
    result[0] = pci_config_read_dword(bus, slot, func, 0x0);
    result[1] = pci_config_read_dword(bus, slot, func, 0x4);
    result[2] = pci_config_read_dword(bus, slot, func, 0x8);
    result[3] = pci_config_read_dword(bus, slot, func, 0xC);

    return *(pci_base_device_header_t *)result;
}

pci_standard_device_header_t pci_get_standard_device_header(uint8_t bus, uint8_t slot, uint8_t func) {
    if (pci_get_vendor(bus, slot) == 0xFFFF)
        return (pci_standard_device_header_t){.base_header.vendor = 0xFFFF};

    uint32_t result[sizeof(pci_standard_device_header_t)/4];
    for (int i = 0; i < sizeof(pci_standard_device_header_t); i+=4)
        result[i/4] = pci_config_read_dword(bus, slot, func, i);

    return *(pci_standard_device_header_t *)result;
}

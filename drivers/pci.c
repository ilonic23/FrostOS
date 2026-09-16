#include <kor/dev/pci.h>
#include <kor/ll/port.h>
#include <kor/types.h>

// Took from OSDEV:
// https://wiki.osdev.org/PCI#Configuration_Space_Access_Mechanism_#1 offset is
// in bytes - just a note for me ;)
u16 pci_config_read_word(u8 bus, u8 slot, u8 func, u8 offset) {
    u32 address;
    u32 lbus = (u32)bus;
    u32 lslot = (u32)slot;
    u32 lfunc = (u32)func;
    u16 tmp = 0;

    // Create configuration address as per Figure 1
    address = (u32)((lbus << 16) | (lslot << 11) | (lfunc << 8) |
                    (offset & 0xFC) | ((u32)0x80000000));

    // Write out the address
    outl(0xCF8, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    tmp = (u16)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
    return tmp;
}

u32 pci_config_read_dword(u8 bus, u8 slot, u8 func, u8 offset) {
    u32 address = (1u << 31) | ((u32)bus << 16) | ((u32)slot << 11) |
                  ((u32)func << 8) | (offset & 0xFC);

    outl(0xCF8, address);
    return inl(0xCFC);
}

void pci_config_write_word(u8 bus, u8 slot, u8 func, u8 offset, u16 value) {
    u32 addr = (1u << 31) | ((u32)bus << 16) | ((u32)slot << 11) |
               ((u32)func << 8) | (offset & 0xFC);
    outl(0xCF8, addr);
    outl(0xCFC + (offset & 2), value);
}

void pci_config_write_dword(u8 bus, u8 slot, u8 func, u8 offset, u32 value) {
    u32 addr = (1u << 31) | ((u32)bus << 16) | ((u32)slot << 11) |
               ((u32)func << 8) | (offset & 0xFC);
    outl(0xCF8, addr);
    outl(0xCFC, value);
}

// 0xFFFF - invalid vendor - non-existent device
u16 pci_get_vendor(u8 bus, u8 slot) {
    return pci_config_read_word(bus, slot, 0, 0);
}

u16 pci_get_device(u8 bus, u8 slot) {
    u32 vendor_device = pci_config_read_dword(bus, slot, 0, 0);
    u16 vendor = (u16)(vendor_device & 0xFFFF);
    if (vendor != 0xFFFF) {
        return (u16)((vendor_device >> 16) & 0xFFFF);
    }
    return vendor;
}

// Doesn't check for invalid vendor!
u8 pci_get_class_code(u8 bus, u8 slot) {
    return (u8)((pci_config_read_word(bus, slot, 0, 10) >> 8) & 0xFF);
}

// Doesn't check for invalid vendor!
u8 pci_get_subclass(u8 bus, u8 slot) {
    return (u8)(pci_config_read_word(bus, slot, 0, 10) & 0xFF);
}

// Doesn't check for invalid vendor!
u8 pci_get_progif(u8 bus, u8 slot) {
    return (u8)((pci_config_read_word(bus, slot, 0, 8) >> 8) & 0xFF);
}

// Doesn't check for invalid vendor!
u8 pci_get_revision(u8 bus, u8 slot) {
    return (u8)(pci_config_read_word(bus, slot, 0, 8) & 0xFF);
}

pci_base_device_header_t pci_get_base_device_header(u8 bus, u8 slot, u8 func) {
    if (pci_get_vendor(bus, slot) == 0xFFFF)
        return (pci_base_device_header_t){.vendor = 0xFFFF};

    u32 result[4];
    result[0] = pci_config_read_dword(bus, slot, func, 0x0);
    result[1] = pci_config_read_dword(bus, slot, func, 0x4);
    result[2] = pci_config_read_dword(bus, slot, func, 0x8);
    result[3] = pci_config_read_dword(bus, slot, func, 0xC);

    return *(pci_base_device_header_t *)result;
}

pci_standard_device_header_t pci_get_standard_device_header(u8 bus, u8 slot,
                                                            u8 func) {
    if (pci_get_vendor(bus, slot) == 0xFFFF)
        return (pci_standard_device_header_t){.base_header.vendor = 0xFFFF};

    u32 result[sizeof(pci_standard_device_header_t) / 4];
    for (u32 i = 0; i < sizeof(pci_standard_device_header_t); i += 4)
        result[i / 4] = pci_config_read_dword(bus, slot, func, i);

    return *(pci_standard_device_header_t *)result;
}

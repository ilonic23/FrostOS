#ifndef KOR_DEV_PCI_H
#define KOR_DEV_PCI_H

#include <kor/types.h>

typedef struct __attribute__((packed)) {
    unsigned int io_space : 1;
    unsigned int memory_space : 1;
    unsigned int bus_master : 1;
    unsigned int special_cycles : 1;
    unsigned int mem_wr_inv_enable : 1; // memory write and invalidate enable
    unsigned int vga_palette_snoop : 1;
    unsigned int parity_err_response : 1;
    unsigned int res1 : 1; // reserved 1
    unsigned int serr_enable : 1;
    unsigned int fast_b2b_enable : 1; // fast back-to-back enable
    unsigned int interrupt_disable : 1;
    unsigned int res2 : 5; // reserved 2
} pci_command_register_t;

typedef struct __attribute__((packed)) {
    unsigned int res1 : 3;
    unsigned int interrupt_status : 1;
    unsigned int capabilities_list : 1;
    unsigned int mhz_capable : 1; // 66 MHz capable
    unsigned int res2 : 1;
    unsigned int fast_b2b_capable : 1;
    unsigned int master_data_parity_err : 1;
    unsigned int devsel_timing : 2;
    unsigned int signaled_target_abort : 1;
    unsigned int received_target_abort : 1;
    unsigned int received_master_abort : 1;
    unsigned int signaled_system_err : 1;
    unsigned int detected_parity_err : 1;
} pci_status_register_t;

typedef struct __attribute__((packed)) {
    u16 vendor;
    u16 device_id;
    pci_command_register_t command;
    pci_status_register_t status;
    u8 revision_id;
    u8 progif;
    u8 subclass;
    u8 class_code;
    u8 cache_line_size;
    u8 latency_timer;
    u8 header_type;
    u8 bist;
} pci_base_device_header_t;

typedef struct __attribute__((packed)) {
    pci_base_device_header_t base_header;
    u32 bar0;
    u32 bar1;
    u32 bar2;
    u32 bar3;
    u32 bar4;
    u32 bar5;
    u32 cardbus_cis_ptr;
    u16 subsystem_vendor_id;
    u16 subsystem_id;
    u32 expansion_rom_base_addr;
    u8 capabilities_ptr;
    u32 res1 : 24;
    u32 res2;
    u8 interrupt_line;
    u8 interrupt_pin;
    u8 min_grant;
    u8 max_latency;
} pci_standard_device_header_t;

typedef struct __attribute__((packed)) {
    pci_base_device_header_t base_header;
    u32 bar0;
    u32 bar1;
    u8 primary_bus_num;
    u8 secondary_bus_num;
    u8 subordinate_bus_num;
    u8 secondary_latency_timer;
    u8 io_base;
    u8 io_limit;
    u16 secondary_status;
    u16 memory_base;
    u16 memory_limit;
    u16 pref_memory_base;
    u16 pref_memory_limit;
    u32 pref_base_upper_32;
    u32 pref_limit_upper_32;
    u16 io_base_upper_16;
    u16 io_limit_upper_16;
    u8 capability_ptr;
    u32 res1 : 24;
    u32 expansion_rom_base_addr;
    u8 interrupt_line;
    u8 interrupt_pin;
    u16 bridge_control;
} pci_p2p_bridge_header_t;

typedef struct __attribute__((packed)) {
    pci_base_device_header_t base_header;
    u32 cardbus_socket;
    u8 offset_capabilities_list;
    u8 res1;
    u16 secondary_status;
    u8 pci_bus_num;
    u8 cardbus_bus_num;
    u8 subordinate_bus_num;
    u8 cardbus_latency_timer;
    u32 memory_base_addr0;
    u32 memory_limit0;
    u32 memory_base_addr1;
    u32 memory_limit1;
    u32 io_base_addr0;
    u32 io_limit0;
    u32 io_base_addr1;
    u32 io_limit1;
    u8 interrupt_line;
    u8 interrupt_pin;
    u16 bridge_control;
    u16 subsystem_device_id;
    u16 subsystem_vendor_id;
    u32 /*16*/ bit_pc_card_legacy_mode_base_addr;
} pci_pci2cardbus_bridge_header_t;

u16 pci_config_read_word(u8 bus, u8 slot, u8 func, u8 offset);
u32 pci_config_read_dword(u8 bus, u8 slot, u8 func, u8 offset);
u16 pci_get_vendor(u8 bus, u8 slot);
u16 pci_get_device(u8 bus, u8 slot);
u8 pci_get_class_code(u8 bus, u8 slot);
u8 pci_get_subclass(u8 bus, u8 slot);
u8 pci_get_progif(u8 bus, u8 slot);
u8 pci_get_revision(u8 bus, u8 slot);
pci_base_device_header_t pci_get_base_device_header(u8 bus, u8 slot, u8 func);
pci_standard_device_header_t pci_get_standard_device_header(u8 bus, u8 slot,
                                                            u8 func);

#endif // KOR_DEV_PCI_H

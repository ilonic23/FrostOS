#include <kor/types.h>
#include <kstring.h>

void cpuid_asm(u32 leaf, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx) {
    u32 a = leaf, b, c, d;

    // GCC/Clang inline assembly syntax (AT&T syntax by default)
    __asm__ __volatile__(
        "cpuid"
        : "=a"(a), "=b"(b), "=c"(c),
          "=d"(d)   // Output operands: C variables receive register values
        : "a"(leaf) // Input operand: EAX is set to 'leaf' value
        : "memory"  // Clobbered registers/memory
    );

    *eax = a;
    *ebx = b;
    *ecx = c;
    *edx = d;
}

void get_cpu_manufacturer(char *to) {
    u32 eax, ebx, edx, ecx;

    union {
        struct {
            u32 ebx;
            u32 edx;
            u32 ecx;
        } s;
        char str[13];
    } cpu = {0};

    cpuid_asm(0, &eax, &ebx, &ecx, &edx);

    // asm volatile(
    //     "cpuid"
    //     : "=b"(ebx), "=d"(edx), "=c"(ecx)
    //     : "a"(0)
    // );

    cpu.s.ebx = ebx;
    cpu.s.edx = edx;
    cpu.s.ecx = ecx;
    cpu.str[12] = 0;
    memcpy(to, cpu.str, 13);
}

u8 get_cpu_human_id(char *to) {
    u32 eax, ebx, ecx, edx;

    // Check the highest supported extended function
    cpuid_asm(0x80000000, &eax, &ebx, &ecx, &edx);
    unsigned int max_extended_leaf = eax;

    if (max_extended_leaf >= 0x80000004) {
        // Retrieve the brand string in three parts (leaves 0x80000002 to
        // 0x80000004)

        // Part 1
        cpuid_asm(0x80000002, (u32 *)(to + 0), (u32 *)(to + 4), (u32 *)(to + 8),
                  (u32 *)(to + 12));

        // Part 2
        cpuid_asm(0x80000003, (u32 *)(to + 16), (u32 *)(to + 20),
                  (u32 *)(to + 24), (u32 *)(to + 28));

        // Part 3
        cpuid_asm(0x80000004, (u32 *)(to + 32), (u32 *)(to + 36),
                  (u32 *)(to + 40), (u32 *)(to + 44));

        to[48] = '\0'; // Ensure null termination
        return 1;
    } else {
        return 0;
    }

    return 1;
}

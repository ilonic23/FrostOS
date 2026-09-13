#include <kor/ll/idt.h>
#include <kor/ll/isr.h>
#include <kor/ll/port.h>

#include <kor/kernel.h>

isr_t interrupt_handlers[256];

/* Can't do this with a loop because we need the address
 * of the function names */
void isr_install() {
    set_idt_gate(0, (usize)isr0);
    set_idt_gate(1, (usize)isr1);
    set_idt_gate(2, (usize)isr2);
    set_idt_gate(3, (usize)isr3);
    set_idt_gate(4, (usize)isr4);
    set_idt_gate(5, (usize)isr5);
    set_idt_gate(6, (usize)isr6);
    set_idt_gate(7, (usize)isr7);
    set_idt_gate(8, (usize)isr8);
    set_idt_gate(9, (usize)isr9);
    set_idt_gate(10, (usize)isr10);
    set_idt_gate(11, (usize)isr11);
    set_idt_gate(12, (usize)isr12);
    set_idt_gate(13, (usize)isr13);
    set_idt_gate(14, (usize)isr14);
    set_idt_gate(15, (usize)isr15);
    set_idt_gate(16, (usize)isr16);
    set_idt_gate(17, (usize)isr17);
    set_idt_gate(18, (usize)isr18);
    set_idt_gate(19, (usize)isr19);
    set_idt_gate(20, (usize)isr20);
    set_idt_gate(21, (usize)isr21);
    set_idt_gate(22, (usize)isr22);
    set_idt_gate(23, (usize)isr23);
    set_idt_gate(24, (usize)isr24);
    set_idt_gate(25, (usize)isr25);
    set_idt_gate(26, (usize)isr26);
    set_idt_gate(27, (usize)isr27);
    set_idt_gate(28, (usize)isr28);
    set_idt_gate(29, (usize)isr29);
    set_idt_gate(30, (usize)isr30);
    set_idt_gate(31, (usize)isr31);
    set_idt_gate(153, (usize)isr99);

    // Remap the PIC
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    // Install the IRQs
    set_idt_gate(32, (usize)irq0);
    set_idt_gate(33, (usize)irq1);
    set_idt_gate(34, (usize)irq2);
    set_idt_gate(35, (usize)irq3);
    set_idt_gate(36, (usize)irq4);
    set_idt_gate(37, (usize)irq5);
    set_idt_gate(38, (usize)irq6);
    set_idt_gate(39, (usize)irq7);
    set_idt_gate(40, (usize)irq8);
    set_idt_gate(41, (usize)irq9);
    set_idt_gate(42, (usize)irq10);
    set_idt_gate(43, (usize)irq11);
    set_idt_gate(44, (usize)irq12);
    set_idt_gate(45, (usize)irq13);
    set_idt_gate(46, (usize)irq14);
    set_idt_gate(47, (usize)irq15);

    set_idt(); // Load with ASM
}

/* To print the message which defines every exception */
char *exception_messages[] = {"Division By Zero",
                              "Debug",
                              "Non Maskable Interrupt",
                              "Breakpoint",
                              "Into Detected Overflow",
                              "Out of Bounds",
                              "Invalid Opcode",
                              "No Coprocessor", // 7

                              "Double Fault",
                              "Coprocessor Segment Overrun",
                              "Bad TSS",
                              "Segment Not Present",
                              "Stack Fault",
                              "General Protection Fault", // 13
                              "Page Fault",
                              "Unknown Interrupt",

                              "Coprocessor Fault",
                              "Alignment Check",
                              "Machine Check",
                              "Reserved",
                              "Reserved",
                              "Reserved",
                              "Reserved",
                              "Reserved",

                              "Reserved",
                              "Reserved",
                              "Reserved",
                              "Reserved",
                              "Reserved",
                              "Reserved",
                              "Reserved",
                              "Reserved"};

void isr_handler(registers_t *r) {
    if (r->int_no >= IRQ0 && r->int_no <= IRQ15)
        return;

    if (interrupt_handlers[r->int_no] != 0) {
        isr_t handler = interrupt_handlers[r->int_no];
        handler(r);
    } else
        kernel_panic(r->int_no, exception_messages[r->int_no], r);
}

void register_interrupt_handler(u8 n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

void irq_handler(registers_t *r) {
    /* After every interrupt we need to send an EOI to the PICs
     * or they will not send another interrupt again */
    if (r->int_no >= 40)
        outb(0xA0, 0x20); /* slave */
    outb(0x20, 0x20);     /* master */

    /* Handle the interrupt in a more modular way */
    if (interrupt_handlers[r->int_no] != 0) {
        isr_t handler = interrupt_handlers[r->int_no];
        handler(r);
    }
}

#include <kor/clock/pit.h>
#include <kor/clock/rtc.h>
#include <kor/input/ps2_keyboard.h>
#include <kor/input/ps2_mouse.h>

void irq_install() {
    asm volatile("sti");
    pit_init(1000); // IRQ0 - PIT
    kb_init();      // IRQ1 - keyboard
    rtc_init(0x06); // IRQ8 - rtc
    mouse_init();   // IRQ12 - mouse
    // install_isr99();  FIXME: Should be in isr_install...
}

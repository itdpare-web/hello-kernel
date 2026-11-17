/* Flat 32-bit GDT, IDT, and 8259 PIC remap. IRQ1 (keyboard) is vector 0x21. */

#include "io.h"
#include "interrupts.h"

struct gdt_entry {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char  base_mid;
    unsigned char  access;
    unsigned char  gran;
    unsigned char  base_high;
} __attribute__((packed));

struct gdt_ptr {
    unsigned short limit;
    unsigned int   base;
} __attribute__((packed));

struct idt_entry {
    unsigned short offset_low;
    unsigned short selector;
    unsigned char  zero;
    unsigned char  type_attr;
    unsigned short offset_high;
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;
    unsigned int   base;
} __attribute__((packed));

extern void gdt_flush(const struct gdt_ptr *ptr);
extern void idt_flush(const struct idt_ptr *ptr);
extern void irq1_stub(void);

static struct gdt_entry gdt[3];
static struct gdt_ptr   gp;
static struct idt_entry idt[256];
static struct idt_ptr   ip;

static void gdt_set(int i, unsigned int base, unsigned int limit,
                    unsigned char access, unsigned char flags)
{
    gdt[i].limit_low  = (unsigned short)(limit & 0xFFFF);
    gdt[i].base_low   = (unsigned short)(base & 0xFFFF);
    gdt[i].base_mid   = (unsigned char)((base >> 16) & 0xFF);
    gdt[i].access     = access;
    gdt[i].gran       = (unsigned char)(((limit >> 16) & 0x0F) | (flags & 0xF0));
    gdt[i].base_high  = (unsigned char)((base >> 24) & 0xFF);
}

void gdt_init(void)
{
    gdt_set(0, 0, 0, 0, 0);
    gdt_set(1, 0, 0xFFFFF, 0x9A, 0xC0); /* kernel code, RX, 32-bit, 4 KiB gran */
    gdt_set(2, 0, 0xFFFFF, 0x92, 0xC0); /* kernel data, RW, 32-bit, 4 KiB gran */

    gp.limit = (unsigned short)(sizeof(gdt) - 1);
    gp.base  = (unsigned int)gdt;
    gdt_flush(&gp);
}

static void pic_remap(void)
{
    /* ICW1: start init, expect ICW4 */
    outb(0x20, 0x11);
    io_wait();
    outb(0xA0, 0x11);
    io_wait();
    /* ICW2: vector offsets 0x20 / 0x28 */
    outb(0x21, 0x20);
    io_wait();
    outb(0xA1, 0x28);
    io_wait();
    /* ICW3: master has a slave on IRQ2, slave identity 2 */
    outb(0x21, 0x04);
    io_wait();
    outb(0xA1, 0x02);
    io_wait();
    /* ICW4: 8086 mode */
    outb(0x21, 0x01);
    io_wait();
    outb(0xA1, 0x01);
    io_wait();

    /* Unmask only IRQ1 (keyboard) on the master; mask the slave entirely. */
    outb(0x21, 0xFD);
    outb(0xA1, 0xFF);
}

void idt_init(void)
{
    unsigned int handler = (unsigned int)irq1_stub;
    int i;

    for (i = 0; i < 256; i++) {
        idt[i].offset_low  = 0;
        idt[i].selector    = 0;
        idt[i].zero        = 0;
        idt[i].type_attr   = 0;
        idt[i].offset_high = 0;
    }

    pic_remap();

    idt[0x21].offset_low  = (unsigned short)(handler & 0xFFFF);
    idt[0x21].selector    = 0x08;  /* kernel code segment */
    idt[0x21].zero        = 0;
    idt[0x21].type_attr   = 0x8E;  /* present, ring 0, 32-bit interrupt gate */
    idt[0x21].offset_high = (unsigned short)((handler >> 16) & 0xFFFF);

    ip.limit = (unsigned short)(sizeof(idt) - 1);
    ip.base  = (unsigned int)idt;
    idt_flush(&ip);
}

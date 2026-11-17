/* Multiboot v1 header, 32-bit entry, and a few CPU helpers. */

.set ALIGN,    1 << 0
.set MEMINFO,  1 << 1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .bss
.align 16
stack_bottom:
    .skip 16384
stack_top:

.section .text
.global _start
.type _start, @function
_start:
    movl $stack_top, %esp
    call kmain
    cli
.hang:
    hlt
    jmp .hang
.size _start, . - _start

/* void gdt_flush(const struct gdt_ptr *ptr) */
.global gdt_flush
.type gdt_flush, @function
gdt_flush:
    movl 4(%esp), %eax
    lgdt (%eax)
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    ljmp $0x08, $reload_cs
reload_cs:
    ret

/* void idt_flush(const struct idt_ptr *ptr) */
.global idt_flush
.type idt_flush, @function
idt_flush:
    movl 4(%esp), %eax
    lidt (%eax)
    ret

/* IRQ1 (keyboard, vector 0x21) stub: save regs, call C, iret. */
.global irq1_stub
.type irq1_stub, @function
irq1_stub:
    pusha
    cld
    call keyboard_irq
    popa
    iret

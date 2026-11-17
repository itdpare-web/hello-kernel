/* Kernel entry: print a greeting, then echo the PS/2 keyboard on IRQ1. */

#include "console.h"
#include "interrupts.h"
#include "keyboard.h"

void kmain(void)
{
    console_init();
    gdt_init();
    idt_init();
    keyboard_init();

    console_write("Hello Kernel\n");
    console_write("> ");
    console_mark_prompt();

    __asm__ volatile ("sti");
    for (;;)
        __asm__ volatile ("hlt");
}

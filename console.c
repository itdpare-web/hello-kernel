/* VGA text mode (0xB8000) + COM1 serial console, with a hardware cursor. */

#include "io.h"
#include "console.h"

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_BUFFER ((volatile unsigned short *)0xB8000)
#define VGA_COLOR  0x0F
#define COM1       0x3F8
#define CRTC_INDEX 0x3D4
#define CRTC_DATA  0x3D5

static int row;
static int col;
static int min_pos; /* linear index; backspace will not go before this */

static void serial_init(void)
{
    outb(COM1 + 1, 0x00); /* disable interrupts */
    outb(COM1 + 3, 0x80); /* enable DLAB */
    outb(COM1 + 0, 0x03); /* divisor low  (38400 baud) */
    outb(COM1 + 1, 0x00); /* divisor high */
    outb(COM1 + 3, 0x03); /* 8 bits, no parity, 1 stop bit */
    outb(COM1 + 2, 0xC7); /* enable FIFO */
    outb(COM1 + 4, 0x0B); /* IRQs enabled, RTS/DSR set */
}

static void serial_putchar(char c)
{
    while ((inb(COM1 + 5) & 0x20) == 0)
        ;
    outb(COM1, (unsigned char)c);
}

static void update_hw_cursor(void)
{
    unsigned short pos = (unsigned short)(row * VGA_WIDTH + col);

    outb(CRTC_INDEX, 0x0E);
    outb(CRTC_DATA, (unsigned char)(pos >> 8));
    outb(CRTC_INDEX, 0x0F);
    outb(CRTC_DATA, (unsigned char)(pos & 0xFF));
}

static void scroll(void)
{
    volatile unsigned short *vga = VGA_BUFFER;
    unsigned short blank = ((unsigned short)VGA_COLOR << 8) | ' ';
    int i;

    for (i = 0; i < VGA_WIDTH * (VGA_HEIGHT - 1); i++)
        vga[i] = vga[i + VGA_WIDTH];
    for (i = VGA_WIDTH * (VGA_HEIGHT - 1); i < VGA_WIDTH * VGA_HEIGHT; i++)
        vga[i] = blank;

    if (row > 0)
        row--;
    min_pos -= VGA_WIDTH;
    if (min_pos < 0)
        min_pos = 0;
}

static void put_vga(char c)
{
    volatile unsigned short *vga = VGA_BUFFER;

    if (c == '\n') {
        col = 0;
        row++;
    } else if (c == '\r') {
        col = 0;
    } else if (c == '\b') {
        int pos = row * VGA_WIDTH + col;
        if (pos > min_pos) {
            if (col > 0) {
                col--;
            } else if (row > 0) {
                row--;
                col = VGA_WIDTH - 1;
            }
            vga[row * VGA_WIDTH + col] = ((unsigned short)VGA_COLOR << 8) | ' ';
        }
    } else {
        vga[row * VGA_WIDTH + col] = ((unsigned short)VGA_COLOR << 8) | (unsigned char)c;
        col++;
        if (col >= VGA_WIDTH) {
            col = 0;
            row++;
        }
    }

    if (row >= VGA_HEIGHT)
        scroll();
    update_hw_cursor();
}

void console_init(void)
{
    volatile unsigned short *vga = VGA_BUFFER;
    unsigned short blank = ((unsigned short)VGA_COLOR << 8) | ' ';
    int i;

    for (i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        vga[i] = blank;

    row = 0;
    col = 0;
    min_pos = 0;
    update_hw_cursor();
    serial_init();
}

void console_mark_prompt(void)
{
    min_pos = row * VGA_WIDTH + col;
}

void console_putc(char c)
{
    if (c == '\b') {
        int pos = row * VGA_WIDTH + col;
        if (pos <= min_pos)
            return;
        put_vga('\b');
        serial_putchar('\b');
        serial_putchar(' ');
        serial_putchar('\b');
        return;
    }

    put_vga(c);
    if (c == '\n')
        serial_putchar('\r');
    serial_putchar(c);
}

void console_write(const char *s)
{
    while (*s)
        console_putc(*s++);
}

/* PS/2 keyboard (scancode set 1) on IRQ1: translate, echo, simple line editing. */

#include "io.h"
#include "console.h"
#include "keyboard.h"

#define KBD_DATA    0x60
#define KBD_STATUS  0x64
#define KBD_RELEASE 0x80
#define SC_EXTEND   0xE0
#define SC_LSHIFT   0x2A
#define SC_RSHIFT   0x36
#define PIC1_CMD    0x20
#define PIC_EOI     0x20

/* Index is the make code. 0 means “not a printable / editing key”. */
static const char keymap[2][0x40] = {
    {
        /* unshifted, scancode set 1 */
        0,    0,    '1',  '2',  '3',  '4',  '5',  '6',
        '7',  '8',  '9',  '0',  '-',  '=',  '\b', 0,
        'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',
        'o',  'p',  '[',  ']',  '\n', 0,    'a',  's',
        'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',
        '\'', '`',  0,    '\\', 'z',  'x',  'c',  'v',
        'b',  'n',  'm',  ',',  '.',  '/',  0,    0,
        0,    ' '
    },
    {
        /* shifted */
        0,    0,    '!',  '@',  '#',  '$',  '%',  '^',
        '&',  '*',  '(',  ')',  '_',  '+',  '\b', 0,
        'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',
        'O',  'P',  '{',  '}',  '\n', 0,    'A',  'S',
        'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',
        '"',  '~',  0,    '|',  'Z',  'X',  'C',  'V',
        'B',  'N',  'M',  '<',  '>',  '?',  0,    0,
        0,    ' '
    }
};

static int shift_down;
static int extended;

void keyboard_init(void)
{
    int i;

    /* Drain any pending bytes so we start from a clean controller state. */
    for (i = 0; i < 256 && (inb(KBD_STATUS) & 1); i++)
        (void)inb(KBD_DATA);

    shift_down = 0;
    extended = 0;
}

void keyboard_irq(void)
{
    unsigned char sc = inb(KBD_DATA);
    unsigned char code;
    int released;
    char ch;

    if (sc == SC_EXTEND) {
        extended = 1;
        goto eoi;
    }

    released = (sc & KBD_RELEASE) != 0;
    code = (unsigned char)(sc & 0x7F);

    if (code == SC_LSHIFT || code == SC_RSHIFT) {
        if (released)
            shift_down = 0;
        else
            shift_down = 1;
        extended = 0;
        goto eoi;
    }

    if (extended || released) {
        extended = 0;
        goto eoi;
    }

    if (code < (unsigned char)(sizeof(keymap[0]) / sizeof(keymap[0][0]))) {
        ch = keymap[shift_down ? 1 : 0][code];
        if (ch != 0) {
            console_putc(ch);
            if (ch == '\n') {
                console_write("> ");
                console_mark_prompt();
            }
        }
    }

eoi:
    outb(PIC1_CMD, PIC_EOI);
}

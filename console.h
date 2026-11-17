#ifndef CONSOLE_H
#define CONSOLE_H

void console_init(void);
void console_putc(char c);
void console_write(const char *s);
/* Disallow backspace from erasing anything before the current cursor. */
void console_mark_prompt(void);

#endif

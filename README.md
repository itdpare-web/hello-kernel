# Hello Kernel

一个最小的 32 位 x86 Multiboot 内核：启动后在 VGA 文本模式和 COM1 串口打印 `Hello Kernel` 和提示符 `> `，然后通过中断读取 PS/2 键盘，并把输入回显到屏幕和串口。

键盘采用中断驱动：加载平坦的 32 位 GDT、重映射 8259 PIC、安装 IDT 中的 IRQ1（向量 `0x21`）处理函数，随后 `sti` 并在 `hlt` 循环中等待按键。扫描码使用 XT / scancode set 1，轮询方案未使用。

## 文件说明

| 文件 | 作用 |
| --- | --- |
| `boot.s` | Multiboot v1 头、设置栈、调用 `kmain`；以及 `gdt_flush` / `idt_flush` / `irq1_stub` |
| `kmain.c` | 初始化控制台、GDT、IDT、键盘，打印问候和提示符，然后 `sti; hlt` |
| `console.c` / `console.h` | VGA 显存 `0xB8000` 输出、硬件光标（端口 `0x3D4` / `0x3D5`）、换行、退格、折行与滚屏，并同步写到 COM1（`0x3F8`） |
| `interrupts.c` / `interrupts.h` | 平坦 GDT、IDT、PIC 重映射；只放开 IRQ1 |
| `keyboard.c` / `keyboard.h` | IRQ1 处理：从端口 `0x60` 读取 scancode set 1，Shift 大小写/符号，回车与退格 |
| `io.h` | `inb` / `outb` |
| `linker.ld` | 将内核加载到 1 MiB，并把 Multiboot 头放在映像最前面 |
| `Makefile` | 编译、链接与运行命令 |
| `kernel.bin` | 构建得到的 Multiboot ELF 内核 |

## 依赖

需要 32 位汇编/链接工具，以及 QEMU：

```bash
sudo apt-get install gcc-multilib qemu-system-x86
```

本项目用 GNU `as`（`--32`）和 `gcc -m32 -ffreestanding -nostdlib`，不依赖 nasm 或 libc。

## 构建

```bash
make
```

会生成 `kernel.bin`。

## 运行

无图形、串口输出到终端（推荐，便于确认打印内容）：

```bash
qemu-system-i386 -kernel kernel.bin -serial stdio -display none
```

或：

```bash
make run
```

也可以带窗口查看 VGA 画面并直接打字：

```bash
qemu-system-i386 -kernel kernel.bin -serial stdio
```

启动后应看到：

```
Hello Kernel
> _
```

在提示符后输入会回显到屏幕和串口。Enter 换行并再次打印 `> `，Backspace 删除一个字符（不会删掉提示符），行尾自动折行，写满一屏后向上滚屏。按住 Shift 可输入大写字母和 `!@#$` 等符号。

内核会在 `hlt` 中等待下一次键盘中断，用 `Ctrl-A X`（或直接结束 QEMU 进程）退出。

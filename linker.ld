ENTRY(_start)

PHDRS
{
    text PT_LOAD FLAGS(5); /* RX */
    data PT_LOAD FLAGS(6); /* RW */
}

SECTIONS
{
    /* Load the kernel at 1 MiB, the conventional Multiboot address. */
    . = 1M;

    .text : {
        /* Multiboot header must appear in the first 8 KiB of the image. */
        *(.multiboot)
        *(.text)
    } :text

    .rodata : {
        *(.rodata .rodata.*)
    } :text

    .data : {
        *(.data)
    } :data

    .bss : {
        *(COMMON)
        *(.bss)
    } :data
}

ENTRY(Reset_handler)
/* _estack = 0x20002000; */
MEMORY
{
  ram (rwx) : ORIGIN = 0x20000000, LENGTH = 8K
  rom (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
}
SECTIONS
{
    .  = 0x08000000;         /* From 0x08000000 */
    .text :
    {
		*(.isr_vector)    /* Vector table */
		*(.init)
		*(.adv_main)   /*前置main方法，防止引导程序跳main的时候超出范围*/
        *(.text)      /* Program code */
		*(.text*)
        *(.rodata)    /* Read only data */
		*(.rodata*)
		. = ALIGN(4);
    } >rom
    /*要写入的数据的段描述，每页2K */
    .data_flash :
    {
		. = ALIGN(0x400);
		*(.data_flash)
    } >rom

    .  = 0x20000000;  /* From 0x20000000 */
    .data :
    {
        *(.data)      /* Data memory */
    } >ram AT > rom

    .bss :
    {
        *(.bss)       /* Zero-filled run time allocate data memory */
		*(.bss*)
    } >ram AT > rom
	.stack   0x20002000 :
	{
		_estack = .;
		*(.stack)
	}
}

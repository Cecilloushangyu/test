MEMORY
{
  RAM (rw) : ORIGIN = 0x10200000, LENGTH = 0x00600000
}

ENTRY(Reset_Handler)

SECTIONS
{
    .text :
    {
        Image$$VECTORS$$Base = .;
        * (RESET)
        KEEP(*(.isr_vector))
        Image$$VECTORS$$Limit = .;

        *(SVC_TABLE)
        Image$$EXEC$$RO$$Base = .;
        __text_start__ = .;
        *(.text*)

        KEEP(*(.init))
        KEEP(*(.fini))


        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)


        *crtbegin.o(.dtors)
        *crtbegin?.o(.dtors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
        *(SORT(.dtors.*))
        *(.dtors)
        
        Image$$RO_DATA$$Base = .;
        *(.rodata*)
        Image$$RO_DATA$$Limit = .;
        Image$$EXEC$$RO$$Limit = .;
        __text_end__ = . ;
        KEEP(*(.eh_frame*))
    } > RAM

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > RAM

    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > RAM
    __exidx_end = .;

    .data :
    {    	
    	Image$$EXEC$$RW$$Base = .;
        __data_start__ = .;        
		*(vtable)
        *(.data*)
        Image$$RW_DATA$$Limit = .;
        
        . = ALIGN(8);
        
        PROVIDE (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE (__preinit_array_end = .);

        . = ALIGN(8);

        PROVIDE (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE (__init_array_end = .);


        . = ALIGN(8);

        PROVIDE (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE (__fini_array_end = .);

        . = ALIGN(8);
		Image$$EXEC$$RW$$Limit = . ;
        __data_end__ = .;

    } > RAM


    .bss ALIGN(0x400) (NOLOAD):
    {
    	Image$$EXEC$$ZI$$Base = .;
        Image$$ZI_DATA$$Base = .;
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        __bss_end__ = .;
        Image$$ZI_DATA$$Limit = .;
        Image$$EXEC$$ZI$$Limit = .;
    } > RAM

    .heap (NOLOAD):
    {
        . = ALIGN(8);
        __HeapStart__ = .;
        Image$$HEAP$$ZI$$Base = .;
        . = ORIGIN(RAM) + LENGTH(RAM) - 0x00001000 - 0x00000100 - 0x00001000 - 0x00000100 - 0x00000100 - 0x00000100;
        Image$$HEAP$$ZI$$Limit = .;
        __HeapLimit__ = .;
        __end__ = .;
        end = __end__;
    } > RAM


    .stack (NOLOAD):
    {

        __StackTop = .;
        Image$$SYS_STACK$$ZI$$Base = .;
        . += 0x00001000;
        Image$$SYS_STACK$$ZI$$Limit = .;
        __stack = .;

        Image$$FIQ_STACK$$ZI$$Base = .;
        . += 0x00000100;
        Image$$FIQ_STACK$$ZI$$Limit = .;

        Image$$IRQ_STACK$$ZI$$Base = .;
        . += 0x00001000;
        Image$$IRQ_STACK$$ZI$$Limit = .;

        Image$$SVC_STACK$$ZI$$Base = .;
        . += 0x00000100;
        Image$$SVC_STACK$$ZI$$Limit = .;

        Image$$ABT_STACK$$ZI$$Base = .;
        . += 0x00000100;
        Image$$ABT_STACK$$ZI$$Limit = .;

        Image$$UND_STACK$$ZI$$Base = .;
        . += 0x00000100;
        Image$$UND_STACK$$ZI$$Limit = .;

    } > RAM
}

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
/* end of RAM */
_estack = 0x20080000;
/* Generate a link error if heap and stack don't fit into RAM */
/* required amount of heap  */
_Min_Heap_Size = 0x800;
/* required amount of stack */
_Min_Stack_Size = 0x1000;

/* Specify the memory areas */
MEMORY
{
  FLASH (rx)       : ORIGIN = 0x8000000,  LENGTH = 2048K
  RAM (xrw)        : ORIGIN = 0x20000000, LENGTH = 499K
  Memory_B1(xrw)   : ORIGIN = 0x2007C000, LENGTH = 0x80
  Memory_B2(xrw)   : ORIGIN = 0x2007C080, LENGTH = 0x80
  Memory_B3(xrw)   : ORIGIN = 0x2007C100, LENGTH = 0x17d0 
  Memory_B4(xrw)   : ORIGIN = 0x2007D8D0, LENGTH = 0x17d0
}

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    /* .text sections (code) */
    *(.text)
    /* .text* sections (code) */
    *(.text*)
    /* glue arm to thumb code */
    *(.glue_7)
    /* glue thumb to arm code */
    *(.glue_7t)
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    /* define a global symbols at end of code */
    _etext = .;
  } >FLASH

  /* Constant data goes into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    /* .rodata sections (constants, strings, etc.) */
    *(.rodata)
    /* .rodata* sections (constants, strings, etc.) */
    *(.rodata*)
    . = ALIGN(4);
  } >FLASH

  .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data : 
  {
    . = ALIGN(4);
    /* create a global symbol at data start */
    _sdata = .;
    /* .data sections */
    *(.data)
    /* .data* sections */
    *(.data*)

    . = ALIGN(4);
    /* define a global symbol at data end */
    _edata = .;
  } >RAM AT> FLASH

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;
    /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;
    /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM

  

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
  .tstRxDMADescSection (NOLOAD) : { *(.RxDescSection)   } >Memory_B1
  .tstTxDMADescSection (NOLOAD) : { *(.TxDescSection)   } >Memory_B2
  .tu08RxBufferSection (NOLOAD) : { *(.RxBufferSection) } >Memory_B3
  .tu08TxBufferSection (NOLOAD) : { *(.TxBufferSection) } >Memory_B4
}

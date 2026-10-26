/* Default linker script, for normal executables */
OUTPUT_FORMAT("elf32-spu", "elf32-spu",
	      "elf32-spu")
OUTPUT_ARCH(spu)
ENTRY(entry)
SEARCH_DIR("=/usr/local/lib"); SEARCH_DIR("=/lib"); SEARCH_DIR("=/usr/lib");
/* Do we need any of these for elf?
   __DYNAMIC = 0;    */
SECTIONS
{
  /* Read-only sections, merged into text segment: */
  PROVIDE (__executable_start = 0x0004000); . = 0x0004000;
  .interp         : { *(.interp) }
  .hash           : { *(.hash) }
  .dynsym         : { *(.dynsym) }
  .dynstr         : { *(.dynstr) }
  .gnu.version    : { *(.gnu.version) }
  .gnu.version_d  : { *(.gnu.version_d) }
  .gnu.version_r  : { *(.gnu.version_r) }
  .rel.init       : { *(.rel.init) }
  .rela.init      : { *(.rela.init) }
  .rel.text       : { *(.rel.text .rel.text.* .rel.gnu.linkonce.t.*) }
  .rela.text      : { *(.rela.text .rela.text.* .rela.gnu.linkonce.t.*) }
  .rel.fini       : { *(.rel.fini) }
  .rela.fini      : { *(.rela.fini) }
  .rel.rodata     : { *(.rel.rodata .rel.rodata.* .rel.gnu.linkonce.r.*) }
  .rela.rodata    : { *(.rela.rodata .rela.rodata.* .rela.gnu.linkonce.r.*) }
  .rel.data.rel.ro   : { *(.rel.data.rel.ro*) }
  .rela.data.rel.ro   : { *(.rel.data.rel.ro*) }
  .rel.data       : { *(.rel.data .rel.data.* .rel.gnu.linkonce.d.*) }
  .rela.data      : { *(.rela.data .rela.data.* .rela.gnu.linkonce.d.*) }
  .rel.tdata	  : { *(.rel.tdata .rel.tdata.* .rel.gnu.linkonce.td.*) }
  .rela.tdata	  : { *(.rela.tdata .rela.tdata.* .rela.gnu.linkonce.td.*) }
  .rel.tbss	  : { *(.rel.tbss .rel.tbss.* .rel.gnu.linkonce.tb.*) }
  .rela.tbss	  : { *(.rela.tbss .rela.tbss.* .rela.gnu.linkonce.tb.*) }
  .rel.ctors      : { *(.rel.ctors) }
  .rela.ctors     : { *(.rela.ctors) }
  .rel.dtors      : { *(.rel.dtors) }
  .rela.dtors     : { *(.rela.dtors) }
  .rel.got        : { *(.rel.got) }
  .rela.got       : { *(.rela.got) }
  .rel.bss        : { *(.rel.bss .rel.bss.* .rel.gnu.linkonce.b.*) }
  .rela.bss       : { *(.rela.bss .rela.bss.* .rela.gnu.linkonce.b.*) }
  .rel.plt        : { *(.rel.plt) }
  .rela.plt       : { *(.rela.plt) }

  .init           :
  {
    KEEP (*(.init))
  } =0
  .plt            : { *(.plt) }
  .SpuGUID   :
  {
	_start = . ;
		__SPU_GUID = .;
		LONG(0x40200000) /* nop */
		LONG(0x00200000) /* lnop */
		LONG(0x40200000) /* nop */
		LONG(0x00200000) /* lnop */
	}
	
	.Header :
	{
		LONG(0x32000000 + (entry - .)*0x20 ) /* br 16 - jump to 'entry' function */
		LONG(LOADADDR(PpuData)-_start) /* code+data size - bss */
		LONG(0)
		LONG(0)
		LONG(0)
	} =0
	
	/* include overlay code - this has to happen before .text segment or it won't work. include up to 5 overlays */
ov_vma_start = . ;
ov_lma_start = 0x40000;
vma_start_CodeOverlay1 = (ov_vma_start + 127) & ~(127);
lma_start_CodeOverlay1 = (ov_lma_start + 127) & ~(127);
CodeOverlay1 vma_start_CodeOverlay1 : AT ( lma_start_CodeOverlay1 ) { *OVL1*.o ( .text .rodata ) } 
vma_stop_CodeOverlay1 = (vma_start_CodeOverlay1 + SIZEOF( CodeOverlay1 ) + 127) & ~(127);
lma_stop_CodeOverlay1 = (lma_start_CodeOverlay1 + SIZEOF( CodeOverlay1 ) + 127) & ~(127);

vma_start_CodeOverlay2 = (ov_vma_start + 127) & ~(127);
lma_start_CodeOverlay2 = (lma_stop_CodeOverlay1 + 127) & ~(127);
CodeOverlay2 vma_start_CodeOverlay2 : AT ( lma_start_CodeOverlay2 ) { *OVL2*.o ( .text .rodata ) } 
vma_stop_CodeOverlay2 = (vma_start_CodeOverlay2 + SIZEOF( CodeOverlay2 ) + 127) & ~(127);
lma_stop_CodeOverlay2 = (lma_start_CodeOverlay2 + SIZEOF( CodeOverlay2 ) + 127) & ~(127);

vma_start_CodeOverlay3 = (ov_vma_start + 127) & ~(127);
lma_start_CodeOverlay3 = (lma_stop_CodeOverlay2 + 127) & ~(127);
CodeOverlay3 vma_start_CodeOverlay3 : AT ( lma_start_CodeOverlay3 ) { *OVL3*.o ( .text .rodata ) } 
vma_stop_CodeOverlay3 = (vma_start_CodeOverlay3 + SIZEOF( CodeOverlay3 ) + 127) & ~(127);
lma_stop_CodeOverlay3 = (lma_start_CodeOverlay3 + SIZEOF( CodeOverlay3 ) + 127) & ~(127);

vma_start_CodeOverlay4 = (ov_vma_start + 127) & ~(127);
lma_start_CodeOverlay4 = (lma_stop_CodeOverlay3 + 127) & ~(127);
CodeOverlay4 vma_start_CodeOverlay4 : AT ( lma_start_CodeOverlay4 ) { *OVL4*.o ( .text .rodata ) } 
vma_stop_CodeOverlay4 = (vma_start_CodeOverlay4 + SIZEOF( CodeOverlay4 ) + 127) & ~(127);
lma_stop_CodeOverlay4 = (lma_start_CodeOverlay4 + SIZEOF( CodeOverlay4 ) + 127) & ~(127);

vma_start_CodeOverlay5 = (ov_vma_start + 127) & ~(127);
lma_start_CodeOverlay5 = (lma_stop_CodeOverlay4 + 127) & ~(127);
CodeOverlay5 vma_start_CodeOverlay5 : AT ( lma_start_CodeOverlay5 ) { *OVL5*.o ( .text .rodata ) } 
vma_stop_CodeOverlay5 = (vma_start_CodeOverlay5 + SIZEOF( CodeOverlay5 ) + 127) & ~(127);
lma_stop_CodeOverlay5 = (lma_start_CodeOverlay5 + SIZEOF( CodeOverlay5 ) + 127) & ~(127);

. = MAX( MAX( MAX( MAX( MAX( ov_vma_start , vma_stop_CodeOverlay1 ) , vma_stop_CodeOverlay2 ) , vma_stop_CodeOverlay3 ) , vma_stop_CodeOverlay4 ) , vma_stop_CodeOverlay5 );

   .text :
   {
    *(.text .stub .text.* .gnu.linkonce.t.*)
    KEEP (*(.text.*personality*))
    
    *(.spu.elf)
    /* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
  } =0
  .fini           :
  {
    KEEP (*(.fini))
  } =0
  
  PROVIDE (__etext = .);
  PROVIDE (_etext = .);
  PROVIDE (etext = .);
  .rodata         : { *(.rodata .rodata.* .gnu.linkonce.r.*) }
  .rodata1        : { *(.rodata1) }
  .eh_frame_hdr : { *(.eh_frame_hdr) }
  .eh_frame       : ONLY_IF_RO { KEEP (*(.eh_frame)) }
  .gcc_except_table   : ONLY_IF_RO { KEEP (*(.gcc_except_table)) *(.gcc_except_table.*) }
  /* Adjust the address for the data segment.  We want to adjust up to
     the same address within the page on the next page up.  */
  . = ALIGN(0x80);
  /* Exception handling  */
  .eh_frame       : ONLY_IF_RW { KEEP (*(.eh_frame)) }
  .gcc_except_table   : ONLY_IF_RW { KEEP (*(.gcc_except_table)) *(.gcc_except_table.*) }
  /* Thread Local Storage sections  */
  .tdata	  : { *(.tdata .tdata.* .gnu.linkonce.td.*) }
  .tbss		  : { *(.tbss .tbss.* .gnu.linkonce.tb.*) *(.tcommon) }
  /* Ensure the __preinit_array_start label is properly aligned.  We
     could instead move the label definition inside the section, but
     the linker would then create the section even if it turns out to
     be empty, which isn't pretty.  */
  . = ALIGN(16);
  PROVIDE (__preinit_array_start = .);
  .preinit_array     : { KEEP (*(.preinit_array)) }
  PROVIDE (__preinit_array_end = .);
  PROVIDE (__init_array_start = .);
  .init_array     : { KEEP (*(.init_array)) }
  PROVIDE (__init_array_end = .);
  PROVIDE (__fini_array_start = .);
  .fini_array     : { KEEP (*(.fini_array)) }
  PROVIDE (__fini_array_end = .);
  .ctors          :
  {
    __CTOR_LIST__ = .;
    LONG(-1);
    /* gcc uses crtbegin.o to find the start of
       the constructors, so we make sure it is
       first.  Because this is a wildcard, it
       doesn't matter if the user does not
       actually link against crtbegin.o; the
       linker won't look for a file to match a
       wildcard.  The wildcard also means that it
       doesn't matter which directory crtbegin.o
       is in.  */
    KEEP (*crtbegin*.o(.ctors))
    /* We don't want to include the .ctor section from
       from the crtend.o file until after the sorted ctors.
       The .ctor section from the crtend file contains the
       end of ctors marker and it must be last */
    KEEP (*(EXCLUDE_FILE (*crtend*.o ) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    LONG(0);
  }
  .dtors          :
  {
    __DTOR_LIST__ = .;
    LONG(-1);
    KEEP (*crtbegin*.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend*.o ) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    LONG(0);
  }
  .jcr            : { KEEP (*(.jcr)) }
  .data.rel.ro : { *(.data.rel.ro.local) *(.data.rel.ro*) }
  .dynamic        : { *(.dynamic) }
  .got            : { *(.got.plt) *(.got) }
  .data           :
  {
	/* insert overlay table into SPU data segment */
. = ALIGN(0x10);	_ovly_table = . ;

. = ALIGN(0x10);	__ovly_info_CodeOverlay1 = .;	LONG(ADDR(CodeOverlay1));
LONG((SIZEOF(CodeOverlay1) + 127) & ~(127));
LONG(LOADADDR(CodeOverlay1)-_start);
LONG(0);

. = ALIGN(0x10);	__ovly_info_CodeOverlay2 = .;	LONG(ADDR(CodeOverlay2));
LONG((SIZEOF(CodeOverlay2) + 127) & ~(127));
LONG(LOADADDR(CodeOverlay2)-_start);
LONG(0);

. = ALIGN(0x10);	__ovly_info_CodeOverlay3 = .;	LONG(ADDR(CodeOverlay3));
LONG((SIZEOF(CodeOverlay3) + 127) & ~(127));
LONG(LOADADDR(CodeOverlay3)-_start);
LONG(0);

. = ALIGN(0x10);	__ovly_info_CodeOverlay4 = .;	LONG(ADDR(CodeOverlay4));
LONG((SIZEOF(CodeOverlay4) + 127) & ~(127));
LONG(LOADADDR(CodeOverlay4)-_start);
LONG(0);

. = ALIGN(0x10);	__ovly_info_CodeOverlay5 = .;	LONG(ADDR(CodeOverlay5));
LONG((SIZEOF(CodeOverlay5) + 127) & ~(127));
LONG(LOADADDR(CodeOverlay5)-_start);
LONG(0);

. = ALIGN(0x10);	_novlys = .;	LONG(2);. = ALIGN(0x10);
 
    *(.data .data.* .gnu.linkonce.d.*)
    KEEP (*(.gnu.linkonce.d.*personality*))
    SORT(CONSTRUCTORS)
  }
  .data1          : { *(.data1) }
  .toe ALIGN(128) : { *(.toe) } = 0
  _edata = .;
  PROVIDE (edata = .);
  . = ALIGN(128);
  
  __ppudata_start = .;
  __ppudata_module_offset = (__ppudata_start - _start);
  PpuData __ppudata_module_offset : AT (__ppudata_start) { *(ppudata) }
  
  __bss_loadstart = LOADADDR(PpuData) + SIZEOF(PpuData);
  .bss      __ppudata_start : AT (__bss_loadstart)
  {
   *(.dynbss)
   *(.bss .bss.* .gnu.linkonce.b.*)
   *(COMMON)
   /* Align here to ensure that the .bss section occupies space up to
      _end.  Align after .bss to ensure correct alignment even if the
      .bss section disappears because there are no input sections.  */
   . = ALIGN(16);
  }
  . = ALIGN(16);
  __bss_start = ADDR(.bss);
  _end = __bss_start + SIZEOF(.bss);
  PROVIDE (end = .);
  
  /* Stabs debugging sections.  */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment       0 : { *(.comment) }
  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */
  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
  PROVIDE (__stack = 0x3fff0);
  /DISCARD/ : { *(.note.GNU-stack) }
  
}

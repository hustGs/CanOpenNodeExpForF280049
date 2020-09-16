#ifndef _CODE_IN_FLASH
   #define _BEGIN_START 0x000000
#else
   #define _BEGIN_START 0x080000
#endif
#ifndef _CODE_IN_FLASH
   #define _FLASH_BANK0_START 0x080000
#else
   #define _FLASH_BANK0_START 0x080002
#endif
#ifndef _CODE_IN_FLASH
   #define _FLASH_BANK0_START_LEN 0x001000
#else
   #define _FLASH_BANK0_START_LEN 0x000FFE
#endif

MEMORY
{
PAGE 0 :
   /* BEGIN is used for the "boot to SARAM" bootloader mode   */

   BEGIN           	: origin = _BEGIN_START, length = 0x000002

   RAMM0          	: origin = 0x0000F3, length = 0x000186 // for m0 section1
   RAMM01			: origin = 0x000279, length = 0x000186 // for m0 section2

   RAMLS0          	: origin = 0x008000, length = 0x000800 // Local Shared RAM between CPU and CLA
   RAMLS1          	: origin = 0x008800, length = 0x000800
   RAMLS2      		: origin = 0x009000, length = 0x000800
   RAMLS3      		: origin = 0x009800, length = 0x000800
   RAMLS4      		: origin = 0x00A000, length = 0x000800
   RESET           	: origin = 0x3FFFC0, length = 0x000002

 /* Flash sectors: you can use FLASH for program memory when the RAM is filled up*/
   /* BANK 0 */
   FLASH_BANK0_SEC0  : origin = _FLASH_BANK0_START, length = _FLASH_BANK0_START_LEN	/* on-chip Flash */
   FLASH_BANK0_SEC1  : origin = 0x081000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK0_SEC2  : origin = 0x082000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK0_SEC3  : origin = 0x083000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK0_SEC4  : origin = 0x084000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK0_SEC5  : origin = 0x085000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK0_SEC6  : origin = 0x086000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK0_SEC7  : origin = 0x087000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK0_SEC8  : origin = 0x088000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK0_SEC9  : origin = 0x089000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK0_SEC10 : origin = 0x08A000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK0_SEC11 : origin = 0x08B000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK0_SEC12 : origin = 0x08C000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK0_SEC13 : origin = 0x08D000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK0_SEC14 : origin = 0x08E000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK0_SEC15 : origin = 0x08F000, length = 0x001000	/* on-chip Flash */

  /* BANK 1 */
   FLASH_BANK1_SEC0  : origin = 0x090000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK1_SEC1  : origin = 0x091000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK1_SEC2  : origin = 0x092000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK1_SEC3  : origin = 0x093000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK1_SEC4  : origin = 0x094000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK1_SEC5  : origin = 0x095000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK1_SEC6  : origin = 0x096000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK1_SEC7  : origin = 0x097000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK1_SEC8  : origin = 0x098000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK1_SEC9  : origin = 0x099000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK1_SEC10 : origin = 0x09A000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK1_SEC11 : origin = 0x09B000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK1_SEC12 : origin = 0x09C000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK1_SEC13 : origin = 0x09D000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK1_SEC14 : origin = 0x09E000, length = 0x001000	/* on-chip Flash */
   FLASH_BANK1_SEC15 : origin = 0x09F000, length = 0x001000	/* on-chip Flash */

PAGE 1 :

   BOOT_RSVD       : origin = 0x000002, length = 0x0000F1     /* Part of M0, BOOT rom will use this for stack */
   RAMM1           : origin = 0x000400, length = 0x0003F8     /* on-chip RAM block M1 */
//   RAMM1_RSVD      : origin = 0x0007F8, length = 0x000008     /* Reserve and do not use for code as per the errata advisory "Memory: Prefetching Beyond Valid Memory" */

   RAMLS5      : origin = 0x00A800, length = 0x000A00
   RAMLS6      : origin = 0x00B200, length = 0x000600
   RAMLS7      : origin = 0x00B800, length = 0x000800
   
   RAMGS0      : origin = 0x00C000, length = 0x002000
   RAMGS1      : origin = 0x00E000, length = 0x002000
   RAMGS2      : origin = 0x010000, length = 0x002000
   RAMGS3      : origin = 0x012000, length = 0x001FF8
//   RAMGS3_RSVD : origin = 0x013FF8, length = 0x000008     /* Reserve and do not use for code as per the errata advisory "Memory: Prefetching Beyond Valid Memory" */
}

/*You can arrange the .text, .cinit, .const, .pinit, .switch and .econst to FLASH when RAM is filled up.*/
SECTIONS
{

#ifndef _CODE_IN_FLASH
	codestart        : > BEGIN,     PAGE = 0
   .TI.ramfunc      : > RAMM0,      PAGE = 0
   .text            : >> RAMGS0| RAMGS1  | RAMGS2 |RAMGS3,   	PAGE = 1 //code and float const
   .cinit           : >  RAMLS0,     PAGE = 0 //initialized global/static variables
   .switch          : > RAMM0,     PAGE = 0 //
#else
	codestart        : > BEGIN,     PAGE = 0, ALIGN(4)
   .text            : >> FLASH_BANK0_SEC2 | FLASH_BANK0_SEC3 | FLASH_BANK0_SEC5,   PAGE = 0, ALIGN(4)
   .cinit           : > FLASH_BANK0_SEC1,     PAGE = 0, ALIGN(4)
   .switch          : > FLASH_BANK0_SEC1,     PAGE = 0, ALIGN(4)
//   .TI.ramfunc      : > RAMM0,      PAGE = 0

   	#if defined(__TI_EABI__)
	   .TI.ramfunc      : LOAD = FLASH_BANK0_SEC1,
	                      RUN = RAMLS0,
	                      LOAD_START(RamfuncsLoadStart),
	                      LOAD_SIZE(RamfuncsLoadSize),
	                      LOAD_END(RamfuncsLoadEnd),
	                      RUN_START(RamfuncsRunStart),
	                      RUN_SIZE(RamfuncsRunSize),
	                      RUN_END(RamfuncsRunEnd),
	                      PAGE = 0, ALIGN(4)
	#else
	   .TI.ramfunc      : LOAD = FLASH_BANK0_SEC1,
	                      RUN = RAMLS0,
	                      LOAD_START(_RamfuncsLoadStart),
	                      LOAD_SIZE(_RamfuncsLoadSize),
	                      LOAD_END(_RamfuncsLoadEnd),
	                      RUN_START(_RamfuncsRunStart),
	                      RUN_SIZE(_RamfuncsRunSize),
	                      RUN_END(_RamfuncsRunEnd),
	                      PAGE = 0, ALIGN(4)
	#endif
#endif

   .reset           : > RESET,     PAGE = 0, TYPE = DSECT /* not used, */
   .stack           : > RAMLS4,     PAGE = 0  //stack use an exclusive space

#if defined(__TI_EABI__)
   .bss             : > RAMLS5,     PAGE = 1 //uninitialized global/static variables
   .bss:output      : > RAMLS5,     PAGE = 1
	#ifndef _CODE_IN_FLASH
	   .init_array      : > RAMM0,      PAGE = 0
	   .const           : > RAMLS5,     PAGE = 1
	#else
	   .init_array      : > FLASH_BANK0_SEC1,       PAGE = 0,       ALIGN(4)
	   .const           : > FLASH_BANK0_SEC4,       PAGE = 0,       ALIGN(4)
	#endif
   .data            : > RAMLS5,     PAGE = 1
   .sysmem          : > RAMLS5,     PAGE = 1
   .bss:cio         : > RAMLS0,     PAGE = 0
#else
	#ifndef _CODE_IN_FLASH
	   .pinit           : > RAMM0,      PAGE = 0
	   .econst          : > RAMLS5,     PAGE = 1
   #else
	   .pinit           : > FLASH_BANK0_SEC1,       PAGE = 0,       ALIGN(4)
	   .econst          : > FLASH_BANK0_SEC4,    PAGE = 0, ALIGN(4)
   #endif

   .ebss            : >> RAMLS1 | RAMLS2 | RAMLS3,     PAGE = 0
   .esysmem         : > RAMLS7,     PAGE = 1
   .cio             : > RAMLS0,     PAGE = 0 
#endif

   ramgs0           : > RAMGS0,    PAGE = 1
   ramgs1           : > RAMGS1,    PAGE = 1  

}


/*
//===========================================================================
// End of file.
//===========================================================================
*/

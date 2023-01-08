#ifndef FLASH_BLOB_H
#define FLASH_BLOB_H
#include "rtthread.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define VERS       1           // Interface Version 1.01

#define UNKNOWN    0           // Unknown
#define ONCHIP     1           // On-chip Flash Memory
#define EXT8BIT    2           // External Flash Device on 8-bit  Bus
#define EXT16BIT   3           // External Flash Device on 16-bit Bus
#define EXT32BIT   4           // External Flash Device on 32-bit Bus
#define EXTSPI     5           // External Flash Device on SPI

#define SECTOR_NUM 16         // Max Number of Sector Items

struct FlashSectors  {
  unsigned long   szSector;    // Sector Size in Bytes
  unsigned long AddrSector;    // Address of Sector
};

typedef struct FlashDevice  {
   unsigned short     Vers;    // Version Number and Architecture
   char       DevName[128];    // Device Name and Description
   unsigned short  DevType;    // Device Type: ONCHIP, EXT8BIT, EXT16BIT, ...
   unsigned long    DevAdr;    // Default Device Start Address
   unsigned long     szDev;    // Total Size of Device
   unsigned long    szPage;    // Programming Page Size
   unsigned long       Res;    // Reserved for future Extension
   unsigned char  valEmpty;    // Content of Erased Memory

   unsigned long    toProg;    // Time Out of Program Page Function
   unsigned long   toErase;    // Time Out of Erase Sector Function

   struct FlashSectors sectors[SECTOR_NUM];
}flash_dev_t;

typedef struct {
	int (*Init)(uint32_t adr, uint32_t clk, uint32_t fnc);
	int (*UnInit)(uint32_t fnc);
    int (*EraseChip)(void);
	int (*EraseSector)(uint32_t adr);
	int (*ProgramPage)(uint32_t adr, uint32_t sz, uint8_t* buf);  
}flash_ops_t;

typedef struct
{
    flash_ops_t tFlashops;
    flash_dev_t *ptFlashDev;
    int *pPrgData;
    rt_slist_t slist;      
}flash_blob_t;


extern void flash_dev_register(flash_blob_t *ptFlashDevice);
extern bool target_flash_init(uint32_t flash_start, int32_t size);
extern bool target_flash_uninit(uint32_t flash_start);
extern int32_t target_flash_write(uint32_t addr, const uint8_t *buf, int32_t size);
extern int32_t target_flash_erase(uint32_t addr, int32_t size);
extern int32_t target_flash_read(uint32_t addr, const uint8_t *buf, int32_t size);
#endif

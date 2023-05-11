#ifndef FLASH_BLOB_H
#define FLASH_BLOB_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#ifndef RT_NULL
#define RT_NULL                         (0)
struct rt_slist_node
{
    struct rt_slist_node *next;                         /**< point to next node. */
};
typedef struct rt_slist_node rt_slist_t;                /**< Type for single list. */


#define rt_container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

#define RT_SLIST_OBJECT_INIT(object) { RT_NULL }


static __inline void rt_slist_init(rt_slist_t *l)
{
    l->next = RT_NULL;
}

static __inline void rt_slist_append(rt_slist_t *l, rt_slist_t *n)
{
    struct rt_slist_node *node;

    node = l;
    while (node->next) node = node->next;

    /* append the node to the tail */
    node->next = n;
    n->next = RT_NULL;
}


#define rt_slist_entry(node, type, member) \
    rt_container_of(node, type, member)


#define rt_slist_for_each(pos, head) \
    for (pos = (head)->next; pos != RT_NULL; pos = pos->next)

#endif




#define VERS       1           // Interface Version 1.01

#define UNKNOWN    0           // Unknown
#define ONCHIP     1           // On-chip Flash Memory
#define EXT8BIT    2           // External Flash Device on 8-bit  Bus
#define EXT16BIT   3           // External Flash Device on 16-bit Bus
#define EXT32BIT   4           // External Flash Device on 32-bit Bus
#define EXTSPI     5           // External Flash Device on SPI

#define SECTOR_NUM 16         // Max Number of Sector Items

#define FLASH_DRV_VERS (0x0100+VERS)

#define SECTOR_END 0xFFFFFFFF, 0xFFFFFFFF

struct FlashSectors  {
    unsigned long   szSector;    // Sector Size in Bytes
    unsigned long   AddrSector;    // Address of Sector
};

typedef struct FlashDevice  {
    unsigned short  Vers;       // Version Number and Architecture
    char            DevName[32];// Device Name and Description
    unsigned short  DevType;    // Device Type: ONCHIP, EXT8BIT, EXT16BIT, ...
    unsigned long   DevAdr;    // Default Device Start Address
    unsigned long   szDev;    // Total Size of Device
    unsigned long   szPage;    // Programming Page Size
    unsigned long   Res;    // Reserved for future Extension
    unsigned char   valEmpty;    // Content of Erased Memory

    unsigned long   toProg;    // Time Out of Program Page Function
    unsigned long   toErase;    // Time Out of Erase Sector Function

    struct FlashSectors sectors[SECTOR_NUM];
} flash_dev_t;

typedef struct {
    int32_t (*Init)(uint32_t adr, uint32_t clk, uint32_t fnc);
    int32_t (*UnInit)(uint32_t fnc);
    int32_t (*EraseChip)(void);
    int32_t (*EraseSector)(uint32_t adr);
    int32_t (*ProgramPage)(uint32_t adr, uint32_t sz, uint8_t* buf);
    int32_t (*Read)(uint32_t adr, uint32_t sz, uint8_t* buf);
} flash_ops_t;

typedef struct {
    flash_ops_t tFlashops;
    flash_dev_t const *ptFlashDev;
    rt_slist_t slist;
} flash_blob_t;


extern void flash_dev_register(flash_blob_t *ptFlashDevice);
extern bool target_flash_init(uint32_t addr);
extern bool target_flash_uninit(uint32_t addr);
extern int32_t target_flash_write(uint32_t addr, const uint8_t *buf, int32_t size);
extern int32_t target_flash_erase(uint32_t addr, int32_t size);
extern int32_t target_flash_read(uint32_t addr, uint8_t *buf, int32_t size);
#endif

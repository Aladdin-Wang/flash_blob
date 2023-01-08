#include "flash_blob.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

//#define DRV_DEBUG
#define LOG_TAG             "flash"
#include <drv_log.h>

static bool s_bIsInit = false;
static rt_slist_t _slist_head = RT_SLIST_OBJECT_INIT(_slist_head);
register int *pPrgDataBase __asm("r9");

void flash_dev_register(flash_blob_t *ptFlashDevice)
{
    rt_slist_init(&(ptFlashDevice->slist));
    rt_slist_append(&_slist_head, &(ptFlashDevice->slist));
}

static flash_blob_t * flash_dev_find(uint32_t flash_start)
{
    rt_slist_t *node;
    rt_slist_for_each(node, &_slist_head) {
        flash_blob_t *ptFlashDevice = rt_slist_entry(node, flash_blob_t, slist);

        if(flash_start >= ptFlashDevice->ptFlashDev->DevAdr &&
           flash_start < ptFlashDevice->ptFlashDev->DevAdr + ptFlashDevice->ptFlashDev->szDev) {
            return ptFlashDevice;
        }
    }
    return NULL;
}

bool target_flash_init(uint32_t flash_start, int32_t size)
{
    if (flash_start % 4 != 0) {
        LOG_E("flash addr must be 4-byte alignment");
        return NULL;
    }

    flash_blob_t *ptFlashDevice = flash_dev_find(flash_start);

    if(ptFlashDevice != NULL) {
        pPrgDataBase = ptFlashDevice->pPrgData;
        ptFlashDevice->tFlashops.Init(flash_start, 0, 0);
        return true;
    }

    return false;
}

bool target_flash_uninit(uint32_t flash_start)
{
    flash_blob_t *ptFlashDevice = flash_dev_find(flash_start);

    if(ptFlashDevice != NULL) {
        pPrgDataBase = ptFlashDevice->pPrgData;
        ptFlashDevice->tFlashops.UnInit(flash_start);
        return true;
    }

    return true;
}

int target_flash_write(uint32_t addr, const uint8_t *buf, int32_t size)
{
    flash_blob_t *ptFlashDevice = flash_dev_find(addr);

    if(ptFlashDevice != NULL) {
        pPrgDataBase = ptFlashDevice->pPrgData;

        while(size > 0) {
            uint32_t write_size = size > ptFlashDevice->ptFlashDev->szPage ? ptFlashDevice->ptFlashDev->szPage : size;

            if( 0 != ptFlashDevice->tFlashops.ProgramPage(addr, write_size, (uint8_t *)buf)) {
                LOG_E("Programming Failed");
                return -1;
            }

            addr += write_size;
            buf  += write_size;
            size -= write_size;
        }

        return size;
    }

    return -1;
}

int32_t target_flash_read(uint32_t addr, const uint8_t *buf, int32_t size)
{
    flash_blob_t *ptFlashDevice;
    pPrgDataBase = ptFlashDevice->pPrgData;
    return size;
}

int32_t target_flash_erase(uint32_t addr, int32_t size)
{
    int32_t wSector, wRemainLen;
    flash_blob_t *ptFlashDevice = flash_dev_find(addr);

    if(ptFlashDevice != NULL) {
        if (size > ptFlashDevice->ptFlashDev->szDev) {
            LOG_E("erase outrange flash size! addr is (0x%p)\n", (void *)(addr + size));
            return -1;
        }

        pPrgDataBase = ptFlashDevice->pPrgData;
        wRemainLen = size;
        while(wRemainLen > 0) {
            if(0 != ptFlashDevice->tFlashops.EraseSector(addr)) {
                LOG_E("erase Failed! addr is (0x%p)\n", (void *)addr);
                return -1;
            }

            for(wSector = 0; wSector < SECTOR_NUM; wSector++) {
                if(ptFlashDevice->ptFlashDev->sectors[wSector + 1].szSector == 0XFFFFFFFF )
                    break;

                if(((addr - ptFlashDevice->ptFlashDev->DevAdr) < ptFlashDevice->ptFlashDev->sectors[wSector + 1].AddrSector) &&
                   ((addr - ptFlashDevice->ptFlashDev->DevAdr) >= ptFlashDevice->ptFlashDev->sectors[wSector].AddrSector) )
                    break;
            }

            addr += ptFlashDevice->ptFlashDev->sectors[wSector].szSector;
            wRemainLen -= ptFlashDevice->ptFlashDev->sectors[wSector].szSector;
        }

        return size;
    }

    return -1;
}

int32_t target_flash_verify (uint32_t addr, uint8_t *buf, int32_t size)
{

    return size;
}

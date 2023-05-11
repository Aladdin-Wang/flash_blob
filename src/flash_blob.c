#include "flash_blob.h"

static rt_slist_t _slist_head = RT_SLIST_OBJECT_INIT(_slist_head);

static flash_blob_t * flash_dev_find(uint32_t addr)
{
    rt_slist_t *node;
    rt_slist_for_each(node, &_slist_head) {
        flash_blob_t *ptFlashDevice = rt_slist_entry(node, flash_blob_t, slist);

        if(addr >= ptFlashDevice->ptFlashDev->DevAdr &&
           addr < ptFlashDevice->ptFlashDev->DevAdr + ptFlashDevice->ptFlashDev->szDev) {
            return ptFlashDevice;
        }
    }
    return NULL;
}

void flash_dev_register(flash_blob_t *ptFlashDevice)
{
    rt_slist_init(&(ptFlashDevice->slist));
    rt_slist_append(&_slist_head, &(ptFlashDevice->slist));
}

bool target_flash_init(uint32_t addr)
{
    if (addr % 4 != 0) {
        /*LOG_E("flash addr must be 4-byte alignment");*/
        return NULL;
    }

    flash_blob_t *ptFlashDevice = flash_dev_find(addr);

    if(ptFlashDevice != NULL) {
        ptFlashDevice->tFlashops.Init(addr, 0, 0);
        return true;
    }

    return false;
}

bool target_flash_uninit(uint32_t addr)
{
    flash_blob_t *ptFlashDevice = flash_dev_find(addr);

    if(ptFlashDevice != NULL) {
        ptFlashDevice->tFlashops.UnInit(addr);
        return true;
    }

    return true;
}

int target_flash_write(uint32_t addr, const uint8_t *buf, int32_t size)
{
    flash_blob_t *ptFlashDevice = flash_dev_find(addr);
    uint32_t wWriteSize = size;
    if(ptFlashDevice != NULL) {
        if ((addr - ptFlashDevice->ptFlashDev->DevAdr + size) > ptFlashDevice->ptFlashDev->szDev)
        {
            /*LOG_E("write outrange flash size! addr is (0x%p)", (void *)(addr + size));*/
            return -1;
        }
        while(wWriteSize > 0) {
            uint32_t wWritelength = wWriteSize > ptFlashDevice->ptFlashDev->szPage ? ptFlashDevice->ptFlashDev->szPage : wWriteSize;

            if( 0 != ptFlashDevice->tFlashops.ProgramPage(addr, wWritelength, (uint8_t *)buf)) {
                /*LOG_E("Programming Failed");*/
                return -1;
            }

            addr += wWritelength;
            buf  += wWritelength;
            wWriteSize -= wWritelength;
        }
        return size;
    }

    return -1;
}

int32_t target_flash_read(uint32_t addr, uint8_t *buf, int32_t size)
{
    flash_blob_t *ptFlashDevice = flash_dev_find(addr);

    if(ptFlashDevice != NULL) {
        if ((addr - ptFlashDevice->ptFlashDev->DevAdr + size) > ptFlashDevice->ptFlashDev->szDev)
        {
            /*LOG_E("read outrange flash size! addr is (0x%p)", (void *)(addr + size));*/
            return -1;
        }
        if(ptFlashDevice->tFlashops.Read) {
            if( 0 != ptFlashDevice->tFlashops.Read(addr, size, (uint8_t *)buf)) {
                /*LOG_E("Programming Failed");*/
                return -1;
            }
        } else {
            for (uint16_t i = 0; i < size; i++, buf++, addr++) {
                *buf = *(uint8_t *) addr;
            }
        }

        return size;
    }

    return -1;
}

int32_t target_flash_erase(uint32_t addr, int32_t size)
{
    int32_t wSector, wRemainLen;
    flash_blob_t *ptFlashDevice = flash_dev_find(addr);

    if(ptFlashDevice != NULL) {
        if (size > ptFlashDevice->ptFlashDev->szDev) {
            /*LOG_E("erase outrange flash size! addr is (0x%p)\n", (void *)(addr + size));*/
            return -1;
        }
        wRemainLen = size;

        while(wRemainLen > 0) {
            if(0 != ptFlashDevice->tFlashops.EraseSector(addr)) {
                /*LOG_E("erase Failed! addr is (0x%p)\n", (void *)addr);*/
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


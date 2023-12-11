#include "flash_blob.h"

extern const  flash_blob_t  onchip_flash_device;

static const flash_blob_t * const flash_table[] = 
{
    &onchip_flash_device
};

static const size_t flash_table_len = sizeof(flash_table) / sizeof(flash_table[0]);

const flash_blob_t *  flash_dev_find(uint32_t addr)
{
    for (uint16_t i = 0; i < flash_table_len; i++) {
        if(addr >= flash_table[i]->ptFlashDev->DevAdr &&
           addr < flash_table[i]->ptFlashDev->DevAdr + flash_table[i]->ptFlashDev->szDev) {
            return flash_table[i];
        }
    }

    return NULL;
}

bool target_flash_init(uint32_t addr)
{
    if (addr % 4 != 0) {
        /*flash addr must be 4-byte alignment*/
        return false;
    }

    const flash_blob_t *ptFlashDevice = flash_dev_find(addr);

    if(ptFlashDevice != NULL) {
        ptFlashDevice->tFlashops.Init(addr, 0, 0);
        return true;
    }

    return false;
}

bool target_flash_uninit(uint32_t addr)
{
    const flash_blob_t *ptFlashDevice = flash_dev_find(addr);

    if(ptFlashDevice != NULL) {
        ptFlashDevice->tFlashops.UnInit(addr);
        return true;
    }

    return true;
}

int target_flash_write(uint32_t addr, const uint8_t *buf, size_t size)
{
    const flash_blob_t *ptFlashDevice = flash_dev_find(addr);

    __ATOM_CODE{
        if(ptFlashDevice != NULL) {
            if (addr % 4 != 0) {
                /*addr must be 4-byte alignment*/
				size = 0;
                continue;
            }

            if ((addr - ptFlashDevice->ptFlashDev->DevAdr + size) > ptFlashDevice->ptFlashDev->szDev) {
                /*write outrange flash size*/
				size = 0;
                continue;
            }

            if(ptFlashDevice->tFlashops.Program) {
                if( 0 != ptFlashDevice->tFlashops.Program(addr, size, (uint8_t *)buf)) {
                    /*Programming Failed*/
					size = 0;
                    continue;
                }
            } else {
				size = 0;
                continue;
            }
        }
    }
    return size;
}

int target_flash_read(uint32_t addr, uint8_t *buf, size_t size)
{
    const flash_blob_t *ptFlashDevice = flash_dev_find(addr);

    __ATOM_CODE{
        if(ptFlashDevice != NULL) {
            if (addr % 4 != 0) {
                /*addr must be 4-byte alignment*/
			    size = 0;
                continue;
            }

            if ((addr - ptFlashDevice->ptFlashDev->DevAdr + size) > ptFlashDevice->ptFlashDev->szDev) {
                /*read outrange flash size*/
			    size = 0;
                continue;
            }

            if(ptFlashDevice->tFlashops.Read) {
                if( 0 != ptFlashDevice->tFlashops.Read(addr, size, (uint8_t *)buf)) {
                    /*Read Failed*/
					size = 0;
                    continue;
                }
            } else {
                for (uint16_t i = 0; i < size; i++, buf++, addr++) {
                    *buf = *(uint8_t *) addr;
                }
            }
        }
    }

    return size;
}

int target_flash_erase(uint32_t addr, size_t size)
{
    size_t wSector = 0;
    size_t wEraseSize = 0;
    const flash_blob_t *ptFlashDevice = flash_dev_find(addr);
    if(ptFlashDevice != NULL) {
		__ATOM_CODE{
			if (size > ptFlashDevice->ptFlashDev->szDev) {
				/*erase outrange flash size */
				wEraseSize = 0;
				continue;
			}

			while(wEraseSize < size) {
				if(ptFlashDevice->tFlashops.EraseSector) {
					if(0 != ptFlashDevice->tFlashops.EraseSector(addr)) {
						/*erase Failed*/
						break;
					}
				} else {
					break;
				}

				for(wSector = 0; wSector < SECTOR_NUM; wSector++) {
					if(ptFlashDevice->ptFlashDev->sectors[wSector + 1].szSector == 0XFFFFFFFF )
						break;

					if(((addr - ptFlashDevice->ptFlashDev->DevAdr) < ptFlashDevice->ptFlashDev->sectors[wSector + 1].AddrSector) &&
					   ((addr - ptFlashDevice->ptFlashDev->DevAdr) >= ptFlashDevice->ptFlashDev->sectors[wSector].AddrSector) )
						break;
				}

				addr += ptFlashDevice->ptFlashDev->sectors[wSector].szSector;
				wEraseSize += ptFlashDevice->ptFlashDev->sectors[wSector].szSector;
			}
	    }
        return wEraseSize;
    }

    return 0;
}


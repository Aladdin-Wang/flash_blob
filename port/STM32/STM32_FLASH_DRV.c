#include "flash_blob.h"
#include "STM32_FLASH_DEV.c"
/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

static int32_t Init(uint32_t adr, uint32_t clk, uint32_t fnc)
{
    HAL_FLASH_Unlock();
    return (0);
}

/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

static int32_t UnInit(uint32_t fnc)
{
    HAL_FLASH_Lock();
    return (0);
}

/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */

static int32_t EraseChip(void)
{
    int32_t result = 0;
    uint32_t PAGEError = 0;
    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_MASSERASE;
#if defined(FLASH_BANK2_END)
    EraseInitStruct.Banks       = FLASH_BANK_2;
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
    {
        result = 1;
    }
#endif
    EraseInitStruct.Banks       = FLASH_BANK_1;
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
    {
        result = 1;
    }
    return result;
}

/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */
static int32_t EraseSector(uint32_t adr)
{
    int32_t result = 0;
    uint32_t PAGEError = 0;

    /*Variable used for Erase procedure*/
    FLASH_EraseInitTypeDef EraseInitStruct;
#if defined(FLASH_BANK2_END)
    /* Fill EraseInit structure*/
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = adr;
    EraseInitStruct.NbPages     = 1;
    if(adr > FLASH_BANK1_END){
        EraseInitStruct.Banks       = FLASH_BANK_2;
    }else{
        EraseInitStruct.Banks       = FLASH_BANK_1;
    }

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
    {
        result = 1;
    }
#else
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.Sector = (adr - FLASH_BANK1_BASE) / FLASH_SECTOR_SIZE;;
    EraseInitStruct.NbSectors   = 1;
    EraseInitStruct.Banks       = FLASH_BANK_1;
    EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
    {
        result = 1;
    }
#endif
    return result;
}

/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */
static int32_t ProgramPage(uint32_t addr, uint32_t sz, uint8_t* buf)
{
    int32_t result = 0;
	uint32_t write_granularity = FLASH_NB_32BITWORD_IN_FLASHWORD * 4;
    uint32_t write_size = write_granularity;
    uint32_t end_addr   = addr + sz - 1,write_addr;
	uint8_t write_buffer[32] = {0};
	write_addr = (uint32_t)buf;
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR);
    while (addr < end_addr) {
		if(end_addr - addr + 1 < write_granularity)
        {
            write_size = end_addr - addr + 1;
            for(size_t i = 0; i < write_size; i++)
            {
                write_buffer[i] = *((uint8_t *)(write_addr + i));
            }
            write_addr = (uint32_t)((uint32_t *)write_buffer);
        }
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, addr, write_addr) == HAL_OK) {
		   for(uint8_t i = 0; i < write_size; i++)
            {
                if (*(uint8_t *)(addr + i) != *(uint8_t *)(write_addr + i))
                {
                result = 1;
                break;
                }
            }
            addr += write_granularity;
            write_addr  += write_granularity;
        } else {
            result = 1;
            break;
        }
    }
    return result;
}

const  flash_blob_t  onchip_flash_device = {
    .tFlashops.Init = Init,  
    .tFlashops.UnInit = UnInit,  
    .tFlashops.EraseChip = EraseChip,  
    .tFlashops.EraseSector = EraseSector,  
    .tFlashops.Program = ProgramPage,  
    .tFlashops.Read = NULL,
    .ptFlashDev = &FlashDevice,
};


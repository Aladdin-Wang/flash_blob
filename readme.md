# flash_blob

## 1、介绍

利用MDK的FLM文件快速生成通用flash驱动。
- 同时支持多个flash芯片级联，根据地址自动选择flash驱动。
- 可以作为直接操作 `片内 Flash` 和 `SPI Flash` 的后端实现。

### 1.1、API接口
```c
extern bool target_flash_init(uint32_t flash_start, int32_t size);
extern bool target_flash_uninit(uint32_t flash_start);
extern int32_t target_flash_write(uint32_t addr, const uint8_t *buf, int32_t size);
extern int32_t target_flash_erase(uint32_t addr, int32_t size);
extern int32_t target_flash_read(uint32_t addr, const uint8_t *buf, int32_t size);
```

### 1.2、目录结构

| doc   | 文档         |
| ----- | ------------ |
| src   | 源代码       |
| inc   | 头文件       |
| tools | 驱动生成工具 |

### 1.3、许可证

Agile Upgrade 遵循 `Apache-2.0` 许可，详见 `LICENSE` 文件。



## 2、使用 flash_blob
1. 在rtthread软件包中找到flash_blob，然后添加进工程。

2. 添加对应芯片的代码进工程，如果有多个flash器件，可以连续添加。

注意：多个设备的话每个flash的FlashDevice 的设备起始地址不可重叠，flash抽象层根据地址，自动选择相应的驱动。

 以上步骤完成后，就可以快速使用了，例如将YMODEM接收到的数据，写到flash中，代码如下：


```c
uint8_t *ymodem_call_back_receive(uint8_t *pchBuffer, uint16_t hwSize)
{
    static char *s_pchFileName = NULL, *s_pchFileSize = NULL;
    static uint32_t s_wFileSize = 0, s_wRemainLen = 0, s_wOffSet = 0;

    static enum  {
        START = 0,
        RECEIVE,
        END,
    } s_tState = {START};

    switch(s_tState) {
        case START:
            s_wOffSet = 0;
            s_pchFileName = (char *)&pchBuffer[0];
            s_pchFileSize = (char *)&pchBuffer[strlen(s_pchFileName) + 1];
            s_wFileSize = atol(s_pchFileSize);

            LOG_D("Ymodem file_name:%s", s_pchFileName);
            LOG_D("Ymodem file_size:%d", s_wFileSize);

			if(target_flash_init(APP_PART_ADDR, s_wFileSize) == false) {
				LOG_E("target flash uninit.");
				return NULL;
			}

			if(target_flash_erase(APP_PART_ADDR, s_wFileSize) < 0) {
				LOG_E("target flash erase error.");
				return NULL;
			}            

            s_tState = RECEIVE;
            break;

        case RECEIVE:
            s_wRemainLen = s_wFileSize - s_wOffSet;

            if(hwSize > s_wRemainLen) {
                hwSize = s_wRemainLen;
                s_tState = END;
            }

			if(target_flash_write(APP_PART_ADDR + s_wOffSet, pchBuffer, hwSize) < 0) {
				LOG_E("target flash write data error.");
				return NULL;
			}
			s_wOffSet += hwSize;
            
            break;

        case END:
            target_flash_uninit(APP_PART_ADDR);                        
            s_tState = START;
            break;
    }

    return s_chBuffer;
}
```

 
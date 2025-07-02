#include <string.h>
#include "ff_gen_drv.h"


#define BLOCK_SIZE                512
#define RAM_DEVICE_SIZE						(BLOCK_SIZE * 128)

#pragma data_alignment=1 
__ALIGN_BEGIN static uint8_t ucDiskSpace[RAM_DEVICE_SIZE] @ ".dtcm" __ALIGN_END;
#define RAM_DEVICE_ADDR						(ucDiskSpace)

static volatile DSTATUS Stat = STA_NOINIT;


static DSTATUS
	RAMDISK_initialize (BYTE);
static DSTATUS 
	RAMDISK_status (BYTE);
static DRESULT
	RAMDISK_read (BYTE, BYTE*, DWORD, UINT);
#if _USE_WRITE == 1
static DRESULT
	RAMDISK_write (BYTE, const BYTE*, DWORD, UINT);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
static DRESULT
	RAMDISK_ioctl (BYTE, BYTE, void*);
#endif /* _USE_IOCTL == 1 */


const Diskio_drvTypeDef RAMDISK_Driver = {
  RAMDISK_initialize,
  RAMDISK_status,
  RAMDISK_read, 
#if  _USE_WRITE == 1
  RAMDISK_write,
#endif /* _USE_WRITE == 1 */  
#if  _USE_IOCTL == 1
  RAMDISK_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* Routines ------------------------------------------------------------------*/
static DSTATUS
	RAMDISK_initialize(BYTE lun) {
  
	Stat = STA_NOINIT;
  /* Configure the SRAM device */
  //BSP_SRAM_Init();
	memset(ucDiskSpace, 0xFF, sizeof(ucDiskSpace));
  
  Stat &= ~STA_NOINIT;
  return Stat;
}


static DSTATUS
	RAMDISK_status(BYTE lun) {
  
	Stat = STA_NOINIT;
  Stat &= ~STA_NOINIT;

  return Stat;
}


static DRESULT
	RAMDISK_read(BYTE lun, BYTE *buff, DWORD sector, UINT count) {
  
	uint32_t BufferSize = (BLOCK_SIZE * count); 
  uint8_t *pSramAddress = (uint8_t *) (RAM_DEVICE_ADDR + (sector * BLOCK_SIZE)); 
  
  for(; BufferSize != 0; BufferSize--)
  {
    *buff++ = *(__IO uint8_t *)pSramAddress++;  
  } 
  
  return RES_OK;
}

static uint8_t * pSramAddressGlob = NULL;
static uint32_t BufferSizeGlob = 0;


#if _USE_WRITE == 1
static DRESULT
	RAMDISK_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count) {
  
	//uint32_t BufferSize = (BLOCK_SIZE * count) + count;
	uint32_t BufferSize = BLOCK_SIZE * count;
	BufferSizeGlob = BufferSize;
  uint8_t *pSramAddress = (uint8_t *) (RAM_DEVICE_ADDR + (sector * BLOCK_SIZE));
	pSramAddressGlob = pSramAddress;
  
  for(; BufferSize != 0; BufferSize--)
  {
    *(__IO uint8_t *)pSramAddress++ = *buff++;    
  } 
  
  return RES_OK;
}
#endif /* _USE_WRITE == 1 */


#if _USE_IOCTL == 1
static DRESULT
	RAMDISK_ioctl(BYTE lun, BYTE cmd, void *buff) {
  
	DRESULT res = RES_ERROR;
  
  if (Stat & STA_NOINIT) return RES_NOTRDY;
  
  switch (cmd)
  {
  /* Make sure that no pending write process */
  case CTRL_SYNC :
    res = RES_OK;
    break;
  
  /* Get number of sectors on the disk (DWORD) */
  case GET_SECTOR_COUNT :
    *(DWORD*)buff = RAM_DEVICE_SIZE / BLOCK_SIZE;
    res = RES_OK;
    break;
  
  /* Get R/W sector size (DWORD) */
  case GET_SECTOR_SIZE :
    *(DWORD*)buff = BLOCK_SIZE;
    res = RES_OK;
    break;
  
  /* Get erase block size in unit of sector (DWORD) */
  case GET_BLOCK_SIZE :
    *(DWORD*)buff = 1;
    break;
  
  default:
    res = RES_PARERR;
  }
  
  return res;
}
#endif /* _USE_IOCTL == 1 */
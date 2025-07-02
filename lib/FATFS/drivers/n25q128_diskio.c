#include <string.h>
#include "ff_gen_drv.h"
#include "stm32746g_discovery_qspi.h"

#define PAGE_SIZE					256 									//in bytes
#define SUBSECTOR_SIZE		(16*PAGE_SIZE)				//in bytes (4096/0x1000/4Kb) 
#define MAINSECTOR_SIZE		(16*SUBSECTOR_SIZE)		//in bytes (65536/0x10000/64Kb)
#define MEMORY_SIZE				(256*MAINSECTOR_SIZE)	//in bytes (0x1000000/16Mb)
#define DEVICE_ADDR				(0)


static volatile DSTATUS Stat = STA_NOINIT;
static uint8_t * pSramAddressGlob = NULL;
static uint32_t BufferSizeGlob = 0;

static DSTATUS
	N25Q128_initialize (BYTE);
static DSTATUS 
	N25Q128_status (BYTE);
static DRESULT
	N25Q128_read (BYTE, BYTE*, DWORD, UINT);
#if _USE_WRITE == 1
static DRESULT
N25Q128_write (BYTE, const BYTE*, DWORD, UINT);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
static DRESULT
	N25Q128_ioctl (BYTE, BYTE, void*);
#endif /* _USE_IOCTL == 1 */


const Diskio_drvTypeDef N25Q128DISK_Driver = {
  N25Q128_initialize,
  N25Q128_status,
  N25Q128_read, 
#if  _USE_WRITE == 1
  N25Q128_write,
#endif /* _USE_WRITE == 1 */  
#if  _USE_IOCTL == 1
  N25Q128_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* Routines ------------------------------------------------------------------*/
static DSTATUS
	N25Q128_initialize(BYTE lun) {
  
	BYTE res;
	
	Stat = STA_NOINIT;
	res = BSP_QSPI_Init();
	
	if (res == QSPI_OK)
		Stat = 0;
	
  return Stat;
}


static DSTATUS
	N25Q128_status(BYTE lun) {

  return Stat;
}


static DRESULT
	N25Q128_read(BYTE lun, BYTE *buff, DWORD sector, UINT count) {
  
	UINT BufferSize = count; 
  BYTE *pSramAddress = (BYTE *) (DEVICE_ADDR + (sector * SUBSECTOR_SIZE));
	pSramAddressGlob = pSramAddress;
	BufferSizeGlob = BufferSize;
	
	for(; BufferSize != 0; BufferSize--) {
		if (BSP_QSPI_Read(buff, (UINT)pSramAddress, SUBSECTOR_SIZE) != QSPI_OK)
			return RES_ERROR;
		pSramAddress += SUBSECTOR_SIZE;
		buff += SUBSECTOR_SIZE; 
  } 
  
  return RES_OK;
}


#if _USE_WRITE == 1
static DRESULT
	N25Q128_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count) {
  
	UINT BufferSize = count; 
  BYTE *pSramAddress = (BYTE *) (DEVICE_ADDR + (sector * SUBSECTOR_SIZE));
	pSramAddressGlob = pSramAddress;
	BufferSizeGlob = BufferSize;
	
	BSP_QSPI_Erase_Block((uint32_t)pSramAddress);
	
	for(; BufferSize != 0; BufferSize--) {
		if (BSP_QSPI_Write((uint8_t*)buff, (UINT)pSramAddress, SUBSECTOR_SIZE) != QSPI_OK)
			return RES_ERROR;
		pSramAddress += SUBSECTOR_SIZE;
		buff += SUBSECTOR_SIZE; 
  } 
  
  return RES_OK;
}
#endif /* _USE_WRITE == 1 */


#if _USE_IOCTL == 1
static DRESULT
	N25Q128_ioctl(BYTE lun, BYTE cmd, void *buff) {
  
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
    *(DWORD*)buff = MEMORY_SIZE / SUBSECTOR_SIZE;
    res = RES_OK;
    break;
  
  /* Get R/W sector size (DWORD) */
  case GET_SECTOR_SIZE :
    *(DWORD*)buff = SUBSECTOR_SIZE;
    res = RES_OK;
    break;
  
  /* Get erase block size in unit of sector (DWORD) */
  case GET_BLOCK_SIZE :
    *(DWORD*)buff = MAINSECTOR_SIZE/SUBSECTOR_SIZE;
    break;
  
  default:
    res = RES_PARERR;
  }
  
  return res;
}
#endif /* _USE_IOCTL == 1 */
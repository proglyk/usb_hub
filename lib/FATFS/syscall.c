/*------------------------------------------------------------------------*/
/* Sample code of OS dependent controls for FatFs                         */
/* (C)ChaN, 2014                                                          */
/*------------------------------------------------------------------------*/

#include <stdlib.h>		/* ANSI memory controls */
#include "../ff.h"

#if _FS_REENTRANT
/*-----------------------------------------------------------------------
 Create a Synchronization Object
------------------------------------------------------------------------
 This function is called in f_mount function to create a new
 synchronization object, such as semaphore and mutex. When a zero is
 returned, the f_mount function fails with FR_INT_ERR.
*/

/* TRUE:Function succeeded, FALSE:Could not create due to any error */
/* Corresponding logical drive being processed */
/* Pointer to return the created sync object */

int
	ff_cre_syncobj( BYTE vol, _SYNC_t *sobj) {
  
	if (__get_IPSR())
		return NULL;
	//vSemaphoreCreateBinary(*sobj);
	
	//*sobj = xQueueGenericCreate(1, semBINARY_SEMAPHORE_QUEUE_LENGTH, queueQUEUE_TYPE_BINARY_SEMAPHORE );
	*sobj = xQueueGenericCreate(1, NULL, queueQUEUE_TYPE_BINARY_SEMAPHORE );
	if (*sobj != NULL)
		xSemaphoreGive(*sobj);
	
	//vSemaphoreCreateBinary(*sobj);
  
  return (*sobj != NULL);
}



/*------------------------------------------------------------------------*/
/* Delete a Synchronization Object                                        */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount function to delete a synchronization
/  object that created with ff_cre_syncobj function. When a zero is
/  returned, the f_mount function fails with FR_INT_ERR.
*/

/* TRUE:Function succeeded, FALSE:Could not delete due to any error */
/* Sync object tied to the logical drive to be deleted */

int
	ff_del_syncobj( _SYNC_t sobj ) {
  
	if (__get_IPSR())
		return 0;
	vSemaphoreDelete(sobj);
	
  return 1;
}



/*------------------------------------------------------------------------*/
/* Request Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on entering file functions to lock the volume.
/  When a zero is returned, the file function fails with FR_TIMEOUT.
*/

/* TRUE:Got a grant to access the volume, FALSE:Could not get a grant */
/* Sync object to wait */

int
	ff_req_grant( _SYNC_t sobj ) {
  
	int ret = 0;
	portBASE_TYPE taskWoken = pdFALSE;
	
	if (__get_IPSR()) {
		if (xSemaphoreTakeFromISR(sobj, &taskWoken) == pdTRUE)
			ret = 1;
		portEND_SWITCHING_ISR(taskWoken);
	} else {
		if (xSemaphoreTake(sobj, _FS_TIMEOUT) == pdTRUE)
			ret = 1;
	}
  
  return ret;
}



/*------------------------------------------------------------------------*/
/* Release Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on leaving file functions to unlock the volume.
*/

/* Sync object to be signaled */

void
	ff_rel_grant(	_SYNC_t sobj ) {
	
	portBASE_TYPE taskWoken = pdFALSE;
	
	if (__get_IPSR()) {
		xSemaphoreGiveFromISR(sobj, &taskWoken);
		portEND_SWITCHING_ISR(taskWoken);
	} else {
		xSemaphoreGive(sobj);
	}
}

#endif




#if _USE_LFN == 3	/* LFN with a working buffer on the heap */
/*------------------------------------------------------------------------*/
/* Allocate a memory block                                                */
/*------------------------------------------------------------------------*/
/* If a NULL is returned, the file function fails with FR_NOT_ENOUGH_CORE.
*/

void* ff_memalloc (	/* Returns pointer to the allocated memory block */
	UINT msize		/* Number of bytes to allocate */
)
{
	return malloc(msize);	/* Allocate a new memory block with POSIX API */
}


/*------------------------------------------------------------------------*/
/* Free a memory block                                                    */
/*------------------------------------------------------------------------*/

void ff_memfree (
	void* mblock	/* Pointer to the memory block to free */
)
{
	free(mblock);	/* Discard the memory block with POSIX API */
}

#endif

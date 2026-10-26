/* common headers */
#include <stdint.h>
#include <stdio.h>
#include <spu_intrinsics.h>

/* Lv2 OS headers */
#include <sys/spu_thread.h>

/* spurs */
//#include <cell/spurs.h>
#include <cell/spurs/types.h>
#include <cell/spurs/task.h>

/* spu_printf */
#include <spu_printf.h>

/* system dma */
#include <cell/dma.h>
#define DMA_BUFFER_SIZE 1024
unsigned char buffer[DMA_BUFFER_SIZE];

void cellSpursMain(qword argTask, uint64_t argTaskset)
{
	(void)argTask;
	(void)argTaskset;
	
	vec_uint4 task_arg = (vec_uint4)argTask;
	uint32_t dest	= spu_extract(task_arg, 0);
	uint32_t source = spu_extract(task_arg, 1);
	uint32_t size   = spu_extract(task_arg, 2);
	uint32_t dmaSize;

//	spu_printf("Memcopy To %x From %x size %d\n",dest,source,size);

	while( size )
	{
		dmaSize = size > DMA_BUFFER_SIZE ? DMA_BUFFER_SIZE : size;
		size -= dmaSize;
//		spu_printf("Memcopy To %x From %x size %d\n",dest,source,size);
		// Read To LS From Source
		cellDmaLargeGet ((void*)buffer, (uint32_t)source, dmaSize, 0, 0, 0); 
		cellDmaWaitTagStatusAll(1 << 0);
		// Write From LS to Dest
		cellDmaLargePut ((void*)buffer, (uint32_t)dest, dmaSize, 0, 0, 0); 
		cellDmaWaitTagStatusAll(1 << 0);
		// Increment Source
		source += dmaSize;
		// Increment Dest
		dest += dmaSize;
	
	}
		

	cellSpursShutdownTaskset (cellSpursGetTasksetAddress ());
	cellSpursExit ();

	/* never reach here */
	return;
}



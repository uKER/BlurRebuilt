#include "Display/Utils/GCM/libp3x.h"

#ifndef __SPU__ 

#include <stdio.h>
#include <string.h>
#include <sys/paths.h>
#include <sys/memory.h>
#include <cell/gcm.h>

#include "Display/Utils/GCM/libp3x.h"

#define P3X_FOURCC(ch0, ch1, ch2, ch3) ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) | ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24 ))

#define P3X_DATABLOCK_FOURCC   P3X_FOURCC( 'D', 'a', 't', 'a' )
#define P3X_EVENT_FOURCC       P3X_FOURCC( 'E', 'v', 'n', 't' )
#define P3X_HEADER_FOURCC      P3X_FOURCC( 'P', 'i', 'x', '3' )

#define P3X_DATABLOCK_VERSION  1
#define P3X_EVENT_VERSION      5
#define P3X_HEADER_VERSION     4

// number of methods in gcm_macros.h - upper limit of address is 0x10000
#define RSX_NUM_METHODS 16384

uint32_t rsx_state_valid[RSX_NUM_METHODS] = { 0 };
uint32_t rsx_state_cache[RSX_NUM_METHODS] = { 0 };

uint32_t rsx_old_state_cache[RSX_NUM_METHODS] = { 0 };

uint32_t* p3x_prev_current_ptr = NULL;
uint32_t* p3x_prev_event_ptr = NULL;
uint32_t* p3x_begin_ptr = NULL;

FILE* p3x_file = NULL;
uint32_t p3x_capturing = 0;

uint32_t* p3x_callstack_return = NULL;

uint32_t p3x_draw_type = 0;
uint32_t p3x_draw_min_index = 0;
uint32_t p3x_draw_max_index = 0;
uint32_t p3x_draw_min_vertex = 0;
uint32_t p3x_draw_max_vertex = 0;

uint32_t p3x_verbose = 0;

uint32_t p3x_event_count = 0;

uint32_t p3x_skip_mip = 0;

CellGcmConfig p3x_cfg;

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

struct p3x_chunk
{
	uint32_t fourcc;
	uint32_t version;
	uint32_t size;
};

struct p3x_data_block
{  
	uint32_t Fourcc;  
	uint32_t Version;  
	uint32_t Size;  
	uint32_t Offset;  // destination offset and location (MSB)  
	char Name[32];  
	uint32_t Reserved[32];  
};  

struct p3x_event
{  
	uint32_t Fourcc;  
	uint32_t Version;  
	uint32_t Size;  
	char Name[32];  
	uint32_t Addr;
	uint32_t NopBlockOffset;
	char Description[64];
	uint32_t Reserved[16];
};

p3x_tile_setting p3x_tileinfo[P3X_TILE_COUNT]; // 14*7=98 dwords
p3x_zcull_setting p3x_zcullinfo[P3X_ZCULL_COUNT]; // 8*11=88 dwords

struct p3x_header
{  
	uint32_t Fourcc;  
	uint32_t Version;  
	uint32_t Size;  

	uint32_t Pad0[4];

	p3x_tile_setting tile[P3X_TILE_COUNT]; // 14*7=98 dwords
	p3x_zcull_setting zcull[P3X_ZCULL_COUNT]; // 8*11=88 dwords

	uint32_t Reserved[834];  
};

#define P3X_FOURCC_NAMED_EVENT P3X_FOURCC('n','e','v','t')

enum
{
	P3X_NAME_BEGIN,
	P3X_NAME_END,
	P3X_NAME_NEXT
};

struct p3x_named_event
{
	uint32_t Fourcc;  
	uint32_t Version;  
	uint32_t Size;  

	uint32_t type; 
	uint32_t cmdp;
};

uint32_t p3x_get_cache(uint32_t method)
{
    if (!rsx_state_valid[method/4]) 
    {
        printf("P3X: No value for register 0x%x.  Please check that your application initializes all possible render states at the beginning of the capture.\n", method);
        return 0;
    }
    return rsx_state_cache[method/4];
}

uint32_t p3x_get_address(uint32_t loc, uint32_t ofs)
{
  if (loc == CELL_GCM_LOCATION_LOCAL)
    return (uint32_t)p3x_cfg.localAddress + ofs;
  else
  {
    void* addr;
    cellGcmIoOffsetToAddress(ofs, &addr);
    return (uint32_t)addr;
  }
}

void p3x_init_state()
{
    memset(rsx_state_valid, 0, sizeof(rsx_state_valid));
    memset(rsx_state_cache, 0, sizeof(rsx_state_cache));

    p3x_draw_type = 0;
    p3x_draw_min_index = ~0;
    p3x_draw_max_index = 0;
    p3x_draw_min_vertex = ~0;
    p3x_draw_max_vertex = 0;
}

#define P3X_BLOCK_HISTORY_SIZE 256
p3x_data_block p3x_block_history[P3X_BLOCK_HISTORY_SIZE] = { 0 };
int p3x_next_block = 0;

uint32_t p3x_addr_to_offset_loc(void* addr)
{
	uint32_t ofs;
	cellGcmAddressToOffset(addr, &ofs);

	if ((uint32_t)addr >= (uint32_t)p3x_cfg.localAddress && (uint32_t)addr < (uint32_t)p3x_cfg.localAddress + p3x_cfg.localSize)
		return ofs | (CELL_GCM_LOCATION_LOCAL<<31);
	else
		return ofs | (CELL_GCM_LOCATION_MAIN<<31);
}

void p3x_write_block(void* data, uint32_t size)
{
	if (p3x_verbose) printf("P3X: Data block from %p of %d bytes.\n", data, size);

	p3x_data_block block;
	memset(&block, 0, sizeof(p3x_data_block) );

	block.Fourcc = P3X_DATABLOCK_FOURCC;
	block.Version = P3X_DATABLOCK_VERSION;
	block.Size = sizeof(p3x_data_block) + size;
	block.Offset = p3x_addr_to_offset_loc(data);

	// scan for duplicate blocks in the past N entries....note that this can cause incorrect rendering when external processes
	// are modifying memory during the rendering processes, for example a SPU thread generating vertex data.
	// todo- allow some way to flag memory regions as dynamic to p3x, and then skip this step for them.
	// todo- also would be nice to flag memory regions that are render targets, so that we would never write them to the file.
	for (uint32_t i = 0; i < P3X_BLOCK_HISTORY_SIZE; i++)
	{
		if (p3x_block_history[i].Offset == block.Offset && p3x_block_history[i].Size == block.Size)
		{
			if (p3x_verbose) printf("P3X: Duplicate data block eliminated.\n");
			return;
		}
	}
	p3x_block_history[p3x_next_block] = block;
	if (++p3x_next_block >= P3X_BLOCK_HISTORY_SIZE)
		p3x_next_block = 0;

  // test whether the data is in valid (mapped) memory.  requires scanning through page by page.
  // if the memory is not mapped, we print a message and ignore the block - it would cause the RSX to crash were it rendered, anyway.
  uint8_t* data_tester = (uint8_t*)data;
  while (data_tester < (uint8_t*)data + size)
  {
    sys_page_attr_t attr;
    int err = sys_memory_get_page_attribute((sys_addr_t)data_tester, &attr);
    if (err != CELL_OK || (attr.access_right&SYS_MEMORY_ACCESS_RIGHT_PPU_THR)==0)
    {
      uint32_t newsize = data_tester - (uint8_t*)data;
			printf("P3X: Attempted to write a data block from unmapped memory. The data block will be truncated. addr=%p size=%d newSize=%d.\n", data, size, newsize);
      size = newsize;
      if (!size)
        return;
    }
    if (attr.page_size == SYS_MEMORY_PAGE_SIZE_1M)
      data_tester += 1024*1024;
    else //if (attr.page_size == SYS_MEMORY_PAGE_SIZE_64K)
      data_tester += 64*1024;
  }

	fwrite(&block, 1, sizeof(p3x_data_block), p3x_file);
	fwrite(data, 1, size, p3x_file);
}

uint32_t rsx_textureformat_size[] =
{
	CELL_GCM_TEXTURE_B8                     , 1,
	CELL_GCM_TEXTURE_A1R5G5B5               , 2,
	CELL_GCM_TEXTURE_A4R4G4B4               , 2,
	CELL_GCM_TEXTURE_R5G6B5                 , 2,
	CELL_GCM_TEXTURE_A8R8G8B8               , 4,
	CELL_GCM_TEXTURE_COMPRESSED_DXT1        , 8,
	CELL_GCM_TEXTURE_COMPRESSED_DXT23       , 16,
	CELL_GCM_TEXTURE_COMPRESSED_DXT45       , 16,
	CELL_GCM_TEXTURE_COMPRESSED_R8B8_R8G8   , 4,
	CELL_GCM_TEXTURE_COMPRESSED_B8R8_G8R8   , 4,
	CELL_GCM_TEXTURE_G8B8                   , 2,
	CELL_GCM_TEXTURE_R6G5B5                 , 2,
	CELL_GCM_TEXTURE_DEPTH24_D8             , 4,
	CELL_GCM_TEXTURE_DEPTH24_D8_FLOAT       , 4,
	CELL_GCM_TEXTURE_DEPTH16                , 2,
	CELL_GCM_TEXTURE_DEPTH16_FLOAT          , 2,
	CELL_GCM_TEXTURE_X16                    , 2,
	CELL_GCM_TEXTURE_Y16_X16                , 4,
	CELL_GCM_TEXTURE_R5G5B5A1               , 2,
	CELL_GCM_TEXTURE_COMPRESSED_HILO8       , 2,
	CELL_GCM_TEXTURE_COMPRESSED_HILO_S8     , 2,
	CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT  , 8,
	CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT  , 16,
	CELL_GCM_TEXTURE_X32_FLOAT              , 4,
	CELL_GCM_TEXTURE_D1R5G5B5               , 2,
	CELL_GCM_TEXTURE_D8R8G8B8               , 4,
	CELL_GCM_TEXTURE_Y16_X16_FLOAT          , 4,
};

void p3x_write_event(const char* desc, uint32_t* cmdp, bool is_draw)
{
    p3x_event event;
    memset(&event, 0, sizeof(p3x_event));

    event.Fourcc = P3X_EVENT_FOURCC;
    event.Version = P3X_EVENT_VERSION;

    uint32_t size = ((uint32_t)cmdp - (uint32_t)p3x_prev_event_ptr);
    event.Size = sizeof(p3x_event) + size;

	event.Addr = (uint32_t)p3x_prev_event_ptr;

	if( p3x_begin_ptr )
		event.NopBlockOffset = (uint32_t)p3x_begin_ptr - (uint32_t)p3x_prev_event_ptr;
	else
		event.NopBlockOffset = ~0;
	p3x_begin_ptr = NULL;

    strncpy( event.Name, desc, sizeof(event.Name)-1 );

	if (p3x_verbose) printf("P3X: Writing event %s at command buffer location %p. size=%d nopOffset=%d isDraw=%d\n", 
		desc, cmdp, size, event.NopBlockOffset, is_draw);

    fwrite( &event, 1, sizeof(p3x_event), p3x_file );
    fwrite( p3x_prev_event_ptr, 1, size, p3x_file );

	if (p3x_verbose) printf("P3X: Event ranges from %p to %p.  size=%d\n", p3x_prev_event_ptr, cmdp, size);
	p3x_prev_event_ptr = cmdp;
	p3x_event_count++;

	// some events are merely to move the command buffer, or are resolves or clears.  with these events, we intentionally don't include
	// the jump, call or return opcode in the written data.  we also don't bother to write the event data like index and vertex buffers.
	if (!is_draw)
		return;

    // we always write out the fragment program, since it tends to get patched.
    uint32_t fp_offset = p3x_get_cache(CELL_GCM_NV4097_SET_SHADER_PROGRAM);
	// todo- overestimating the size for now. need to scan microcode for end bits, keeping track of jumps (to set max address) as well.
	uint32_t fp_size = 12*1024;
	if (p3x_verbose) printf("P3X: Writing fragment program at address %p.  size=%d\n", (uint8_t*)p3x_cfg.localAddress+(fp_offset&(~1)), fp_size);
    p3x_write_block((uint8_t*)p3x_cfg.localAddress+(fp_offset&(~1)), 8*1024); 

    // index buffer
    uint8_t* index_addr = NULL;
    uint32_t index_bytes = 0;
    if (p3x_draw_type == CELL_GCM_NV4097_DRAW_INDEX_ARRAY) 
    {
        // if we haven't got a valid index range, don't try to dump a buffer.
        if (p3x_draw_max_index == (uint32_t)0 || p3x_draw_min_index == (uint32_t)~0)
        {
            printf("P3X: Index buffer enabled, but unable to determine buffer size.  Capture data will not be correct, and may crash the GPU!\n");
            p3x_draw_type = 0;  // better abort the draw, too.
        }
        else
        {
            uint32_t format_reg = p3x_get_cache(CELL_GCM_NV4097_SET_INDEX_ARRAY_DMA);
            uint32_t offset_reg = p3x_get_cache(CELL_GCM_NV4097_SET_INDEX_ARRAY_ADDRESS);
    
            index_bytes = (format_reg>>4) == CELL_GCM_DRAW_INDEX_ARRAY_TYPE_16 ? 2 : 4;
    
            index_addr = (uint8_t*)p3x_get_address((format_reg&0xf), offset_reg);
            index_addr += index_bytes * p3x_draw_min_index;

            uint32_t index_count = (p3x_draw_max_index-p3x_draw_min_index)+1;
            if (p3x_verbose) printf("P3X: Writing index buffer at %p.  indexSize=%d minIndex=%d maxIndex=%d\n", index_addr, index_bytes, p3x_draw_min_index, p3x_draw_max_index);
            p3x_write_block(index_addr, index_bytes * index_count);
        }
    }

	// scan the index buffer for range of verts to write
	uint32_t vertex_attr_mask = p3x_get_cache(CELL_GCM_NV4097_SET_VERTEX_ATTRIB_INPUT_MASK);
	if (vertex_attr_mask && p3x_draw_type == CELL_GCM_NV4097_DRAW_INDEX_ARRAY) 
	{
		// get reset index params
		uint32_t reset_enable = p3x_get_cache(CELL_GCM_NV4097_SET_RESTART_INDEX_ENABLE);
		uint32_t reset_idx = p3x_get_cache(CELL_GCM_NV4097_SET_RESTART_INDEX);

		if (p3x_verbose) printf("P3X: Scanning index buffer for vertex range...\n");

		// scan index buffer in the correct mode
		uint32_t index_count = (p3x_draw_max_index-p3x_draw_min_index)+1;
		if (index_bytes == 2) 
		{
			for (uint16_t* index_ptr = (uint16_t*)index_addr; index_ptr < (uint16_t*)index_addr+index_count; index_ptr++)
			{
				if (reset_enable && *index_ptr == reset_idx)
					continue;
				p3x_draw_min_vertex = min(p3x_draw_min_vertex, *index_ptr);
				p3x_draw_max_vertex = max(p3x_draw_max_vertex, *index_ptr);
			}
		}
		else
		{
			for (uint32_t* index_ptr = (uint32_t*)index_addr; index_ptr < (uint32_t*)index_addr+index_count; index_ptr++)
			{
				if (reset_enable && *index_ptr == reset_idx)
					continue;
				p3x_draw_min_vertex = min(p3x_draw_min_vertex, *index_ptr);
				p3x_draw_max_vertex = max(p3x_draw_max_vertex, *index_ptr);
			}
		}

		if (p3x_verbose) printf("P3X: Finished calculating vertex range.  minVertex=%d maxVertex=%d\n", p3x_draw_min_vertex, p3x_draw_max_vertex);
	}

    // vertex arrays -- memory ranges are collected, and then collapsed into the minimum set.
    uint32_t vertex_data_start[16] = { 0 };
    uint32_t vertex_data_end[16] = { 0 };
    for (uint32_t i = 0; i < 16; i++) 
    {
        if ((vertex_attr_mask & (1<<i)) == 0) 
            continue;

        // if we haven't got a valid vertex range, don't try to dump a buffer.
        if (p3x_draw_max_vertex == (uint32_t)0 || p3x_draw_min_vertex == (uint32_t)~0)
        {
            printf("P3X: Vertex attribute %d enabled, but unable to determine buffer size.  Capture data will not be correct.\n", i);
        }
        else
        {
            uint32_t format_reg = p3x_get_cache(CELL_GCM_NV4097_SET_VERTEX_DATA_ARRAY_FORMAT+i*4);
            uint32_t offset_reg = p3x_get_cache(CELL_GCM_NV4097_SET_VERTEX_DATA_ARRAY_OFFSET+i*4);

            uint32_t stride = (format_reg>>8)&0xff;

            uint32_t vertex_addr = p3x_get_address((offset_reg>>31), (offset_reg&0x7fffffff));
            vertex_addr += stride * p3x_draw_min_vertex;

            uint32_t num_verts = (p3x_draw_max_vertex-p3x_draw_min_vertex)+1;
            if (p3x_verbose) printf("P3X: Recording vertex stream %d at %x.  numVerts=%d stride=%d\n", i, vertex_addr, num_verts, stride);

            vertex_data_start[i] = vertex_addr;
            vertex_data_end[i] = vertex_addr + stride * num_verts;
        }
    }

    // collapse vertex data blocks, by trying to merge each data stream into each other stream, when 
    // their memory ranges are contiguous or overlap.
    // 
    // this is because we receive a ton of independent "streams", each of which are usually just different
    // interleaved fields in a giant vertex buffer.  if we wrote out all the streams, we would redundantly
    // write the same data lots of times!
    if (p3x_verbose) printf("P3X: Collapsing vertex attributes into contiguous / overlapping regions...\n");
    for (uint32_t i = 15; i > 0; i--) 
    {
        if ((vertex_attr_mask & (1<<i)) == 0) 
            continue;
        for (uint32_t j = 0; j < i; j++) 
        {
            if ((vertex_attr_mask & (1<<j)) == 0) 
                continue;

// lame....
//                // one end of i is inside j
//            if ((vertex_data_start[i] >= vertex_data_start[j] && vertex_data_start[i] <= vertex_data_end[j]) ||
//                (vertex_data_end[i]   >= vertex_data_start[j] && vertex_data_end[i]   <= vertex_data_end[j]) ||
//                // one end of j is inside i
//                (vertex_data_start[j] >= vertex_data_start[i] && vertex_data_start[j] <= vertex_data_end[i]) ||
//                (vertex_data_end[j]   >= vertex_data_start[i] && vertex_data_end[j]   <= vertex_data_end[i]))

            // if the size of the "shrunk" range is not negative, they are contiguous or overlapping
            uint32_t merged_end = min(vertex_data_end[i], vertex_data_end[j]);
            uint32_t merged_start = max(vertex_data_start[i], vertex_data_start[j]);
            if (merged_end >= merged_start) 
            {
				if (p3x_verbose) printf("P3X: Merging stream %d [0x%x-0x%x] into stream %d [0x%x-0x%x].\n", i, vertex_data_start[i], vertex_data_end[i], j, vertex_data_start[j], vertex_data_end[j]);
                // streams are contiguous or overlapping, merge i into j
                vertex_data_start[j] = min(vertex_data_start[i], vertex_data_start[j]);
                vertex_data_end[j] = max(vertex_data_end[i], vertex_data_end[j]);
                // delete the old one
                vertex_data_start[i] = 0;
                vertex_data_end[i] = 0;
                break;
            }
        }
    }
    // write remaining streams to file
    for (uint32_t i = 0; i < 16; i++) 
    {
        if ((vertex_data_end[i]-vertex_data_start[i]) <= 0)
            continue;
		if (p3x_verbose) printf("P3X: Writing vertex stream %d at 0x%x.  size=%d\n", i, vertex_data_start[i], vertex_data_end[i]-vertex_data_start[i]);
        p3x_write_block((void*)vertex_data_start[i], vertex_data_end[i]-vertex_data_start[i]);
    }

    // write textures
    for (uint32_t i = 0; i < 16; i++) 
    {
        uint32_t control = p3x_get_cache(CELL_GCM_NV4097_SET_TEXTURE_CONTROL0 + i*32);
		if ((control & (1<<31)) == 0) // enabled?
			continue;

		uint32_t format_reg = p3x_get_cache(CELL_GCM_NV4097_SET_TEXTURE_FORMAT + i*32);
		uint32_t offset_reg = p3x_get_cache(CELL_GCM_NV4097_SET_TEXTURE_OFFSET + i*32);
		uint32_t rect_reg = p3x_get_cache(CELL_GCM_NV4097_SET_TEXTURE_IMAGE_RECT + i*32);
		uint32_t control3_reg = p3x_get_cache(CELL_GCM_NV4097_SET_TEXTURE_CONTROL3 + i*4);

    uint8_t* tex_addr = (uint8_t*)p3x_get_address(((format_reg&0x3)-1), offset_reg);

		uint32_t format = (format_reg>>8)&0xff;
		uint32_t ln = format & CELL_GCM_TEXTURE_LN;
		format &= ~(CELL_GCM_TEXTURE_LN|CELL_GCM_TEXTURE_UN);

		uint32_t orig_width  = (rect_reg>>16);
		uint32_t orig_height = (rect_reg&0xffff);
		uint32_t depth = (control3_reg>>20);

		uint32_t mipmap = (format_reg>>16);

		uint32_t cubemap = (format_reg>>2)&0x1;
		if (cubemap)
			depth = 6;

		uint32_t width = orig_width;
		uint32_t height = orig_height;

		if ((format == CELL_GCM_TEXTURE_COMPRESSED_DXT1) || 
			(format == CELL_GCM_TEXTURE_COMPRESSED_DXT23) || 
			(format == CELL_GCM_TEXTURE_COMPRESSED_DXT45))
		{
			width = max(1, (width+3)/4);
			height = max(1, (height+3)/4);
		}
		if ((format == CELL_GCM_TEXTURE_COMPRESSED_R8B8_R8G8) || (format == CELL_GCM_TEXTURE_COMPRESSED_B8R8_G8R8))
		{
			width = max(1, (width+1)/2);
		}

		uint32_t bpp = 0;
		for (uint32_t f = 0; f < sizeof(rsx_textureformat_size)/8; f++)
			if (rsx_textureformat_size[2*f+0] == format)
			{
				bpp = rsx_textureformat_size[2*f+1];
				break;
			}

		uint32_t tex_size = 0;

		for (int d = depth-1; d >= 0; d--)
		{
			for (uint32_t m = 0; m < mipmap; m++)
			{
				if (p3x_skip_mip && depth == 1 && mipmap > 1 && m == 0)
					tex_addr += width * height * bpp;
				else
					tex_size += width * height * bpp;
				if (!ln) width = max(1, width/2);
				height = max(1, height/2);
			}
			if (d && !ln) tex_size = (tex_size+127) & ~127;
		}

		if (p3x_verbose) printf("P3X: Writing texture %d at %p.  size=%d format=%d bpp=%d width=%d height=%d depth=%d mipmap=%d ln=%d\n", 
								i, tex_addr, tex_size, format, bpp, orig_width, orig_height, depth, mipmap, ln);

		p3x_write_block(tex_addr, tex_size);
    }
	if (p3x_verbose) printf("P3X: Finsihed writing event.\n\n");
}

char p3x_event_name[256];
uint32_t p3x_event_is_draw = 0;

void p3x_process_method(uint32_t method, uint32_t data, uint32_t* cmdp)
{
    // track register state
    rsx_state_valid[method/4] = 1;
    rsx_state_cache[method/4] = data;

	if (p3x_verbose>=2) printf("P3X: Method 0x%x with value 0x%x.\n", method, data);

    // track arguments to draw calls (range of indices referenced, for buffer captures)
    if (method == CELL_GCM_NV4097_DRAW_INDEX_ARRAY) 
    {
        p3x_draw_type = method;
        uint32_t start = data&0xffffff;
        uint32_t count = (data>>24)+1;
		p3x_draw_min_index = min(p3x_draw_min_index, start);
		p3x_draw_max_index = max(p3x_draw_max_index, start+count-1);
		if (p3x_verbose) printf("P3X: Encountered DrawIndexArray method, recording index range.  start=%d count=%d minIndex=%d maxIndex=%d\n", start, count, p3x_draw_min_index, p3x_draw_max_index);
    }
    else if (method == CELL_GCM_NV4097_DRAW_ARRAYS) 
    {
        p3x_draw_type = method;
        uint32_t start = data&0xffffff;
        uint32_t count = (data>>24)+1;
		if (p3x_verbose) printf("P3X: Encountered DrawArrays method, recording vertex range.  start=%d count=%d\n", start, count);
        p3x_draw_min_vertex = min(p3x_draw_min_vertex, start);
        p3x_draw_max_vertex = max(p3x_draw_max_vertex, start+count-1);
    }
    // used by CELL_GCM_NV4097_INLINE_ARRAY
    else if (method == CELL_GCM_NV4097_ARRAY_ELEMENT16 || method == CELL_GCM_NV4097_ARRAY_ELEMENT32) 
    {
		if (p3x_verbose) printf("P3X: Encountered ArrayElement method, recording vertex range.  data=%d\n", data);
        if (p3x_get_cache(CELL_GCM_NV4097_SET_RESTART_INDEX_ENABLE) &&
            data == p3x_get_cache(CELL_GCM_NV4097_SET_RESTART_INDEX)) 
            return;
        p3x_draw_min_vertex = min(p3x_draw_min_vertex, data);
        p3x_draw_max_vertex = max(p3x_draw_max_vertex, data);
    }
    else if (method == CELL_GCM_NV4097_SET_BEGIN_END) 
    {
        // end events -- quick, capture all the data!
        if (data == 0) 
        {
			if (p3x_verbose) printf("P3X: Encountered SetBeginEnd method type 0 (end), recording a new event.\n");
			strcpy(p3x_event_name, "Draw");
			p3x_event_is_draw = true;
        }
        else
		// begin events -- get ready to learn about the draw event.
        {
			if (p3x_verbose) printf("P3X: Encountered SetBeginEnd method type %d, recording a Draw event.\n", data);
            p3x_draw_type = 0;
            p3x_draw_min_index = ~0;
            p3x_draw_max_index = 0;
            p3x_draw_min_vertex = ~0;
            p3x_draw_max_vertex = 0;
			p3x_begin_ptr = cmdp-8; // rewind BeginEnd and Command.
        }
    }
	else if (method == CELL_GCM_NV4097_CLEAR_SURFACE)
	{
		if (p3x_verbose) printf("P3X: Encountered Clear, recording a new event.\n");
		strcpy(p3x_event_name, "Clear");
		p3x_event_is_draw = false;
	}
	else if (method == CELL_GCM_NV4097_CLEAR_ZCULL_SURFACE)
	{
		if (p3x_verbose) printf("P3X: Encountered ClearZCull, recording a new event.\n");
		strcpy(p3x_event_name, "ClearZCull");
		p3x_event_is_draw = false;
	}
	else if (method == CELL_GCM_NV3089_IMAGE_IN)
	{
		if (p3x_verbose) printf("P3X: Encountered ImageIn, recording a new event.\n");
		strcpy(p3x_event_name, "Copy");
		p3x_event_is_draw = false;
	}
}

void p3x_flush(uint32_t* new_ptr)
{
	if (!p3x_capturing)
		return;

    uint32_t num_iterations = 0;

    uint32_t* cmdp = p3x_prev_current_ptr;
    uint32_t* cmdend = new_ptr;
	if (p3x_verbose) printf("P3X: Flush command buffer parsing the command buffer, starting with location %p and ending at location %p.\n", cmdp, cmdend);

    CellGcmConfig cfg;
    cellGcmGetConfiguration(&cfg);

	while (cmdp != cmdend)
	{
		uint32_t cmd = *cmdp++;
		if (cmd == 0 || (cmd & 0x80000000)) // NOP or Invalid
			continue; 
		if (cmd & CELL_GCM_METHOD_FLAG_JUMP)
        {
			void* dst;
			cellGcmIoOffsetToAddress(cmd & ~CELL_GCM_METHOD_FLAG_JUMP, &dst);
			if (p3x_verbose) printf("P3X: Encountered Jump command from address %p to address %p.  Recording a command buffer move event.\n", cmdp, dst);
            p3x_write_event("Jump", cmdp-1, false);
			cmdp = (uint32_t*)dst;
            p3x_prev_event_ptr = cmdp;
        }
		else if (cmd & CELL_GCM_METHOD_FLAG_CALL)
		{
			void* dst;
			cellGcmIoOffsetToAddress(cmd & ~CELL_GCM_METHOD_FLAG_CALL, &dst);
			if (p3x_verbose) printf("P3X: Encountered Call command from address %p to address %p.  Recording a command buffer move event.\n", cmdp, dst);
            p3x_write_event("Call", cmdp-1, false);
			p3x_callstack_return = cmdp;
			cmdp = (uint32_t*)dst;
            p3x_prev_event_ptr = cmdp;
		}
		else if (cmd & CELL_GCM_METHOD_FLAG_RETURN)
        {
			if (p3x_callstack_return == NULL) 
			{
				printf("P3X: Encountered invalid Return command in command buffer at location %p, would have triggered a command buffer underflow!  Ending command buffer capture early.\n", cmdp);
				p3x_end(cmdp);
				return;
			}
			if (p3x_verbose) printf("P3X: Encountered Return command from address %p to address %p.  Recording a command buffer move event.\n", cmdp, p3x_callstack_return);
            p3x_write_event("Return", cmdp-1, false);
			cmdp = p3x_callstack_return;
            p3x_callstack_return = NULL;
            p3x_prev_event_ptr = cmdp;
        }
		else
		{
			uint32_t increment = (cmd & CELL_GCM_METHOD_FLAG_NON_INCREMENT) ? 0 : 4;
			cmd &= ~CELL_GCM_METHOD_FLAG_NON_INCREMENT;

			uint32_t count = cmd >> CELL_GCM_COUNT_SHIFT;
			uint32_t method = cmd & ((1<<CELL_GCM_COUNT_SHIFT)-1);

			for (uint32_t i = 0; i < count; i++)
			{
				if (cmdp == cmdend) 
				{
					printf("P3X: Encountered end of buffer mid-command.  Continuing capture from this point will not be possible!");
					cmdend = cmdp;
					p3x_end(cmdp);
					return;
				}
                if (++num_iterations >= 10000000) 
                {
                    printf("P3X: Flush exceeded maximum iteration count %d, possible infinite loop in command buffer.  Ending command buffer capture early.\n", num_iterations);
					cmdend = cmdp;
					p3x_end(cmdp);
					return;
                }
				uint32_t data = *cmdp++;
                p3x_process_method(method, data, cmdp);
				method += increment;
			}

			// If event was detected by p3x_process_method, write it to the file.
			if (p3x_event_name[0])
			{
				p3x_write_event(p3x_event_name, cmdp, p3x_event_is_draw);
				p3x_prev_event_ptr = cmdp;
				p3x_event_name[0] = 0;
			}
		}
	}

	if (p3x_verbose) printf("P3X: Flush complete.\n\n");

    p3x_prev_current_ptr = cmdend;
}

void p3x_begin(uint32_t* cmdstart, const char* name)
{
	char path[256];
	if (name)
		snprintf(path, sizeof(path), "%s", name);
	else
	{
		static int count = 0;
		for(;;)
		{
			snprintf( path, sizeof(path), "%s/capture%03d.p3x", SYS_APP_HOME, count );
			FILE* fp = fopen(path, "rb" );
			if( 0 == fp )
				break;
			fclose(fp);
			count++;
		}
	}

    p3x_file = fopen( path, "wb" );
    if( !p3x_file ) 
    {
        printf("P3X: Unable to open '%s' for write.\n", path );
        return;
    }

	cellGcmGetConfiguration(&p3x_cfg);

    p3x_header hdr;
    memset( &hdr, 0, sizeof(p3x_header) );
    hdr.Fourcc = P3X_HEADER_FOURCC;
    hdr.Version = 5;
    hdr.Size = sizeof(p3x_header);

	memcpy(hdr.tile, p3x_tileinfo, sizeof(hdr.tile));
	memcpy(hdr.zcull, p3x_zcullinfo, sizeof(hdr.zcull));

    // write header
    fwrite( &hdr, 1, sizeof(p3x_header), p3x_file );

    printf("P3X: Saving capture to '%s'...\n", path );
	p3x_prev_current_ptr = cmdstart;
	p3x_prev_event_ptr = cmdstart;
	p3x_callstack_return = 0;
    p3x_capturing = 1;
	p3x_event_count = 0;
}

void p3x_end(uint32_t* cmdp)
{
	p3x_flush(cmdp);

	int size = ftell(p3x_file);
    fclose( p3x_file );
    p3x_file = NULL;

	printf("P3X: Capture complete.  Recorded %d events, using %d bytes.\n", p3x_event_count, size );
    p3x_capturing = 0;
}

void p3x_add_named_event(const char* name, uint32_t* cmdp, uint32_t begin)
{
	if (!p3x_file)
		return;

	p3x_named_event evt;
	evt.Fourcc = P3X_FOURCC_NAMED_EVENT;
	evt.Version = 1;

	evt.cmdp = (uint32_t)cmdp;
	evt.type = begin;

	int len = name ? strlen(name)+1 : 0;
	evt.Size = sizeof(p3x_named_event)+len;

	fwrite(&evt, 1, sizeof(p3x_named_event), p3x_file);
	if (name) fwrite(name, 1, len, p3x_file);
}

bool p3x_is_capturing()
{
	return p3x_file != NULL;
}

void p3x_begin_named_event(const char* name, uint32_t* cmdp)
{
	p3x_add_named_event(name, cmdp, P3X_NAME_BEGIN);
}

void p3x_end_named_event(uint32_t* cmdp)
{
	p3x_add_named_event(NULL, cmdp, P3X_NAME_END);
}

void p3x_name_next_event(uint32_t* cmdp, const char* name, ...)
{
	char work[1024];
	va_list arg;
	va_start(arg, name);
	vsnprintf(work, sizeof(work), name, arg);
	va_end(arg);
	p3x_add_named_event(work, cmdp, P3X_NAME_NEXT);
}

#endif // ifndef __SPU__ 

// cpu profiling
#include <cell/atomic.h>
#include <stdlib.h>

p3x_prof_info_t p3x_prof = { 0 };

#ifndef __SPU__ 

#include <stdint.h>
#include <sys/ppu_thread.h>
#include <sys/time_util.h>

p3x_prof_event_t* p3x_prof_event_cur = NULL;
int p3x_prof_frame = 0;

uint64_t p3x_prof_start_time = 0;

#define P3X_MAX_THREADS 128

p3x_prof_thread_t p3x_threads[P3X_MAX_THREADS];

int p3x_thread_count = 0;

FILE* p3x_prof_out = NULL;

p3x_prof_thread_t* p3x_prof_get_thread(uint64_t id)
{
	for (int i = 0; i < p3x_thread_count; i++)
		if (p3x_threads[i].thread_id == id)
			return &p3x_threads[i];
	int idx = cellAtomicIncr32((uint32_t*)&p3x_thread_count);
	if (idx > P3X_MAX_THREADS)
	{
		cellAtomicStore32((uint32_t*)&p3x_thread_count, P3X_MAX_THREADS);
		return NULL;
	}
	p3x_prof_thread_t* thr = &p3x_threads[idx];
	thr->thread_id = id;
	thr->stack_level = 0;
	memset(thr->stack, 0, sizeof(thr->stack));
	snprintf(thr->name, sizeof(thr->name), "thread 0x%llx", id);
	return thr;
}

void p3x_prof_set_thread_name(uint64_t id, const char* name)
{
	p3x_prof_thread_t* thr = p3x_prof_get_thread(id);
	snprintf(thr->name, sizeof(thr->name), "%s", name);
}

void p3x_prof_begin()
{
	p3x_prof_frame = 0;
	p3x_prof_event_cur = p3x_prof.event_buffer;
	SYS_TIMEBASE_GET(p3x_prof_start_time);
	p3x_mark_frame();
}

void p3x_prof_capture(const char* name)
{
	char path[256];
	if (name)
		snprintf(path, sizeof(path), "%s", name);
	else
	{
		static int count = 0;
		for(;;)
		{
			snprintf( path, sizeof(path), "%s/User/capture%03d.c3x", SYS_APP_HOME, count );
			FILE* fp = fopen(path, "r" );
			if( 0 == fp )
				break;
			fclose(fp);
			count++;
		}
	}

	printf("P3X: Saving CPU capture to '%s'...\n", path );

	p3x_prof_out = fopen( path, "w" );
	if( !p3x_prof_out ) 
	{
		printf("P3X: Unable to open '%s' for write.\n", path );
		return;
	}

	fprintf(p3x_prof_out, "#,Thread,Level,Start,Stop,Time,Description,Color\n");

	// init stacks
	for (int i = 0; i < p3x_thread_count; i++)
	{
		p3x_threads[i].stack_level = 0;
		memset(p3x_threads[i].stack, 0, sizeof(p3x_threads[i].stack));
	}

	// freeze the end pointers in case some threads are still trying to write to the buffer.
	p3x_prof_event_t* event_last = (p3x_prof_event_t*)cellAtomicAdd32((uint32_t*)&p3x_prof_event_cur, sizeof(p3x_prof_event_t));

	// init last event to maximum tick value, to simplify logic.
	event_last->tick = 0xffffffffffffffffULL;

	// fixup timestamps for spu batches first.
	// SPU threads have their tick bits reversed and are only 32bits.
	p3x_prof_start_time &= 0xffffffff;
	for (p3x_prof_event_t* evt = p3x_prof.event_buffer; evt < event_last; evt++)
	{
		if (evt->thread_id & 0x80000000)
			evt->tick = 0xffffffff - evt->tick;
		evt->tick = (evt->tick & 0xffffffff) - p3x_prof_start_time;
	}

	// treat begin and end arrays as a single sorted array, merging by timestamp as we iterate.
	p3x_prof_event_t* event_ptr = p3x_prof.event_buffer;

	for (; event_ptr < event_last; event_ptr++)
	{
		p3x_prof_thread_t* thr = p3x_prof_get_thread(event_ptr->thread_id);
		if (!thr)
			continue;
		if (p3x_verbose) printf("idx=%d thr='%s' stack_level=%d tick=%lld name='%s'\n",
			event_ptr - p3x_prof.event_buffer, thr->name, thr->stack_level, event_ptr->tick, event_ptr->desc);

		if (strncmp(event_ptr->desc, "$frame", 6) == 0)
		{
			// process a frame event.
			// print out the CSV row.
			fprintf(p3x_prof_out, "%d,%s,%d,%llu,%llu,%lld,%s,%08x\n",
				event_ptr-p3x_prof.event_buffer, thr->name, thr->stack_level, 
				event_ptr->tick, event_ptr->tick, 0LL, 
				event_ptr->desc, event_ptr->color);
		}
		else if (strcmp(event_ptr->desc, "$end") == 0)
		{
			// process an end event
			// pop the event off the stack for this thread and print it.
			if (thr->stack_level > 0)
			{
				p3x_prof_event_t* begin_ptr = thr->stack[--thr->stack_level];
				if (begin_ptr)
				{
					if (p3x_verbose) printf("      [idx=%d tick=%llu name='%s']\n",
						begin_ptr - p3x_prof.event_buffer, begin_ptr->tick, begin_ptr->desc);

					// print out the CSV row.
					fprintf(p3x_prof_out, "%d,%s,%d,%llu,%llu,%lld,%s,%08x\n",
						begin_ptr-p3x_prof.event_buffer, thr->name, thr->stack_level, 
						begin_ptr->tick, event_ptr->tick, event_ptr->tick-begin_ptr->tick, 
						begin_ptr->desc, begin_ptr->color);
				}
			}
			thr->stack[thr->stack_level] = NULL;
		}
		else // must be a start event
		{
			// process a begin event
			// just push it on the stack and advance
			if (thr->stack_level < (int)sizeof(thr->stack)/(int)sizeof(thr->stack[0])-1)
				thr->stack[thr->stack_level++] = event_ptr;
		}
	}

	fclose(p3x_prof_out);

	printf("P3X: Capture complete.  Recorded %d events.\n", event_last - p3x_prof.event_buffer);
}

void p3x_prof_init(uint32_t num_events)
{
	if (p3x_prof.event_buffer)
	{
		free(p3x_prof.event_buffer);
		p3x_prof.event_buffer = NULL;
	}
	p3x_prof.buffer_size = num_events;
	if (num_events)
	{
		p3x_prof.event_buffer = (p3x_prof_event_t*)memalign(128, sizeof(p3x_prof_event_t)*num_events);
	}
	p3x_prof_event_cur = p3x_prof.event_buffer;
	p3x_prof.event_cur_ptr = &p3x_prof_event_cur;

  uint64_t id;
	sys_ppu_thread_get_id(&id);
  p3x_prof_set_thread_name(id, "PPU Main");

  p3x_prof_set_thread_name(0x80000000, "SPU 0");
  p3x_prof_set_thread_name(0x80000001, "SPU 1");
  p3x_prof_set_thread_name(0x80000002, "SPU 2");
  p3x_prof_set_thread_name(0x80000003, "SPU 3");
  p3x_prof_set_thread_name(0x80000004, "SPU 4");
  p3x_prof_set_thread_name(0x80000005, "SPU 5");
}

void p3x_begin_named_event(uint32_t color, const char* desc)
{
	if (!p3x_prof.event_buffer)
		return;
		
	p3x_prof_event_t* prof = (p3x_prof_event_t*)cellAtomicAdd32((uint32_t*)p3x_prof.event_cur_ptr, sizeof(p3x_prof_event_t));
	if (prof >= p3x_prof.event_buffer + p3x_prof.buffer_size)
	{
		cellAtomicStore32((uint32_t*)p3x_prof.event_cur_ptr, (uint32_t)(p3x_prof.event_buffer + p3x_prof.buffer_size));
		return;
	}
	sys_ppu_thread_get_id(&prof->thread_id);
	prof->color = color;
	snprintf(prof->desc, sizeof(prof->desc), "%s", desc);
	SYS_TIMEBASE_GET(prof->tick);
}

void p3x_end_named_event()
{
	p3x_begin_named_event(0, "$end");
}

void p3x_mark_frame()
{
	char work[40];
	snprintf(work, sizeof(work), "$frame%d", p3x_prof_frame);
	p3x_begin_named_event(0, work);
	p3x_prof_frame++;
}

void* p3x_prof_get_info_ptr()
{
	return &p3x_prof;
}

#else // #ifndef __SPU__ 

#include <cell/dma.h>
#include <cell/spurs.h>

// temp hack
#include "jobqueue.h"
#define p3x_prof (_jqThisWorker->P3XInfo)

void p3x_begin_named_event(uint32_t color, const char* desc)
{
	uint32_t buf[32] __attribute__((aligned(128)));
	p3x_prof_event_t* prof_ptr = (p3x_prof_event_t*)cellAtomicAdd32(buf, (uint64_t)p3x_prof.event_cur_ptr, sizeof(p3x_prof_event_t));
	if (prof_ptr >= p3x_prof.event_buffer + p3x_prof.buffer_size)
	{
		cellAtomicStore32(buf, (uint64_t)p3x_prof.event_cur_ptr, (uint32_t)(p3x_prof.event_buffer + p3x_prof.buffer_size));
		return;
	}
	p3x_prof_event_t prof __attribute__((aligned(16)));
	__builtin_memcpy(prof.desc, desc, sizeof(prof.desc));
	prof.thread_id = cellSpursGetCurrentSpuId()|0x80000000;
	prof.color = color;
	prof.tick = spu_read_decrementer();
	cellDmaPut(&prof, (uint64_t)prof_ptr, sizeof(prof), 31, 0, 0);
	cellDmaWaitTagStatusAll(1<<31);
}

void p3x_end_named_event()
{
	p3x_begin_named_event(0, "$end");
}

#undef p3x_prof

#endif // __SPU__

#ifndef LIBP3X_H
#define LIBP3X_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void p3x_begin(uint32_t* current, const char* name = NULL);
void p3x_end(uint32_t* current);
void p3x_flush(uint32_t* current);
bool p3x_is_capturing();

void p3x_name_next_event(uint32_t* cmdp, const char* name, ...);

struct p3x_tile_setting
{
	uint32_t location;
	uint32_t offset;
	uint32_t size;
	uint32_t pitch;
	uint32_t compmode;
	uint32_t base;
	uint32_t bank;
};

struct p3x_zcull_setting
{
	uint32_t offset;
	uint32_t width;
	uint32_t height;
	uint32_t cullstart;
	uint32_t zformat;
	uint32_t aaformat;
	uint32_t zculldir;
	uint32_t zcullformat;
	uint32_t sFunc;
	uint32_t sRef;
	uint32_t sMask;
};

enum
{
	P3X_TILE_COUNT = 14,
	P3X_ZCULL_COUNT = 11,
};

extern p3x_tile_setting p3x_tileinfo[P3X_TILE_COUNT]; // 14*7=98 dwords
extern p3x_zcull_setting p3x_zcullinfo[P3X_ZCULL_COUNT]; // 8*11=88 dwords

// cpu+spu profiling
struct p3x_prof_event_t
{
	uint64_t thread_id;
	uint64_t tick;
	uint32_t color;
	uint32_t pad[3];
	char desc[96];
} __attribute__((aligned(16)));

struct p3x_prof_thread_t
{
	uint64_t thread_id;
	char name[120];
	int stack_level;
	p3x_prof_event_t* stack[31];
} __attribute__((aligned(16)));

struct p3x_prof_info_t
{
	p3x_prof_event_t* event_buffer;
	p3x_prof_event_t** event_cur_ptr;
	int buffer_size;
	uint32_t pad;
} __attribute__((aligned(16)));

void p3x_begin_named_event(uint32_t color, const char* desc);
void p3x_end_named_event();
void p3x_mark_frame();

void* p3x_prof_get_info_ptr();

void p3x_prof_init(uint32_t num_events);
void p3x_prof_set_thread_name(uint64_t id, const char* name);

void p3x_prof_begin();
void p3x_prof_capture(const char* name=NULL);

#ifdef __cplusplus
};
#endif

#endif

/**
	ali.h - A single header file consisting of things the C std lib is missing.
	For now, it contains:
		- some util stuff (ali_util)
		- some simple types and aliases (ali_types)
		- logging (ali_log)
		- dynamic array (ali_da)
		- temp allocator (ali_temp_alloc)
		- arena (ali_arena)
		- utf8 (ali_utf8)
		- string view (ali_sv)
		- string builder (ali_sb)
		- measure code (ali_measure)
		- math (ali_math)
		- random numbers (ali_rand)

	This is a stb-style header file, which means you use this file like it's a normal
	header file, ex.:
		#include "ali.h"
	To include implementations, do this in ONE of your translation units:
		#define ALI_IMPLEMENTATION
		#include "ali.h"

	ali.h also supports removing the ali_* prefix of functions and macros by defining ALI_REMOVE_PREFIX.

	License:
	MIT License
	Copyright 2024 spiel_meister <soviczan7@gmail.com>

	Permission is hereby granted, free of charge, to any person obtaining a copy of this
	software and associated documentation files (the "Software"), to deal in the Software
	without restriction, including without limitation the rights to use, copy, modify,
	merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
	permit persons to whom the Software is furnished to do so.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
	PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
	OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef ALI_H_
#define ALI_H_
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define ALI_VERSION "0.1.0"

// Customizable functions
#ifndef ALI_MALLOC
#include <stdlib.h>
#define ALI_MALLOC malloc
#endif // ALI_MALLOC

#ifndef ALI_REALLOC
#include <stdlib.h>
#define ALI_REALLOC realloc
#endif // ALI_REALLOC

#ifndef ALI_FREE
#include <stdlib.h>
#define ALI_FREE free
#endif // ALI_FREE

#ifndef ALI_ASSERT
#include <assert.h>
#define ALI_ASSERT assert
#endif // ALI_ASSERT

#ifndef ALI_MEMCPY
#include <string.h>
#define ALI_MEMCPY memcpy
#endif // ALI_MEMCPY

#ifndef ALI_ABORT
#include <stdlib.h>
#define ALI_ABORT abort
#endif // ALI_ABORT

// @module ali_util
#define ALI_ARRAY_LEN(arr) (sizeof(arr)/sizeof((arr)[0]))
#define ALI_INLINE_ARRAY(Type, ...) ( (Type[]) { __VA_ARGS__ } )

#define ALI_UNUSED(thing) (void)(thing)
#define ALI_UNREACHABLE() do { fprintf(stderr, "%s:%d: UNREACABLE\n", __FILE__, __LINE__); ALI_ABORT(); } while (0)
#define ALI_TODO() do { fprintf(stderr, "%s:%d: TODO: %s not implemented\n", __FILE__, __LINE__, __func__); ALI_ABORT(); } while (0)
#define ALI_PANIC(...) do { fprintf(stderr, __VA_ARGS__); ALI_ABORT(); } while (0)

#define ALI_SWAP(Type, a, b) do { \
		Type __tmp = *(a); \
		*(a) = *(b); \
		*(b) = (__tmp); \
	} while (0)

#define ALI_RETURN_DEFER(value) do { result = value; goto defer } while (0)

#define ALI_FORMAT_ATTRIBUTE(str, vstart) __attribute__((__format__(printf, str, vstart)))

// 'path/to/file.c' -> 'file.c', '/path/to/dir' -> 'dir'
const char* ali_path_name(const char* path);

// @module ali_util end

// @module ali_types
typedef uint8_t  ali_u8;
typedef uint16_t ali_u16;
typedef uint32_t ali_u32;
typedef uint64_t ali_u64;

typedef int8_t  ali_i8;
typedef int16_t ali_i16;
typedef int32_t ali_i32;
typedef int64_t ali_i64;

typedef float ali_f32;

#ifdef __x86_64__
typedef double ali_f64;
#endif // __x86_64__

typedef ali_u64 ali_usize;
typedef ali_i64 ali_isize;
// @module ali_types end

// @module ali_log
#ifndef ALI_NO_LOG
typedef enum {
	LOG_INFO = 0,
	LOG_WARN,
	LOG_ERROR,

	LOG_COUNT_
}AliLogLevel;

extern FILE* ali_global_logfile;
extern AliLogLevel ali_global_loglevel;

void ali_init_global_log();

ALI_FORMAT_ATTRIBUTE(2, 3)
void ali_log_log(AliLogLevel level, const char* fmt, ...);

#define ali_log_info(...) ali_log_log(LOG_INFO, __VA_ARGS__)
#define ali_log_warn(...) ali_log_log(LOG_WARN, __VA_ARGS__)
#define ali_log_error(...) ali_log_log(LOG_ERROR, __VA_ARGS__)

#else // ALI_LOG_END
#define ali_log_info printf
#define ali_log_warn printf
#define ali_log_error(...) fprintf(stderr, __VA_ARGS__)
#endif // ALI_NO_LOG
// @module ali_log end

// @module ali_da
#ifndef ALI_DA_DEFAULT_INIT_CAPACITY
#define ALI_DA_DEFAULT_INIT_CAPACITY 8
#endif // ALI_DA_DEFAULT_INIT_CAPACITY

typedef struct {
	size_t count;
	size_t capacity;
	ali_u8 data[];
}AliDaHeader;

AliDaHeader* ali_da_new_header_with_size(size_t init_capacity, size_t item_size);
AliDaHeader* ali_da_get_header_with_size(void* da, size_t item_size);
void* ali_da_maybe_resize_with_size(void* da, size_t to_add, size_t item_size);
void* ali_da_free_with_size(void* da, size_t item_size);

#define ali_da_new_header(init_capacity) ali_da_new_header_with_size(da, init_capacity, sizeof(*(da)))
#define ali_da_get_header(da) ali_da_get_header_with_size(da, sizeof(*(da)))
#define ali_da_maybe_resize(da, to_add) ali_da_maybe_resize_with_size(da, to_add, sizeof(*(da)))
#define ali_da_getlen(da) ((da) == NULL ? 0 : ali_da_get_header(da)->count)

#define ali_da_reset(da) (ALI_ASSERT((da) != NULL), ali_da_get_header(da)->count = 0)

#define ali_da_append(da, item) ((da) = ali_da_maybe_resize(da, 1), (da)[ali_da_get_header_with_size(da, sizeof(*(da)))->count++] = item)

#define ali_da_free(da) ((da) = ali_da_free_with_size(da, sizeof(*(da))))
#define ali_da_remove_unordered(da, i) (ALI_ASSERT(i >= 0), (da)[i] = (da)[--ali_da_get_header(da)->count])
#define ali_da_remove_ordered(da, i) (ALI_ASSERT(i >= 0), memmove(da + i, da + i + 1, (ali_da_get_header(da)->count - i - 1) * sizeof(*(da))), ali_da_get_header(da)->count--)

#define ali_da_for(da, iter_name) for (size_t iter_name = 0; iter_name < ali_da_getlen(da); ++iter_name)
#define ali_da_foreach(da, Type, iter_name) for (Type* iter_name = da; iter_name < da + ali_da_getlen(da); ++iter_name)

// @module ali_da end

// @module ali_temp_alloc
#ifndef ALI_TEMP_BUF_SIZE
#define ALI_TEMP_BUF_SIZE (8 << 20)
#endif // ALI_TEMP_BUF_SIZE

void* ali_temp_alloc(size_t size);
ALI_FORMAT_ATTRIBUTE(1, 2) char* ali_temp_sprintf(const char* fmt, ...);
size_t ali_temp_stamp(void);
void ali_temp_rewind(size_t stamp);
void ali_temp_reset(void);

void* ali_temp_memdup(void* data, ali_usize data_size);

// @module ali_temp_alloc end

// @module ali_arena 
#ifndef ALI_REGION_DEFAULT_CAP
#define ALI_REGION_DEFAULT_CAP (4 << 10)
#endif // ALI_REGION_DEFAULT_CAP

typedef struct AliRegion {
	size_t count;
	size_t capacity;
	struct AliRegion* next;

	ali_u8 data[];
}AliRegion;

typedef struct {
	AliRegion *start, *end;
}AliArena;

typedef struct {
	AliRegion* r;
	size_t count;
}AliArenaMark;

AliRegion* ali_region_new(size_t capacity);
void* ali_region_alloc(AliRegion* self, size_t size);

void* ali_arena_alloc(AliArena* self, size_t size);
void* ali_arena_memdup(AliArena* self, const void* mem, size_t size_bytes);
char* ali_arena_strdup(AliArena* self, const char* cstr);

ALI_FORMAT_ATTRIBUTE(2, 3)
char* ali_arena_sprintf(AliArena* self, const char* fmt, ...);

AliArenaMark ali_arena_mark(AliArena* self);
void ali_arena_rollback(AliArena* self, AliArenaMark mark);
void ali_arena_reset(AliArena* self);
void ali_arena_free(AliArena* self);
// @module ali_arena end

// @module ali_utf8
typedef ali_u8 ali_utf8;
typedef ali_u32 ali_utf8codepoint;

#define ALI_UTF8(cstr) (ali_utf8*)(cstr)

size_t ali_utf8len(const ali_utf8* utf8);
ali_utf8codepoint ali_utf8c_to_codepoint(const ali_utf8* utf8c, size_t* codepoint_size);
ali_utf8codepoint* ali_utf8_to_codepoints(const ali_utf8* utf8, size_t* count);

bool ali_is_codepoint_valid(ali_utf8codepoint codepoint);
size_t ali_codepoint_size(ali_utf8codepoint codepoint);
const ali_utf8* ali_codepoint_to_utf8(ali_utf8codepoint codepoint);
ali_utf8* ali_codepoints_to_utf8(ali_utf8codepoint* codepoints, size_t len);

ali_utf8codepoint* ali_temp_utf8_to_codepoints(const ali_utf8* utf8, size_t* count);
ali_utf8* ali_temp_codepoints_to_utf8(ali_utf8codepoint* codepoints, size_t len);

#define ali_free_utf8 ALI_FREE
#define ali_free_codepoints ALI_FREE

// @module ali_utf8 end

// @module ali_sv
typedef struct {
	char* start;
	size_t len;
}AliSv;

#define ALI_SV_FMT "%.*s"
#define ALI_SV_F(sv) (int)(sv).len, (sv).start

#define ali_sv_from_cstr(cstr) ((AliSv) { .start = cstr, .len = strlen(cstr) })
#define ali_sv_from_parts(start_, len_) ((AliSv) { .start = start_, .len = len_ })

void ali_sv_step(AliSv* self);
AliSv ali_sv_trim_left(AliSv self);
AliSv ali_sv_trim_right(AliSv self);
AliSv ali_sv_trim(AliSv self);

AliSv ali_sv_chop_left(AliSv* self, size_t n);
AliSv ali_sv_chop_right(AliSv* self, size_t n);
AliSv ali_sv_chop_by_c(AliSv* self, char c);

bool ali_sv_chop_long(AliSv* self, int base, long* out);
bool ali_sv_chop_float(AliSv* self, float* out);
bool ali_sv_chop_double(AliSv* self, double* out);

bool ali_sv_eq(AliSv left, AliSv right);
bool ali_sv_starts_with(AliSv self, AliSv prefix);
bool ali_sv_ends_with(AliSv self, AliSv suffix);

char* ali_temp_sv_to_cstr(AliSv sv);

// @module ali_sv end

// @module ali_sb
typedef struct {
	char* data;
	size_t count;
	size_t capacity;
}AliSb;

void ali_sb_maybe_resize(AliSb* self, size_t to_add);
void ali_sb_push_strs_null(AliSb* self, ...);
ALI_FORMAT_ATTRIBUTE(2, 3)
void ali_sb_push_sprintf(AliSb* self, const char* fmt, ...);
void ali_sb_free(AliSb* self);

bool ali_sb_read_file(AliSb* self, const char* path);
bool ali_sb_write_file(AliSb* self, const char* path);

#define ali_sb_push_strs(...) ali_sb_push_strs_null(__VA_ARGS__, NULL)

#define ali_sb_to_sv(sb) ali_sv_from_parts((sb).items, (sb).count)

// @module ali_sb end

// @module ali_measure
double ali_get_now();

#ifndef ALI_MEASUREMENTS_COUNT
#define ALI_MEASUREMENTS_COUNT 1024
#endif // ALI_MEASUREMENTS_COUNT

void ali_measure_start(const char* name);
void ali_measure_end(const char* name);

void ali_print_measurements(void);
// @module ali_measure end

// @module ali_math

ali_f32 ali_lerpf(ali_f32 a, ali_f32 b, ali_f32 t);
ali_f32 ali_normalizef(ali_f32 start, ali_f32 end, ali_f32 value);
ali_u64 ali_rotl64(ali_u64 x, ali_u8 k);

ali_f32 ali_quadbezierf(ali_f32 start, ali_f32 end, ali_f32 control, ali_f32 t);
ali_f32 ali_cubebezierf(ali_f32 start, ali_f32 end, ali_f32 control1, ali_f32 control2, ali_f32 t);
// @module ali_math end

// @module ali_rand

typedef struct {
	ali_u64 state[4];
}AliXoshiro256ppState;

ali_u64 ali_xoshiro256pp_next(AliXoshiro256ppState *state);
void ali_xoshiro256pp_seed(AliXoshiro256ppState *state, ali_u64 seed[4]);

ali_u64 ali_rand();
ali_u64* ali_temp_rand_sequence(ali_usize count);
ali_f64 ali_rand_float();
ali_u64 ali_rand_range(ali_u64 min, ali_u64 max);

// @module ali_rand end

#endif // ALI_H_

#ifdef ALI_IMPLEMENTATION
#undef ALI_IMPLEMENTATION
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>

// @module ali_util

const char* ali_path_name(const char* path) {
	const char* slash = strrchr(path, '/');
	return slash != NULL ? slash + 1 : path;
}

// @module ali_util end

// @module ali_log
#ifndef ALI_NO_LOG

static_assert(LOG_COUNT_ == 3, "Log level was added");
const char* loglevel_to_str[LOG_COUNT_] = {
	[LOG_INFO] = "INFO",
	[LOG_WARN] = "WARN",
	[LOG_ERROR] = "ERROR",
};

FILE* ali_global_logfile = NULL;
AliLogLevel ali_global_loglevel = LOG_INFO;

void ali_log_log(AliLogLevel level, const char* fmt, ...) {
	if (ali_global_logfile == NULL) ali_global_logfile = stdout;

	va_list args;
	va_start(args, fmt);

	if (ali_global_loglevel >= level) {
		fprintf(ali_global_logfile, "[%s] ", loglevel_to_str[level]);
		vfprintf(ali_global_logfile, fmt, args);
		fprintf(ali_global_logfile, "\n");
	}

	va_end(args);
}
#endif // ALI_LOG_END
// @module ali_log end

// @module ali_da
AliDaHeader* ali_da_new_header_with_size(size_t init_capacity, size_t item_size) {
	AliDaHeader* h = ALI_MALLOC(sizeof(*h) + item_size * init_capacity);
	h->count = 0;
	h->capacity = init_capacity;
	return h;
}

AliDaHeader* ali_da_get_header_with_size(void* da, size_t item_size) {
	if (da == NULL) return ali_da_new_header_with_size(ALI_DA_DEFAULT_INIT_CAPACITY, item_size);
	
	return (AliDaHeader*)da - 1;
}

void* ali_da_maybe_resize_with_size(void* da, size_t to_add, size_t item_size) {
	AliDaHeader* h = ali_da_get_header_with_size(da, item_size);
	
	if (h->count + to_add >= h->capacity) {
		while (h->count + to_add >= h->capacity) {
			if (h->capacity == 0) h->capacity = ALI_DA_DEFAULT_INIT_CAPACITY;
			else h->capacity *= 2;
		}

		h = ALI_REALLOC(h, sizeof(*h) + h->capacity * item_size);
	}

	return h->data;
}

void* ali_da_free_with_size(void* da, size_t item_size) {
	AliDaHeader* h = ali_da_get_header_with_size(da, item_size);
	ALI_FREE(h);
	return (da = NULL);
}

// @module ali_da end

// @module ali_temp_alloc

static ali_u8 ali_temp_buffer[ALI_TEMP_BUF_SIZE] = {0};
static size_t ali_temp_buffer_size = 0;

void* ali_temp_alloc(size_t size) {
	ALI_ASSERT(ali_temp_buffer_size + size < ALI_TEMP_BUF_SIZE);
	void* ptr = ali_temp_buffer + ali_temp_buffer_size;
	ali_temp_buffer_size += size;
	return ptr;
}

void* ali_temp_memdup(void* data, ali_usize data_size) {
	void* mem = ali_temp_alloc(data_size);
	ALI_MEMCPY(mem, data, data_size);
	return mem;
}

char* ali_temp_strdup(const char* str) {
	size_t size = strlen(str) + 1;
	char* copy = ali_temp_alloc(size);
	return ALI_MEMCPY(copy, str, size);
}

char* ali_temp_sprintf(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int n = vsnprintf(NULL, 0, fmt, args);
	va_end(args);

	char* out = ali_temp_alloc(n + 1);
	va_start(args, fmt);
	vsnprintf(out, n + 1, fmt, args);
	va_end(args);

	return out;
}

size_t ali_temp_stamp(void) {
	return ali_temp_buffer_size;
}

void ali_temp_rewind(size_t stamp) {
	ali_temp_buffer_size = stamp;
}

void ali_temp_reset(void) {
	ali_temp_buffer_size = 0;
}

// @module ali_temp_alloc end

// @module ali_arena

AliRegion* ali_region_new(size_t capacity) {
	AliRegion* new = ALI_MALLOC(sizeof(*new) + capacity);
	ALI_ASSERT(new != NULL);
	new->count = 0;
	new->capacity = capacity;
	new->next = NULL;
	return new;
}

void* ali_region_alloc(AliRegion* self, size_t size) {
	if (self->count + size >= self->capacity) return NULL;
	void* ptr = self->data + self->count;
	self->count += size;
	return ptr;
}

void* ali_arena_alloc(AliArena* self, size_t size) {
	if (self->start == NULL) {
		self->start = ali_region_new(ALI_REGION_DEFAULT_CAP);
		self->end = self->start;
	}

	AliRegion* region = self->end;
	void* ptr;
	do {
		ptr = ali_region_alloc(region, size);
		if (ptr == NULL) {
			if (region->next == NULL) {
				region->next = ali_region_new(ALI_REGION_DEFAULT_CAP);
			}
			region = region->next;
		}
	} while (ptr == NULL);
	self->end = region;

	return ptr;
}

void* ali_arena_memdup(AliArena* self, const void* mem, size_t size_bytes) {
	void* data = ali_arena_alloc(self, size_bytes);
	ALI_MEMCPY(data, mem, size_bytes);
	return data;
}

char* ali_arena_strdup(AliArena* self, const char* cstr) {
	return ali_arena_memdup(self, cstr, strlen(cstr) + 1);
}

char* ali_arena_sprintf(AliArena* self, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char* out;
	ALI_ASSERT(vasprintf(&out, fmt, args) >= 0);
	char* real_out = ali_arena_strdup(self, out);
	ALI_FREE(out);

	va_end(args);
	return real_out;
}

void ali_arena_reset(AliArena* self) {
	for (AliRegion* r = self->start; r != NULL; r = r->next) {
		r->count = 0;
	}
	self->end = self->start;
}

void ali_arena_free(AliArena* self) {
	while (self->start != NULL) {
		AliRegion* next = self->start->next;
		ALI_FREE(self->start);
		self->start = next;
	}
	self->end = NULL;
}

AliArenaMark ali_arena_mark(AliArena* self) {
	return (AliArenaMark) { self->end, self->end->count };
}

void ali_arena_rollback(AliArena* self, AliArenaMark mark) {
	if (mark.r == NULL) {
		ali_arena_reset(self);
		return;
	}

	mark.r->count = mark.count;
	for (AliRegion* r = mark.r; r != NULL; r = r->next) {
		r->count = 0;
	}

	self->end = mark.r;
}

// @module ali_arena end

// @module ali_utf8
size_t ali_utf8len(const ali_utf8* utf8) {
	size_t len = 0;
	while (*utf8 != 0) {
		size_t codepoint_size = 0;
		ali_utf8c_to_codepoint(utf8, &codepoint_size);
		len++;
		utf8 += codepoint_size;
	}
	return len;
}

bool ali_codepoint_is_valid(ali_utf8codepoint codepoint) {
	if (codepoint > 0x10FFFF) return false;
	return true;
}

ali_utf8codepoint ali_utf8c_to_codepoint(const ali_utf8* utf8c, size_t* codepoint_size) {
	ali_utf8codepoint codepoint = 0;
	size_t codepoint_size_ = 0;

	if ((utf8c[0] & 0x80) == 0x00) {
		codepoint = utf8c[0];
		codepoint_size_ = 1;
	} else if ((utf8c[0] & 0xE0) == 0xC0) {
		codepoint = ((utf8c[0] & 0x1F) << 6*1) | ((utf8c[1] & 0x3F) << 6*0);
		codepoint_size_ = 2;
	} else if ((utf8c[0] & 0xF0) == 0xE0) {
		codepoint = ((utf8c[0] & 0x1F) << 6*2) | ((utf8c[1] & 0x3F) << 6*1) | ((utf8c[2] & 0x3F) << 6*0);
		codepoint_size_ = 3;
	} else if ((utf8c[0] & 0xF8) == 0xF0) {
		codepoint = ((utf8c[0] & 0x1F) << 6*3) | ((utf8c[1] & 0x3F) << 6*2) | ((utf8c[2] & 0x3F) << 6*1) | ((utf8c[3] & 0x3F) << 6*0);
		codepoint_size_ = 4;
	} else {
		// invalid
		return -1;
	}

	if (codepoint_size) *codepoint_size = codepoint_size_;
	return codepoint;
}

ali_utf8codepoint* ali_utf8_to_codepoints(const ali_utf8* utf8, size_t* count) {
	size_t len = ali_utf8len(utf8);
	*count = len;

	ali_utf8codepoint* codepoints = ALI_MALLOC(len * sizeof(*codepoints));
	len = 0;
	while (*utf8 != 0) {
		size_t codepoint_size = 0;
		codepoints[len++] = ali_utf8c_to_codepoint(utf8, &codepoint_size);
		utf8 += codepoint_size;
	}

	return codepoints;
}

size_t ali_codepoint_size(ali_utf8codepoint codepoint) {
	if (codepoint > 0x10FFFF) return 0;
	else if (codepoint > 0xFFFF) return 4;
	else if (codepoint > 0x07FF) return 3;
	else if (codepoint > 0x007F) return 2;
	else return 1;
}

const ali_utf8* ali_codepoint_to_utf8(ali_utf8codepoint codepoint) {
static ali_utf8 utf8[5] = {0};
	
	size_t codepoint_size = ali_codepoint_size(codepoint);
	if (codepoint_size == 4) {
		utf8[0] = 0xF0 | ((codepoint >> 6*3) & 0x07);
		utf8[1] = 0x80 | ((codepoint >> 6*2) & 0x3F);
		utf8[2] = 0x80 | ((codepoint >> 6*1) & 0x3F);
		utf8[3] = 0x80 | ((codepoint >> 6*0) & 0x3F);
	} else if (codepoint_size == 3) {
		utf8[0] = 0xE0 | ((codepoint >> 6*2) & 0x0F);
		utf8[1] = 0x80 | ((codepoint >> 6*1) & 0x3F);
		utf8[2] = 0x80 | ((codepoint >> 6*0) & 0x3F);
	} else if (codepoint_size == 2) {
		utf8[0] = 0xC0 | ((codepoint >> 6*1) & 0x1F);
		utf8[1] = 0x80 | ((codepoint >> 6*0) & 0x3F);
	} else if (codepoint_size == 1) {
		utf8[0] = codepoint;
	} else {
		// invalid
		return NULL;
	}
	utf8[codepoint_size] = 0;

	return utf8;
}

ali_utf8* ali_codepoints_to_utf8(ali_utf8codepoint* codepoints, size_t len) {
	size_t real_len = 0;
	for (size_t i = 0; i < len; ++i) {
		real_len += ali_codepoint_size(codepoints[i]);
	}

	ali_utf8* utf8 = ALI_MALLOC(real_len + 1);
	ali_utf8* utf8p = utf8;
	for (size_t i = 0; i < len; ++i) {
		size_t codepoint_size = ali_codepoint_size(codepoints[i]);
		memcpy(utf8p, ali_codepoint_to_utf8(codepoints[i]), codepoint_size);
		utf8p += codepoint_size;
	}
	utf8[real_len] = 0;

	return utf8;
}

ali_utf8codepoint* ali_temp_utf8_to_codepoints(const ali_utf8* utf8, size_t* count) {
	ali_utf8codepoint* codepoints = ali_utf8_to_codepoints(utf8, count);
	ali_utf8codepoint* out = ali_temp_alloc(sizeof(*out) * (*count));
	ALI_MEMCPY(out, codepoints, sizeof(*out) * (*count));
	ali_free_codepoints(codepoints);
	return out;
}

ali_utf8* ali_temp_codepoints_to_utf8(ali_utf8codepoint* codepoints, size_t len) {
	ali_utf8* utf8 = ali_codepoints_to_utf8(codepoints, len);
	ali_utf8* out = (ali_utf8*)ali_temp_strdup((char*)utf8);
	ali_free_utf8(utf8);
	return out;
}

// @module ali_utf8 end

// @module ali_sv

void ali_sv_step(AliSv* self) {
	if (self->start == NULL || self->len == 0) return;
	self->start += 1;
	self->len -= 1;
}

AliSv ali_sv_trim_left(AliSv self) {
	while (self.len > 0 && isspace(*self.start)) {
		ali_sv_step(&self);
	}
	return self;
}

AliSv ali_sv_trim_right(AliSv self) {
	while (self.len > 0 && isspace(self.start[self.len - 1])) {
		self.len -= 1;
	}
	return self;
}

AliSv ali_sv_trim(AliSv self) {
	return ali_sv_trim_left(ali_sv_trim_right(self));
}

AliSv ali_sv_chop_by_c(AliSv* self, char c) {
	AliSv chopped = { .start = self->start, .len = 0 };

	while (*self->start != c) {
		ali_sv_step(self);

		if (self->len == 0) break;
	}
	ali_sv_step(self);

	chopped.len = self->start - chopped.start;
	return chopped;
}

AliSv ali_sv_chop_left(AliSv* self, size_t n) {
	if (self->len < n) n = self->len;

	AliSv chunk = ali_sv_from_parts(self->start, n);
	self->start += n;
	self->len -= n;

	return chunk;
}

AliSv ali_sv_chop_right(AliSv* self, size_t n) {
	if (self->len < n) n = self->len;

	AliSv chunk = ali_sv_from_parts(self->start + self->len - n, n);
	self->len -= n;

	return chunk;
}

bool ali_sv_chop_long(AliSv* self, int base, long* out) {
	char* endptr;
	long num = strtol(self->start, &endptr, base);
	if (endptr == self->start) return false;
	*out = num;

	size_t n = endptr - self->start;
	self->start += n;
	self->len -= n;

	return true;
}

bool ali_sv_chop_float(AliSv* self, float* out) {
	char* endptr;
	float num = strtof(self->start, &endptr);
	if (endptr == self->start) return false;
	*out = num;

	size_t n = endptr - self->start;
	self->start += n;
	self->len -= n;

	return true;
}

bool ali_sv_chop_double(AliSv* self, double* out) {
	char* endptr;
	double num = strtod(self->start, &endptr);
	if (endptr == self->start) return false;
	*out = num;

	size_t n = endptr - self->start;
	self->start += n;
	self->len -= n;

	return true;
}

bool ali_sv_eq(AliSv left, AliSv right) {
	bool eq = left.len == right.len;
	for (size_t i = 0; eq && i < left.len; ++i) {
		eq &= left.start[i] == right.start[i];
	}
	return eq;
}

bool ali_sv_starts_with(AliSv self, AliSv prefix) {
	if (self.len < prefix.len) return false;
	return ali_sv_eq(ali_sv_from_parts(self.start, prefix.len), prefix);
}

bool ali_sv_ends_with(AliSv self, AliSv suffix) {
	if (self.len < suffix.len) return false;
	return ali_sv_eq(ali_sv_from_parts(self.start + self.len - suffix.len, suffix.len), suffix);
}

char* ali_temp_sv_to_cstr(AliSv sv) {
	char* out = ali_temp_alloc(sv.len + 1);
	ALI_MEMCPY(out, sv.start, sv.len);
	out[sv.len] = 0;
	return out;
}

// @module ali_sv end

// @module ali_sb
void ali_sb_maybe_resize(AliSb* self, size_t to_add) {
	if (self->count + to_add >= self->capacity) {
		while (self->count + to_add >= self->capacity) {
			if (self->capacity == 0) self->capacity = ALI_DA_DEFAULT_INIT_CAPACITY;
			else self->capacity *= 2;
		}

		self->data = ALI_REALLOC(self->data, self->capacity);
	}
}

void ali_sb_push_strs_null(AliSb* self, ...) {
	va_list args;
	va_start(args, self);

	const char* str = va_arg(args, const char*);
	while (str != NULL) {
		size_t n = strlen(str);
		ali_sb_maybe_resize(self, n);
		ALI_MEMCPY(self->data + self->count, str, n);
		self->count += n;
		str = va_arg(args, const char*);
	}

	va_end(args);
}

void ali_sb_push_sprintf(AliSb* self, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char* str;
	ALI_ASSERT(vasprintf(&str, fmt, args) == 0);
	ali_sb_push_strs(self, str);
	ALI_FREE(str);

	va_end(args);
}

void ali_sb_free(AliSb* self) {
	ALI_FREE(self->data);
	self->data = NULL;
	self->count = 0;
	self->capacity = 0;
}

bool ali_sb_read_file(AliSb* self, const char* path) {
	FILE* f = fopen(path, "rb");
	if (f == NULL) {
		ali_log_error("Couldn't read %s: %s\n", path, strerror(errno));
		return false;
	}

	fseek(f, 0, SEEK_END);
	size_t n = ftell(f);
	fseek(f, 0, SEEK_SET);

	ali_sb_maybe_resize(self, n);
	fread(self->data + self->count, n, 1, f);
	self->count += n;

	if (f != NULL) fclose(f);
	return true;
}

bool ali_sb_write_file(AliSb* self, const char* path) {
	FILE* f = fopen(path, "wb");
	if (f == NULL) {
		ali_log_error("Couldn't write to %s: %s\n", path, strerror(errno));
		return false;
	}

	fwrite(self->data, 1, self->count, f);

	if (f != NULL) fclose(f);
	return true;
}

// @module ali_sb end

// @module ali_measure

double ali_get_now() {
	struct timespec ts;
	ALI_ASSERT(clock_gettime(CLOCK_REALTIME, &ts) == 0);
	return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

typedef struct {
	const char* name;

	double start;

	double total;
	size_t count;
}Measurement;

static Measurement measurements[ALI_MEASUREMENTS_COUNT] = {0};
static size_t measurements_count = 0;

void ali_measurement_push(Measurement measurement) {
	ALI_ASSERT(measurements_count < ALI_MEASUREMENTS_COUNT);
	measurements[measurements_count++] = measurement;
}

Measurement* ali_find_measurement(const char* name) {
	Measurement* found = NULL;

	for (size_t i = 0; found == NULL && i < measurements_count; ++i) {
		if (strcmp(measurements[i].name, name) == 0) {
			found = &measurements[i];
			break;
		}
	}

	return found;
}

void ali_measure_start(const char* name) {
	Measurement* found = ali_find_measurement(name);
	if (found == NULL) {
		Measurement measurement = { .name = name, .start = ali_get_now() };
		ali_measurement_push(measurement);
		return;
	}

	found->start = ali_get_now();
}

void ali_measure_end(const char* name) {
	Measurement* found = ali_find_measurement(name);
	ALI_ASSERT(found != NULL);

	found->total += ali_get_now() - found->start;
	found->count += 1;
}

void ali_print_measurements(void) {
	for (size_t i = 0; i < measurements_count; ++i) {
		ali_log_info("[ali_measure] %s: %lfs", measurements[i].name, measurements[i].total / measurements[i].count);
	}
}

// @module ali_measure end

// @module ali_math

ali_f32 ali_lerpf(ali_f32 a, ali_f32 b, ali_f32 t) {
	return a + (b - a) * t;
}

ali_f32 ali_normalizef(ali_f32 start, ali_f32 end, ali_f32 value) {
	return (value - start) / (end - start);
}

ali_u64 ali_rotl64(ali_u64 x, ali_u8 k) {
	return (x << k) | (x >> (64 - k));

}
ali_f32 ali_quadbezierf(ali_f32 start, ali_f32 end, ali_f32 control, ali_f32 t) {
	ali_f32 a = ali_lerpf(start, control, t);
	ali_f32 b = ali_lerpf(control, end, t);
	return ali_lerpf(a, b, t);
}

ali_f32 ali_cubebezierf(ali_f32 start, ali_f32 end, ali_f32 control1, ali_f32 control2, ali_f32 t) {
	ali_f32 a = ali_quadbezierf(start, control1, control2, t);
	ali_f32 b = ali_quadbezierf(control1, control2, end, t);
	return ali_lerpf(a, b, t);
}

// @module ali_math end

// @module ali_rand

ali_u64 ali_xoshiro256pp_next(AliXoshiro256ppState *state) {
	ali_u64 *s = state->state;
	ali_u64 const result = ali_rotl64(s[1] * 5, 7) * 9;
	ali_u64 const t = s[1] << 17;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;
	s[3] = ali_rotl64(s[3], 45);

	return result;
}

void ali_xoshiro256pp_seed(AliXoshiro256ppState *state, ali_u64 seed[4]) {
	ALI_MEMCPY(state->state, seed, sizeof(state->state));
}

ali_u64* ali_temp_rand_sequence(ali_usize count) {
	ali_u64* out = ali_temp_alloc(sizeof(*out) * count);
	for (ali_usize i = 0; i < count; ++i) out[i] = ali_rand();
	return out;
}

ali_f64 ali_rand_float() {
    ali_u64 value = ali_rand();
    return (ali_f64)value / (ali_f64)UINT64_MAX;
}

ali_u64 ali_rand_range(ali_u64 min, ali_u64 max) {
    if (min > max) ALI_SWAP(ali_u64, &min, &max);
    return ali_rand() % (max - min) + min;
}

AliXoshiro256ppState xoshiro_global_state = { { 0x96EA83C1, 0x218B21E5, 0xAA91FEBD, 0x976414D4 } };

ali_u64 ali_rand() {
	return ali_xoshiro256pp_next(&xoshiro_global_state);
}

#define ali_srand(seed) ali_xoshiro256pp_seed(&xoshiro_global_state, seed)

// @module ali_rand end

#endif // ALI_IMPLEMENTATION

#ifdef ALI_REMOVE_PREFIX
#undef ALI_REMOVE_PREFIX

// @module ali_util
#define ARRAY_LEN ALI_ARRAY_LEN
#define INLINE_ARRAY ALI_INLINE_ARRAY

#define UNUSED ALI_UNUSED
#define UNREACHABLE ALI_UNREACHABLE
#define TODO ALI_TODO
#define PANIC ALI_PANIC

#define RETURN_DEFER ALI_RETURN_DEFER

#define FORMAT_ATTRIBUTE ALI_FORMAT_ATTRIBUTE

#define SWAP ALI_SWAP

#define path_name ali_path_name
// @module ali_util end

// @module ali_types
#define u8 ali_u8
#define u16 ali_u16
#define u32 ali_u32
#define u64 ali_u64

#define i8 ali_i8
#define i16 ali_i16
#define i32 ali_i32
#define i64 ali_i64

#define f32 ali_f32

#ifdef __x86_64__
#define f64 ali_f64
#endif // __x86_64__

#define usize ali_usize
#define isize ali_isize
// @module ali_types end

// @module ali_log

#define global_logfile ali_global_logfile
#define global_loglevel ali_global_loglevel

#define log_log ali_log_log

#define log_info ali_log_info
#define log_warn ali_log_warn
#define log_error ali_log_error

// @module ali_log end

// @module ali_da
#define da_new_header_with_size ali_da_new_header_with_size
#define da_get_header_with_size ali_da_get_header_with_size
#define da_maybe_resize_with_size ali_da_maybe_resize_with_size
#define da_free_with_size ali_da_free_with_size

#define da_new_header ali_da_new_header
#define da_get_header ali_da_get_header
#define da_maybe_resize ali_da_maybe_resize
#define da_getlen ali_da_getlen

#define da_append ali_da_append
#define da_free ali_da_free

#define da_reset ali_da_reset

#define da_remove_unordered ali_da_remove_unordered
#define da_remove_ordered ali_da_remove_ordered

#define da_for ali_da_for
#define da_foreach ali_da_foreach
// @module ali_da end

// @module ali_temp_alloc

#define temp_alloc ali_temp_alloc
#define temp_memdup ali_temp_memdup
#define temp_sprintf ali_temp_sprintf
#define temp_strdup ali_temp_strdup
#define temp_stamp ali_temp_stamp
#define temp_rewind ali_temp_rewind
#define temp_reset ali_temp_reset

// @module ali_temp_alloc end

// @module ali_arena
#define region_new ali_region_new
#define region_alloc ali_region_alloc

#define arena_alloc ali_arena_alloc
#define arena_memdup ali_arena_memdup
#define arena_strdup ali_arena_strdup
#define arena_sprintf ali_arena_sprintf

#define arena_mark ali_arena_mark
#define arena_rollback ali_arena_rollback
#define arena_reset ali_arena_reset
#define arena_free ali_arena_free
// @module ali_arena end

// @module ali_utf8
// NOTE: we mustn't do this
// #define utf8 ali_utf8
#define utf8codepoint ali_utf8codepoint

#define UTF8 ALI_UTF8

#define utf8len ali_utf8len
#define utf8c_to_codepoint ali_utf8c_to_codepoint
#define utf8_to_codepoints ali_utf8_to_codepoints

#define is_codepoint_valid ali_is_codepoint_valid
#define codepoint_size ali_codepoint_size
#define codepoint_to_utf8 ali_codepoint_to_utf8
#define codepoints_to_utf8 ali_codepoints_to_utf8

#define temp_utf8_to_codepoints ali_temp_utf8_to_codepoints
#define temp_codepoints_to_utf8 ali_temp_codepoints_to_utf8
// @module ali_utf8 end

// @module ali_sv
#define SV_FMT ALI_SV_FMT
#define SV_F ALI_SV_F

#define sv_from_cstr ali_sv_from_cstr
#define sv_from_parts ali_sv_from_parts

#define sv_step ali_sv_step
#define sv_trim_left ali_sv_trim_left
#define sv_trim_right ali_sv_trim_right
#define sv_trim ali_sv_trim

#define sv_chop_left ali_sv_chop_left
#define sv_chop_right ali_sv_chop_right
#define sv_chop_by_c ali_sv_chop_by_c

#define sv_chop_long ali_sv_chop_long
#define sv_chop_float ali_sv_chop_float
#define sv_chop_double ali_sv_chop_double

#define sv_eq ali_sv_eq
#define sv_starts_with ali_sv_starts_with
#define sv_ends_with ali_sv_ends_with

#define temp_sv_to_cstr ali_temp_sv_to_cstr
// @module ali_sv end

// @module ali_sb
#define sb_maybe_resize ali_sb_maybe_resize
#define sb_push_strs_null ali_sb_push_strs_null
#define sb_push_sprintf ali_sb_push_sprintf
#define sb_free ali_sb_free

#define sb_push_strs ali_sb_push_strs
#define sb_to_sv ali_sb_to_sv

#define sb_read_file ali_sb_read_file
#define sb_write_file ali_sb_write_file
// @module ali_sb end

// @module ali_measure
#define measure_start ali_measure_start
#define measure_end ali_measure_end

#define print_measurements ali_print_measurements
// @module ali_measure end

#endif // ALI_REMOVE_PREFIX


#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "defines.h"

void mem_init();

void load_section(u32 mem, u8 * bytes, u32 size);
void dump_section(u32 mem, u8 * bytes, u32 size);

int is_zero(u32 mem);

int memRead8 (u32 mem, u8  *out);
int memRead16(u32 mem, u16 *out);
int memRead32(u32 mem, u32 *out);
int memRead64(u32 mem, u64 *out);
int memRead128(u32 mem, u64 *out);
void memWrite8 (u32 mem, u8  value);
void memWrite16(u32 mem, u16 value);
void memWrite32(u32 mem, u32 value);
void memWrite64(u32 mem, u64 value);
void memWrite128(u32 mem, u64 *value);

void tag_read(u32 mem);
void tag_write(u32 mem);

int has_read(u32 mem);
int has_write(u32 mem);

#endif

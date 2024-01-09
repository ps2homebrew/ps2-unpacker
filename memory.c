#include <stdio.h>
#include "memory.h"
#include "R5900.h"

#define MEMSIZE 32 * 1024 * 1024

void dumpasm(u32);

u8 memory[MEMSIZE];
u8 tags[MEMSIZE];

void mem_init() {
    memset(memory, 0, MEMSIZE);
    memset(tags, 0, MEMSIZE);
}

void load_section(u32 mem, u8 * bytes, u32 size) {
    memcpy(memory + mem, bytes, size);
}

void dump_section(u32 mem, u8 * bytes, u32 size) {
    memcpy(bytes, memory + mem, size);
}

int is_zero(u32 mem) {
    return memory[mem] ? 0 : 1;
}

int memRead8(u32 mem, u8  *out) {
    switch ((mem & 0xf8000000) >> 24) {
    case 0: /* default mem mapping */
	break;
    case 8: /* first copy */
	mem &= 0xf7ffffff;
	break;
    default:
	printf("\nUnknown memory mapping: %08X (reading at %08X)\n", mem & 0xf8000000, mem);
	dumpasm(cpuRegs.pc - 4);
	exit(-1);
    }
    if (mem >= MEMSIZE) {
	printf("\nOut of range memory read: %08X at %08X\n", mem, cpuRegs.pc - 4);
	dumpasm(cpuRegs.pc - 4);
	exit(-1);
    }
    *out = memory[mem];
    tag_read(mem);
    return 0;
}

void memWrite8(u32 mem, u8  value) {
    switch ((mem & 0xf8000000) >> 24) {
    case 0: /* default mem mapping */
	break;
    case 8: /* first copy */
	mem &= 0xf7ffffff;
	break;
    default:
	printf("\nUnknown memory mapping: %08X (writing at %08X)\n", mem & 0xf8000000, mem);
	dumpasm(cpuRegs.pc - 4);
	exit(-1);
    }
    if (mem >= MEMSIZE) {
	printf("\nOut of range memory write: %08X at %08X\n", mem, cpuRegs.pc - 4);
	dumpasm(cpuRegs.pc - 4);
	exit(-1);
    }
    memory[mem] = value;
    tag_write(mem);
}

void tag_read(u32 mem) {
    tags[mem] |= 1;
}

void tag_write(u32 mem) {
    tags[mem] |= 2;
}

int has_read(u32 mem) {
    return tags[mem] & 1 ? 1 : 0;
}

int has_write(u32 mem) {
    return tags[mem] & 2 ? 1 : 0;
}

/* ------- */

int memRead16(u32 mem, u16 *out) {
    memRead8(mem +  0, ((u8 *) out) +  0);
    memRead8(mem +  1, ((u8 *) out) +  1);
    return 0;
}

int memRead32(u32 mem, u32 *out) {
    memRead8(mem +  0, ((u8 *) out) +  0);
    memRead8(mem +  1, ((u8 *) out) +  1);
    memRead8(mem +  2, ((u8 *) out) +  2);
    memRead8(mem +  3, ((u8 *) out) +  3);
    return 0;
}

int memRead64(u32 mem, u64 *out) {
    memRead8(mem +  0, ((u8 *) out) +  0);
    memRead8(mem +  1, ((u8 *) out) +  1);
    memRead8(mem +  2, ((u8 *) out) +  2);
    memRead8(mem +  3, ((u8 *) out) +  3);
    memRead8(mem +  4, ((u8 *) out) +  4);
    memRead8(mem +  5, ((u8 *) out) +  5);
    memRead8(mem +  6, ((u8 *) out) +  6);
    memRead8(mem +  7, ((u8 *) out) +  7);
    return 0;
}

int memRead128(u32 mem, u64 *out) {
    memRead8(mem +  0, ((u8 *) out) +  0);
    memRead8(mem +  1, ((u8 *) out) +  1);
    memRead8(mem +  2, ((u8 *) out) +  2);
    memRead8(mem +  3, ((u8 *) out) +  3);
    memRead8(mem +  4, ((u8 *) out) +  4);
    memRead8(mem +  5, ((u8 *) out) +  5);
    memRead8(mem +  6, ((u8 *) out) +  6);
    memRead8(mem +  7, ((u8 *) out) +  7);
    memRead8(mem +  8, ((u8 *) out) +  8);
    memRead8(mem +  9, ((u8 *) out) +  9);
    memRead8(mem + 10, ((u8 *) out) + 10);
    memRead8(mem + 11, ((u8 *) out) + 11);
    memRead8(mem + 12, ((u8 *) out) + 12);
    memRead8(mem + 13, ((u8 *) out) + 13);
    memRead8(mem + 14, ((u8 *) out) + 14);
    memRead8(mem + 15, ((u8 *) out) + 15);
    return 0;
}

void memWrite16(u32 mem, u16 value) {
    memWrite8(mem +  0, *(((u8 *) &value) +  0));
    memWrite8(mem +  1, *(((u8 *) &value) +  1));
}

void memWrite32(u32 mem, u32 value) {
    memWrite8(mem +  0, *(((u8 *) &value) +  0));
    memWrite8(mem +  1, *(((u8 *) &value) +  1));
    memWrite8(mem +  2, *(((u8 *) &value) +  2));
    memWrite8(mem +  3, *(((u8 *) &value) +  3));
}

void memWrite64(u32 mem, u64 value) {
    memWrite8(mem +  0, *(((u8 *) &value) +  0));
    memWrite8(mem +  1, *(((u8 *) &value) +  1));
    memWrite8(mem +  2, *(((u8 *) &value) +  2));
    memWrite8(mem +  3, *(((u8 *) &value) +  3));
    memWrite8(mem +  4, *(((u8 *) &value) +  4));
    memWrite8(mem +  5, *(((u8 *) &value) +  5));
    memWrite8(mem +  6, *(((u8 *) &value) +  6));
    memWrite8(mem +  7, *(((u8 *) &value) +  7));
}

void memWrite128(u32 mem, u64 *value) {
    memWrite8(mem +  0, *(((u8 *) &value) +  0));
    memWrite8(mem +  1, *(((u8 *) &value) +  1));
    memWrite8(mem +  2, *(((u8 *) &value) +  2));
    memWrite8(mem +  3, *(((u8 *) &value) +  3));
    memWrite8(mem +  4, *(((u8 *) &value) +  4));
    memWrite8(mem +  5, *(((u8 *) &value) +  5));
    memWrite8(mem +  6, *(((u8 *) &value) +  6));
    memWrite8(mem +  7, *(((u8 *) &value) +  7));
    memWrite8(mem +  8, *(((u8 *) &value) +  8));
    memWrite8(mem +  9, *(((u8 *) &value) +  9));
    memWrite8(mem + 10, *(((u8 *) &value) + 10));
    memWrite8(mem + 11, *(((u8 *) &value) + 11));
    memWrite8(mem + 12, *(((u8 *) &value) + 12));
    memWrite8(mem + 13, *(((u8 *) &value) + 13));
    memWrite8(mem + 14, *(((u8 *) &value) + 14));
    memWrite8(mem + 15, *(((u8 *) &value) + 15));
}

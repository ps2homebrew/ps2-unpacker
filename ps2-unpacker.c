#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "defines.h"
#include "memory.h"
#include "Interpreter.h"
#include "R5900.h"

u32 ELF_MAGIC = 0x464c457f;

#define PT_LOAD 1
#define PF_X    1
#define PF_W    2
#define PF_R    4

u8 trace = 0;

struct option long_options[] = {
    {"help", 0, NULL, 'h'},
    {"lo", 1, NULL, 'l'},
    {"hi", 1, NULL, 'i'},
    {"align", 1, NULL, 'a'},
    {"trace", 0, NULL, 't'},
    {"raw", 1, NULL, 'r'},
    {"ep", 1, NULL, 'e'},
    {"norun", 0, NULL, 'n'},
    {0, 0, 0, 0}};

/* The primary ELF header. */
typedef struct
{
    u8 ident[16];  /* The first 4 bytes are the ELF magic */
    u16 type;      /* == 2, EXEC (executable file) */
    u16 machine;   /* == 8, MIPS r3000 */
    u32 version;   /* == 1, default ELF value */
    u32 entry;     /* program starting point */
    u32 phoff;     /* program header offset in the file */
    u32 shoff;     /* section header offset in the file, unused for us, so == 0 */
    u32 flags;     /* flags, unused for us. */
    u16 ehsize;    /* this header size ( == 52 ) */
    u16 phentsize; /* size of a program header ( == 32 ) */
    u16 phnum;     /* number of program headers */
    u16 shentsize; /* size of a section header, unused here */
    u16 shnum;     /* number of section headers, unused here */
    u16 shstrndx;  /* section index of the string table */
} elf_header_t;

typedef struct
{
    u32 type;   /* == 1, PT_LOAD (that is, this section will get loaded */
    u32 offset; /* offset in file, on a 4096 bytes boundary */
    u32 vaddr;  /* virtual address where this section is loaded */
    u32 paddr;  /* physical address where this section is loaded */
    u32 filesz; /* size of that section in the file */
    u32 memsz;  /* size of that section in memory (rest is zero filled) */
    u32 flags;  /* PF_X | PF_W | PF_R, that is executable, writable, readable */
    u32 align;  /* == 0x1000 that is 4096 bytes */
} elf_pheader_t;

void show_banner()
{
    printf(
        "PS2-Unpacker v" VERSION " (C) 2004-2005 Nicolas \"Pixel\" Noble\n"
        "This is free software with ABSOLUTELY NO WARRANTY.\n"
        "\n");
}

void show_usage()
{
    printf(
        "Usage: ps2-unpacker [-r base] [-e ep] [-n] [-l lo] [-i hi] [-a align] [-t] <in_elf> <out_elf>\n");
}

u8 ident[] = {
    0x7F, 0x45, 0x4C, 0x46, 0x01, 0x01, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

int main(int argc, char **argv)
{
    FILE *elf, *out;
    elf_header_t *eh, weh;
    elf_pheader_t *eph, weph;
    char *elfname, *outname;
    u8 *bytes;
    u32 size;
    u32 lo_base = 0;
    u32 hi_base = 0;
    u32 align = 16;
    u32 ep = 0;
    u32 raw = 0;
    int i;
    int norun = 0;
    char c;

    show_banner();

    while ((c = getopt_long(argc, argv, "l:i:ta:h:e:r:n", long_options, NULL)) != EOF) {
        switch (c) {
            case 'l':
                lo_base = strtol(optarg, NULL, 0);
                break;
            case 'i':
                hi_base = strtol(optarg, NULL, 0);
                break;
            case 't':
                trace = 1;
                break;
            case 'a':
                align = strtol(optarg, NULL, 0);
                break;
            case 'h':
                show_usage();
                exit(0);
            case 'e':
                ep = strtol(optarg, NULL, 0);
                break;
            case 'r':
                raw = strtol(optarg, NULL, 0);
                break;
            case 'n':
                norun = 1;
                break;
            default:
                printf("Unknown option %c\n", c);
                show_usage();
                exit(-1);
        }
    }

    if ((argc - optind) != 2) {
        printf("%i files specified, I need exactly 2.\n", argc - optind);
        exit(-1);
    }

    elfname = strdup(argv[optind++]);
    outname = strdup(argv[optind++]);

    printf("Initialising memory.\n");
    mem_init();

    printf("Loading elf %s.\n", elfname);
    if (!(elf = fopen(elfname, "rb"))) {
        printf("Error opening output file %s\n", elfname);
        exit(-1);
    }
    fseek(elf, 0, SEEK_END);
    bytes = (u8 *)malloc(size = ftell(elf));
    fseek(elf, 0, SEEK_SET);
    if (fread(bytes, 1, size, elf) != size) {
        printf("Error reading elf.\n");
        exit(-1);
    }

    if (raw) {
        load_section(raw, bytes, size);
    } else {

        if (*((u32 *)bytes) != ELF_MAGIC) {
            printf("This is not an elf file: %08X.\n", *((u32 *)bytes));
            exit(-1);
        }

        eh = (elf_header_t *)bytes;
        eph = (elf_pheader_t *)(bytes + eh->phoff);

        for (i = 0; i < eh->phnum; i++, eph = (elf_pheader_t *)(bytes + eh->phoff + eh->phentsize * i)) {
            if (eph->type != PT_LOAD)
                continue;

            printf("Loading section %i, %08X bytes, at %08X from %08X.\n", i, eph->filesz, eph->vaddr, eph->offset);
            load_section(eph->vaddr, bytes + eph->offset, eph->filesz);
        }
    }

    fclose(elf);
    memset(&cpuRegs, 0, sizeof(cpuRegs));
    if (ep) {
        cpuRegs.pc = ep;
    } else {
        cpuRegs.pc = eh->entry;
    }
    free(bytes);

    if (norun) {
        bounce_pc = ep;
        if (!lo_base)
            lo_base = raw;
        if (!hi_base)
            hi_base = raw + size;
    } else {
        printf("Starting emulation at %08X.\r", cpuRegs.pc);
        fflush(stdout);

        while (!bounce_pc) {
            execI();
        }

        printf("\nGot entry point: %08X\n", bounce_pc);

        printf("Looking for data around.\n");

        if (!lo_base) {
            for (lo_base = bounce_pc; has_write(lo_base - 1); lo_base--)
                ;
            for (; is_zero(lo_base); lo_base++)
                ;
            if (lo_base & (align - 1)) {
                lo_base += align;
                lo_base &= -align;
                lo_base -= align;
            }
        }
        printf("Using lo = %08X\n", lo_base);
        if (!hi_base) {
            for (hi_base = bounce_pc; has_write(hi_base + 1); hi_base++)
                ;
            for (; is_zero(hi_base - 1); hi_base--)
                ;
        }
        printf("Using hi = %08X\n", hi_base);
    }


    printf("Dumping.\n");
    if (!(out = fopen(outname, "wb"))) {
        printf("Error opening output file %s\n", outname);
        exit(-1);
    }

    for (i = 0; i < 16; i++) {
        weh.ident[i] = ident[i];
    }
    weh.type = 2;
    weh.machine = 8;
    weh.version = 1;
    weh.phoff = sizeof(weh);
    weh.shoff = 0;
    weh.flags = 0;
    weh.ehsize = sizeof(weh);
    weh.phentsize = sizeof(weph);
    weh.phnum = 1;
    weh.shoff = 0;
    weh.shnum = 0;
    weh.shentsize = 0;
    weh.entry = bounce_pc;
    weh.shstrndx = 0;

    if (fwrite(&weh, 1, sizeof(weh), out) != sizeof(weh)) {
        printf("Error writing elf header.\n");
        exit(-1);
    }

    weph.type = PT_LOAD;
    weph.flags = PF_R | PF_W | PF_X;
    weph.offset = 0x1000;
    weph.vaddr = lo_base;
    weph.paddr = lo_base;
    weph.filesz = hi_base - lo_base;
    weph.memsz = weph.filesz;
    weph.align = 0x1000;

    if (fwrite(&weph, 1, sizeof(weph), out) != sizeof(weph)) {
        printf("Error writing program header.\n");
        exit(-1);
    }

    fseek(out, 0x1000, SEEK_SET);

    bytes = (u8 *)malloc(hi_base - lo_base);

    dump_section(lo_base, bytes, hi_base - lo_base);

    if (fwrite(bytes, 1, hi_base - lo_base, out) != (hi_base - lo_base)) {
        printf("Error dumping section.\n");
        exit(-1);
    }

    free(bytes);
    fclose(out);

    printf("Done, exitting.\n");
    return 0;
}

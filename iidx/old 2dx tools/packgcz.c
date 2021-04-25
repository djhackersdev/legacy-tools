#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAXX 1024

typedef enum { true = 1, false = 0 } bool_t;
typedef enum { RIGHT, BELOW } position_t;
typedef unsigned char u8_t;
typedef unsigned short u16_t;
typedef unsigned int u32_t;

struct image {
    u16_t width;
    u16_t height;
    u32_t pixels[1];
};

struct gc {
    u32_t fourcc;
    u32_t length_entire;
    u32_t unk1;
    u16_t width_be;
    u16_t height_be;
    u32_t unk2;
    u32_t length_pixels;
    u16_t pixels[0x100000];
};

struct subtexture {
    u16_t x;
    u16_t y;
    u16_t w;
    u16_t h;
    char *name;
    struct image *img;
};

static void die(const char *fmt, ...)
{
    va_list ap;
    
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    
    exit(EXIT_FAILURE);
}

static char *stem(const char *src)
{
    /* Borrowed from crackgcz.c */
    const char *pslash;
    const char *pdot;
    char *result;

    pslash = strrchr(src, '/');
    pdot = strrchr(src, '.');

    if (pslash == NULL) pslash = strrchr(src, '\\');
    if (pdot == NULL) pdot = src + strlen(src);

    if (pslash == NULL) {
        pslash = src;
    } else {
        pslash++;
    }

    result = calloc(pdot - pslash + 1, 1);
    strncpy(result, pslash, pdot - pslash);

    return result;
}

static struct image *read_tga(const char *filename)
{
    struct image *img;
    u16_t width;
    u16_t height;
    u8_t bits;
    FILE *f;
    
    f = fopen(filename, "rb");
    if (f == NULL) die("%s: Error opening file\n", filename);
    
    /* Get dimensions. We assume this is actually a TGA of the common
     * top-left origin true colour type. */
    fseek(f, 0xC, SEEK_SET);
    fread(&width, 2, 1, f);
    fread(&height, 2, 1, f);
    fread(&bits, 1, 1, f);
    
    if (bits != 32) die("%s: Not a 32-bit TGA file\n", filename);
    
    /* Alloc */
    img = malloc(sizeof(struct image) + width * height * 4);
    img->width = width;
    img->height = height;
    
    /* Read in */
    fseek(f, 0x12, SEEK_SET);
    fread(img->pixels, 4, width * height, f);
    
    /* Cleanup */
    fclose(f);
    return img;
}

static struct subtexture *get_subs(char **filenames, int nsubs)
{
    struct subtexture *subs;
    struct image *img;
    int i;
    
    fprintf(stderr, "Reading TGA files ... ");
    subs = malloc(sizeof(struct subtexture) * nsubs);
    
    for (i = 0 ; i < nsubs ; i++) {
        img = read_tga(filenames[i]);
        
        subs[i].w = img->width;
        subs[i].h = img->height;
        subs[i].name = stem(filenames[i]);
        subs[i].img = img;
    }
    
    fprintf(stderr, "%d read\n", nsubs);
    return subs;
}

static int height_pred(const void *ptr1, const void *ptr2)
{
    const struct subtexture *lhs = ptr1;
    const struct subtexture *rhs = ptr2;
    
    /* Descending order */
    return rhs->h == lhs->h ? rhs->w - lhs->w : rhs->h - lhs->h; 
}

static int name_pred(const void *ptr1, const void *ptr2)
{
    const struct subtexture *lhs = ptr1;
    const struct subtexture *rhs = ptr2;
    
    return strcmp(lhs->name, rhs->name);
}

static bool_t place(struct subtexture *subs, int operand_idx,
    int adjunct_idx, position_t pos)
{
    struct subtexture *adjunct = subs + adjunct_idx;
    struct subtexture *operand = subs + operand_idx;
    struct subtexture *sub;
    int i;
    
    /* Position operand relative to adjunct. We leave 1px of separation
     * between the subs to avoid resampling artefacts in-game. */
    operand->x = adjunct->x + (pos == RIGHT ? adjunct->w + 1 : 0);
    operand->y = adjunct->y + (pos == BELOW ? adjunct->h + 1 : 0);
    
    /* Check for X overrun */
    if (operand->x + operand->w > MAXX) return false;
    
    /* Test for collisions */
    for (i = 0 ; i < operand_idx ; i++) {
        sub = subs + i;
        
        if (!(
            operand->x + operand->w < sub->x ||
            operand->x > sub->x + sub->w ||
            operand->y + operand->h < sub->y ||
            operand->y > sub->y + sub->h
        )) return false; /* Collision */
    }

    /* No collisions with any other subs that have been placed so far,
     * accept this placement. */
    return true;
}

static u16_t place_subs(struct subtexture *subs, int nsubs)
{
    u16_t maxy;
    int i;
    int j;
    
    fprintf(stderr, "Arranging subtextures ...\n");
    
    /* Place subs in order of descending height */
    qsort(subs, nsubs, sizeof(struct subtexture), height_pred);
    
    /* Seed the process */
    subs[0].x = 0;
    subs[0].y = 0;
    maxy = 0;
    
    /* For each sub, try to place the sub to the right of a placed sub,
     * or failing that try to place the sub below a placed sub. There is
     * no vertical limit to the tiling's dimension, so the latter approach
     * will always succeed eventually. */
    for (i = 1 ; i < nsubs ; i++) {
        for (j = 0 ; j < i ; j++) {
            if (place(subs, i, j, RIGHT)) break;
        }
        
        if (j == i) for (j = 0 ; j < i ; j++) {
            if (place(subs, i, j, BELOW)) break;
        }
        
        assert(j != nsubs); /* Like I said, place_below() shouldn't fail. */
        
        /* Update maxy */
        maxy = maxy < subs[i].y + subs[i].h ? subs[i].y + subs[i].h : maxy;
        if (i % 100 == 0) fprintf(stderr, "%d/%d ...\n", i, nsubs);
    }

#ifndef NDEBUG
    /* Dump the tiling in the form of some rather bad HTML */
    printf("<style>div { font-size: 8px; position: absolute; "
        "background-color: #77f; }</style>\n");
        
    for (i = 0 ; i < nsubs ; i++) {
        printf("<div style='left: %d; top: %d; width: %d; height: %d'>"
            "%s</div>\n", subs[i].x, subs[i].y, subs[i].w, subs[i].h,
            subs[i].name);
    }
#endif

    /* Finally, sort everything by name */
    qsort(subs, nsubs, sizeof(struct subtexture), name_pred);
    
    fprintf(stderr, "maxy = %d\n", maxy);
    return maxy;
}

static void copy_scanlines(struct subtexture *sub, struct gc *gcs)
{
    struct gc *gc;
    u32_t p32;
    u16_t p16;
    int gc_top;
    int gc_nr;
    int x;
    int y;

    for (y = 0 ; y < sub->h ; y++) {
        gc_nr = (sub->y + y) / 1024;
        gc = gcs + gc_nr;
        gc_top = gc_nr * 1024;
        
        for (x = 0 ; x < sub->w ; x++) {
            p32 = sub->img->pixels[y * sub->w + x];
            p16 =
                  (((p32 & 0xFF000000) >= 0x80) << 15)
                | (((p32 & 0x00FF0000) >> 19) << 10)
                | (((p32 & 0x0000FF00) >> 11) <<  5)
                | (((p32 & 0x000000FF) >>  3));
                
            gc->pixels[(sub->y + y - gc_top) * 1024 + sub->x + x] = p16;
        }
    }
}

static u32_t swab32(u32_t in)
{
    u32_t out;
    
    out = ((in & 0xFF000000) >> 24) | ((in & 0x00FF0000) >>  8)
        | ((in & 0x0000FF00) << 8 ) | ((in & 0x000000FF) << 24);
    return out;
}

static struct gc *pack_gc_files(struct subtexture *subs, int nsubs,int ngcs)
{
    struct subtexture *sub;
    struct gc *gcs;
    int i;

    fprintf(stderr, "Allocating a huge amount of memory and "
        "generating GC files ... ");    
    gcs = calloc(sizeof(struct gc), ngcs);
    
    for (i = 0 ; i < ngcs ; i++) {
        gcs[i].fourcc = 0x204347; /* 'GC \0' */
        gcs[i].length_entire = swab32(sizeof(struct gc));
        gcs[i].width_be = 0x0004; /* stores as 0x0400 BE */
        gcs[i].height_be = 0x0004; /* ditto */
        gcs[i].length_pixels = swab32(1024 * 1024 * 2);
    }
    
    for (i = 0 ; i < nsubs ; i++) {
        sub = subs + i;
        copy_scanlines(sub, gcs);
        
        /* Release a bit of memory */
        free(sub->img);
    }
    
    fprintf(stderr, "Done\n");
    return gcs;
}

static void write_gcz_files(const char *dir, const struct gc *gcs, int ngcs)
{
    static const u32_t exsize = sizeof(struct gc);
    char filename[256];
    u8_t *bytes;
    FILE *f;
    int i;
    int j;
    
    fprintf(stderr, "Writing out GCZ files ... ");
    
    for (i = 0 ; i < ngcs ; i++) {
        /* Open up */
        snprintf(filename, sizeof(filename), "%s/%x.gcz", dir, i);
        f = fopen(filename, "wb");
        if (f == NULL) die("Error opening %s for writing\n", filename);
        
        fwrite(&exsize, 4, 1, f);
        bytes = (u8_t *) (gcs + i);
        
        for (j = sizeof(struct gc) / 8 ; j > 0 ; j--) {
            fputc(0xFF, f);
            fwrite(bytes, 1, 8, f);
            bytes += 8;
        }
    }
    
    fprintf(stderr, "Done\n");
}

static void write_system_idx(const char *dir, const struct subtexture *subs, 
    int nsubs, int ngcs)
{
    char trailer[8];
    char gczname[32];
    char filename[512];
    long p1;
    long p2;
    u32_t rect_offset;
    u32_t label_offset;
    FILE *f;
    u16_t i;
    
    fprintf(stderr, "Creating SYSTEM.IDX ... ");
    
    snprintf(filename, 512, "%s/SYSTEM.IDX", dir);
    f = fopen(filename, "wb");
    if (f == NULL) die("%s: Could not open for writing\n");
    
    rect_offset = 0x1B8;
    label_offset = 0x1B8 + (nsubs + 1) * 8;
    
    /* Write header */
    fwrite(&label_offset, 4, 1, f);
    fputc(0x14, f);
    fputc(0x00, f);
    fputc(ngcs, f);
    fputc(0x00, f);
    fwrite(&rect_offset, 4, 1, f);
    fwrite(&label_offset, 4, 1, f);
    
    /* Write GCZ path list */
    fseek(f, 0x18, SEEK_SET);
    memset(gczname, 0, 32);
    
    for (i = 0 ; i < ngcs ; i++) {
        snprintf(gczname, 32, "/sys/%s/%x.gcz", dir, i);
        fwrite(gczname, 1, 32, f);
    }
    
    /* Write rect list */
    fseek(f, rect_offset + 4, SEEK_SET);
    
    for (i = 0 ; i < nsubs ; i++) {
        fwrite(subs + i, 2, 4, f);
    }
    
    memset(trailer, 0, 8);
    fwrite(trailer, 2, 4, f);
    
    /* Write label list */
    fseek(f, label_offset + 8, SEEK_SET);
    p1 = ftell(f);
    
    for (i = 0 ; i < nsubs ; i++) {
        fwrite(subs[i].name, 1, strlen(subs[i].name) + 1, f);
        fwrite(&i, 2, 1, f);
    }
    
    /* Go back and write the label list length */
    p2 = ftell(f) - p1;
    fseek(f, label_offset + 4, SEEK_SET);
    fwrite(&p2, 4, 1, f);    
    
    fprintf(stderr, "Done\n");
    fclose(f);
}

int main(int argc, char **argv)
{
    struct subtexture *subs;
    struct gc *gcs;
    int nsubs;
    int ngcs;
    u16_t height;

    nsubs = argc - 2;
    subs = get_subs(argv + 2, nsubs);
    height = place_subs(subs, nsubs);
    
    ngcs = height % 1024 == 0 ? height / 1024 : height / 1024 + 1;    
    gcs = pack_gc_files(subs, nsubs, ngcs);
    write_gcz_files(argv[1], gcs, ngcs);
    write_system_idx(argv[1], subs, nsubs, ngcs);
    
    return EXIT_SUCCESS;
}

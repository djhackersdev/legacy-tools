/* gcz2tga.c: Slice up a directory full of GCZ (texture) files into TGA files.
 *
 * Credit goes to afwefwe for reverse-engineering the texture format 
 * and LZSS compression */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Assuming x86, usual endian crap is not accounted for */
typedef unsigned char u8_t;
typedef unsigned short u16_t;
typedef unsigned int u32_t;

struct image
{
    unsigned int width, height;
    u8_t *planes;
};

struct clip
{
    short x, y, w, h;
};

static u16_t swab16(u16_t in)
{
    /* GC headers are big-endian */
    return ((in & 0xFF) << 8) | (in >> 8);
}

static void unpack_gc(struct image *out, u8_t *in, size_t out_sz)
{
    unsigned int npixels;
    unsigned int i; 
    unsigned int j; 
    u16_t *pixels; 
    u16_t *pheight; 
    u16_t *pwidth;
    u16_t *pmagic;
    u16_t pixel;

    pmagic = (u16_t *) in;
    pheight = (u16_t *) (in + 14);
    pwidth = (u16_t *) (in + 12);

    if (*pmagic != 0x4347) {
        fprintf(stderr, "(FAIL: Invalid header)\n");
        exit(EXIT_FAILURE);
    }

    /* Set up output image struct */
    in += 24;
    out->width = swab16(*pwidth);
    out->height = swab16(*pheight);
    out->planes = malloc(out->width * out->height * 4);

    /* Clamp W/H (don't know why this is necessary but it is) */
    out->width = out->width > 1024 ? 1024 : out->width;
    out->height = out->height > 1024 ? 1024 : out->height;

    fprintf(stderr, "(%dx%d)", out->width, out->height);

    /* Unpack pixels */
    pixels = (u16_t *) in;
    npixels = out->width * out->height;

    if (out_sz > npixels * 4) {
        /* Deep image (i.e. 32-bit) */
        memcpy(out->planes, pixels, npixels * 4);
    } else {
	/* Shallow image (i.e. 16-bit) */
        for (i = 0, j = 0 ; i < npixels ; i++) {
            pixel = pixels[i];
            out->planes[j++] = ((pixel      ) & 0x1F)  << 3; /* B */
            out->planes[j++] = ((pixel >>  5) & 0x1F)  << 3; /* G */
            out->planes[j++] = ((pixel >> 10)       )  << 3; /* R */
            out->planes[j++] = pixel & 0x8000 ? 0xFF : 0x00; /* A */
        }
    }
}

static u8_t *expand_lzss(u8_t *lzss, size_t *pout_sz)
{
    static u8_t ring[0x1000];
    unsigned int ring_pos = 0x0FEE; 
    unsigned int chunk_offset; 
    unsigned int chunk_length; 
    u32_t control_word = 1;
    size_t length;
    u8_t cmd1; 
    u8_t cmd2;
    u8_t *out; 
    u8_t *pos; 
    u8_t *in; 

    /* Header = 32 bit unpacked file length */
    length = *((u32_t *) lzss);
    *pout_sz = length;

    if (length > 8000000) {
        fprintf(stderr, "(FAIL: Unreasonably large expanded size %d)\n", 
            length);

        exit(EXIT_FAILURE);
    }

    out = malloc(length * 2); /* Seems to overrun */
    pos = out;
    in = lzss + 4;

    while (length > 0) {
        if (control_word == 1) {
            /* Read a control byte */
            control_word = 0x100 | *in++;
        }

        /* Decode a byte according to the current control byte bit */
        if (control_word & 1) {
            /* Straight copy */
            *pos++ = *in;
            ring[ring_pos] = *in++;

            ring_pos = (ring_pos + 1) % 0x1000;
            length--;
        } else {
            /* Reference to data in ring buffer */
            cmd1 = *in++;
            cmd2 = *in++;

            chunk_length = (cmd2 & 0x0F) + 3;
            chunk_offset = ((cmd2 & 0xF0) << 4) | cmd1;

            for ( ; chunk_length > 0 ; chunk_length--) {
                /* Copy historical data to output AND current ring pos */
                *pos++ = ring[chunk_offset];
                ring[ring_pos] = ring[chunk_offset];

                /* Update counters */
                chunk_offset = (chunk_offset + 1) % 0x1000;
                ring_pos = (ring_pos + 1) % 0x1000;
                length--;
            }
        }

        /* Get next control bit */
        control_word >>= 1;
    }

    return out;
}

static void readfile(const char *filename, u8_t **data, long *nbytes)
{
    FILE *f;

    f = fopen(filename, "rb");
    if (f == NULL) abort();

    fseek(f, 0, SEEK_END);
    *nbytes = ftell(f);
    fseek(f, 0, SEEK_SET);

    *data = malloc(*nbytes);
    fread(*data, *nbytes, 1, f);

    fclose(f);
}

void put8(FILE *f, unsigned char val)
{
    fwrite(&val, 1, 1, f);
}

void put16(FILE *f, unsigned short val)
{
    fwrite(&val, 2, 1, f);
}

void split_images(const char *in_dir, const char *out_dir, 
    struct image *images, int nimages)
{
    struct clip *clips;
    char filename[512];
    long nbytes;
    u8_t *data;
    char *name;
    FILE *f;
    int i;
    int j;
    int k;

    /* Read file and get TOC */
    sprintf(filename, "%s/system.idx", in_dir);
    readfile(filename, &data, &nbytes);
    clips = (struct clip *) (data + 0x01BC);
    name = (char *) (data + 8 + *((long *) data));
    
    /* Guess how many clips there are with a heuristic */
    for (i = 0 ; clips[i].w != 0 && clips[i].h != 0 ; i++) {
        sprintf(filename, "%s/%s.tga", out_dir, name);
        name += strlen(name) + 3;
        
        f = fopen(filename, "wb");
        if (f == NULL) abort();

        /* Locate the correct source image */
        j = 0;
        while (clips[i].y > images[j].height) {
            clips[i].y -= images[j].height;
            j++;
        }

        /* Write header */
        put8(f, 0); put8(f, 0); put8(f, 2);
        put16(f, 0); put16(f, 0); put8(f, 0);
        put16(f, 0); put16(f, 0); put16(f, clips[i].w); put16(f, clips[i].h);
        put8(f, 32); put8(f, 32);

        /* Write scanlines */
        for (k = 0 ; k < clips[i].h ; k++) {
            if (clips[i].y == images[j].height) {
                clips[i].y = 0;
                j++;
            }

            fwrite(images[j].planes + ((images[j].width * clips[i].y) +
                clips[i].x) * 4, clips[i].w, 4, f);
            clips[i].y++;
        }
        
        /* Close output file */
        fclose(f);
    }

    /* Cleanup */
    free(data);
}

int main(int argc, char **argv)
{
    char *in_dir;
    char *out_dir;

    struct image images[32];
    char filename[256];
    unsigned int i;
    long filesize;
    u8_t *lzss, *gc;
    size_t out_sz;
    FILE *f;

    /* Usage */
    if (argc != 3) {
        fprintf(stderr, "Usage: %s [indir] [outdir]\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Setup */
    memset(images, 0, sizeof(images));
    in_dir = argv[1];
    out_dir = argv[2];

    for (i = 0 ; i < 32 ; i++) {
        /* Open 0.gcz, 1.gcz etc ... */
        sprintf(filename, "%s/%d.gcz", in_dir, i);
        f = fopen(filename, "rb");
        if (f == NULL) break;

        /* Read entire file */
        fseek(f, 0, SEEK_END);
        filesize = ftell(f);
        fseek(f, 0, SEEK_SET);

        fprintf(stderr, "%s: fread", filename);
        lzss = malloc(filesize);
        fread(lzss, filesize, 1, f);
        fclose(f);

        /* Decompress */
        fprintf(stderr, "(OK) expand_lzss");
        gc = expand_lzss(lzss, &out_sz);
        free(lzss);

        /* Unpack GC to 32-bit RGBA */
        fprintf(stderr, "(OK) unpack_gc");
        unpack_gc(&images[i], gc, out_sz);
        free(gc);

        fprintf(stderr, "(OK)\n");
    }

    /* Sanity check */
    if (i == 0) {
        fprintf(stderr, "No GCZ files found\n");
        exit(EXIT_FAILURE);
    }

    /* Emit pile of TGAs */
    fprintf(stderr, "split_images");
    split_images(in_dir, out_dir, images, i);
    fprintf(stderr, "(OK)\n\n");
    
    return 0;
}


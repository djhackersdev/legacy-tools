/** unz.c: Decompress miscellaneous stuff like .INZ and .XZ files */
#include <stdlib.h>
#include <stdio.h>

typedef unsigned char u8_t;
typedef unsigned int u32_t;

static void expand_lzss(FILE *in, FILE *out)
{
    static u8_t ring[0x1000];
    unsigned int ring_pos = 0x0FEE; 
    unsigned int chunk_offset; 
    unsigned int chunk_length; 
    unsigned int length;
    u32_t control_word = 1;
    u8_t cmd1; 
    u8_t cmd2;
    u8_t byte;

    /* Read header (i.e. file length, ignore it) */
    fread(&length, 4, 1, in);

    while (!feof(in) && length > 0) {
        if (control_word == 1) {
            /* Read a control byte */
            control_word = 0x100 | getc(in);
        }

        /* Decode a byte according to the current control byte bit */
        if (control_word & 1) {
            /* Straight copy, store into history ring */
            byte = getc(in);

            putc(byte, out);
            ring[ring_pos] = byte;

            ring_pos = (ring_pos + 1) % 0x1000;
            length--;
        } else {
            /* Reference to data in ring buffer */
            cmd1 = getc(in);
            cmd2 = getc(in);

            chunk_length = (cmd2 & 0x0F) + 3;
            chunk_offset = ((cmd2 & 0xF0) << 4) | cmd1;

            for ( ; chunk_length > 0 && length > 0 ; chunk_length--) {
                /* Copy historical data to output AND current ring pos */
                putc(ring[chunk_offset], out);
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
}

int main(int argc, char **argv)
{
    FILE *in;
    FILE *out;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s [infile] [outfile]\n", argv[0]);
        return EXIT_FAILURE;
    }

    in = fopen(argv[1], "rb");
    out = fopen(argv[2], "wb");

    expand_lzss(in, out);

    fclose(in);
    fclose(out);

    return EXIT_SUCCESS;
}


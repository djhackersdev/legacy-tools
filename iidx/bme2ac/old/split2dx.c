/* split2dx.c: Extract all WAVs from a .2DX file (sound archive) */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    static const char base36[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char outname[64];
    FILE *out;
    FILE *in;
    void *bytes;
    int nfiles;
    int nbytes;
    int *toc;
    int i;

    /* Usage */
    if (argc != 3) {
        fprintf(stderr, "Usage: %s [infile] [outdir]\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Open input */
    in = fopen(argv[1], "rb");
    if (!in) {
        fprintf(stderr, "%s: Could not open file for reading\n", argv[1]);
        return EXIT_FAILURE;
    }

    /* Read TOC length */
    fseek(in, 0x14, SEEK_SET);
    fread(&nfiles, 4, 1, in);

    /* Read TOC */
    toc = malloc(4 * nfiles);
    fseek(in, 0x48, SEEK_SET);
    fread(toc, 4, nfiles, in);

    /* Extract files */
    for (i = 0 ; i < nfiles ; i++) {
        /* Name output file */
        sprintf(outname, "%s/%c%c.wav", argv[2],
            base36[(i + 1) / 36], base36[(i + 1) % 36]);

        /* Open output file */
        out = fopen(outname, "wb");
        if (!out) {
            fprintf(stderr, "%s: Could not open file for writing\n", outname);
            free(toc);

            return EXIT_FAILURE;
        }

        /* Put RIFF fourcc which we skipped */
        fwrite("RIFF", 4, 1, out);

        /* Get file length */
        fseek(in, toc[i] + 0x1C, SEEK_SET);
        fread(&nbytes, 4, 1, in);
        fwrite(&nbytes, 4, 1, out);

        /* Copy */
        bytes = malloc(nbytes);
        fread(bytes, 1, nbytes, in);
        fwrite(bytes, 1, nbytes, out);

        /* Clean up */
        fclose(out);
        free(bytes);
    }

    fclose(in);
    free(toc);

    return EXIT_SUCCESS;
}


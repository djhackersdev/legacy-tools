/**
 * Copyright (c) 2013 Tatsh. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT
 * HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * To contribute changes, fork this gist:
 * https://gist.github.com/Tatsh/5476094
 */

#include <sys/mman.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s FILENAME\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  char png_start[] = {0x89, 'P', 'N', 'G'};
  char png_end[] = {'I', 'E', 'N', 'D', 0xAE, 'B', 0x60, 0x82};

  int fd = open(argv[1], O_RDONLY);
  unsigned char *png_data;
  char *i;
  struct stat sb;
  char *addr;
  void *end;
  int v, j;
  int file_index = 1, size = 0;
  char filename[32];
  FILE *f;

  if (fd == -1) {
    fprintf(stderr, "Unable to open \"%s\"\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  if (fstat(fd, &sb) == -1) {
    fprintf(stderr, "Unable to stat \"%s\"\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

  if (addr == MAP_FAILED) {
    fprintf(stderr, "Unable to mmap \"%s\"\n", argv[1]);
    fprintf(stderr, "%s", strerror(errno));
    exit(EXIT_FAILURE);
  }

  end = addr + sb.st_size - 1;

  fprintf(stdout, "mmap() for \"%s\": %p\n", argv[1], addr);
  fprintf(stdout, "end offset = %p\n", end);

  for (i = addr; (void *)i < end; i++) {
    v = *i & 0xff;

    if (v == 0x89 && !strncmp(png_start, i, 4)) {
      while (strncmp(png_end, i, 8)) {
        size++;
        i++;
      }
      i -= size;
      png_data = malloc(size + 8);
      memset(png_data, 0, size);

      for (j = 0; j < size + 8; i++, j++) {
        png_data[j] = *i & 0xff;
      }

      sprintf(filename, "%08d.png", file_index);
      fprintf(stdout, "Writing \"%s\"\n", filename);
      f = fopen(filename, "wb");
      fwrite(png_data, size + 8, 1, f); // Cheap nmemb usage
      fclose(f);

      file_index++;
      size = 0;

      free(png_data);
    }
  }

  munmap(addr, sb.st_size);
  exit(EXIT_SUCCESS);
}

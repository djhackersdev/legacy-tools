#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ONE_BILLION 1000000000
#define IDX_CURRENT_KEY 55
#define IDX_COUNTER 56
#define IDX_LAST 57

typedef unsigned char byte;

static void mix(int *state)
{
    int tmp;
    int i;

    for (i = 1 ; i < 25 ; i++) {
        tmp = state[i];
        tmp -= state[i + 31];

        if (tmp < 0) tmp += ONE_BILLION;
        state[i] = tmp;
    }

    for (i = 25 ; i <= IDX_CURRENT_KEY ; i++) {
        tmp = state[i];
        tmp -= state[i - 24];

        if (tmp < 0) tmp += ONE_BILLION;
        state[i] = tmp;
    }
}

static void update_state(int *state, byte src_byte)
{
    int src;
    int tmp;
    int i;

    src = src_byte;
    tmp = 1;

    state[IDX_CURRENT_KEY] = src;

    for (i = 0x15 ; i <= 0x46E ; i += 0x15) {
        src -= tmp;
        state[i % 0x37] = tmp;
        tmp = src;

        if (tmp < 0) tmp += ONE_BILLION;
        src = state[i % 0x37];
    }

    mix(state);
    mix(state);
    mix(state);

    state[IDX_COUNTER] = 0x37;
}

static byte produce_key(int *state)
{
    int counter;

    counter = state[IDX_COUNTER] + 1;
    state[IDX_COUNTER] = counter;

    if (counter > 0x37) {
        mix(state);
        counter = 1;
    }

    state[IDX_COUNTER] = counter;
    return state[counter];
}

int main(int argc, char **argv)
{
    const byte key[] = {
        0x55, 0x37, 0x9F, 0xCC, 0xE3, 0xA7, 0x7D, 0x99,
        0xDD, 0xAA, 0xBB, 0xCF, 0xFC, 0x67, 0x43, 0x17
    };

    int state[IDX_LAST];
    int quo;
    int rem;
    int i;
    byte b;

    /* Setup */
    i = 0;
    memset(state, 0, sizeof(state));

    /* Descramble */
    while (!feof(stdin)) {
        quo = i / 0x10;
        rem = i % 0x10;
        i++;

        if (rem == 0) update_state(state, key[quo % 0x10]);

        b = getc(stdin);
        b -= produce_key(state);
        /* To rescramble an eout.bin, change -= above to += */

        putc(b, stdout);
    }

    return EXIT_SUCCESS;
}


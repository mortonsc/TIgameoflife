#include <stdbool.h>
#include <string.h>
#include "inc/ti84plus.h"
#include "inc/fastcopy.h"

/*
 * Code by Scott Morton 2016
 */

#define SCREEN_WIDTH_BYTES 12

void assert(int cond, const char *err)
{
    if (!cond) {
        VPutS(err);
        /* pause so I can actually see the error */
        GetKey();
    }
}

/*
 * sets the nth bit of byte to val (leftmost bit is 0)
 */
void set_bit(unsigned char *byte, int n, bool val)
{

    /* ex. if n = 3, mask is 0010 0000b */
    unsigned char mask = 0x80 >> n;

    if (val) {
        /* set the nth bit to 1, without affecting the others */
        *byte |= mask;
    } else {
        /* set the nth bit to 0, without affecting the others */
        *byte &= ~mask;
    }
}

/*
 *  returns the value of the nth bit of block (leftmost bit is 0)
 */
bool get_bit(unsigned char *byte, int n)
{
    /* ex. if n = 3, mask is 0010 0000b */
    unsigned char mask = 0x80 >> n;

    return *byte & mask;
}

bool new_cell(int live_neighbors, bool current)
{
    switch(live_neighbors) {
    case 0:
    case 1:
        /* death by loneliness */
        return false;
    case 2:
        return current;
    case 3:
        /* reproduction */
        return true;
    default:
        /* death by overcrowding */
        return false;
    }
}

void take_step()
{
    /* iteration vars */
    unsigned char *src_byte = plotSScreen;
    unsigned char *start = plotSScreen;
    unsigned char *end = plotSScreen + BUFFER_SIZE;
    /* first byte of the current row */
    unsigned char *start_row = plotSScreen;
    /* last byte of the current row */
    unsigned char *end_row = plotSScreen + SCREEN_WIDTH_BYTES - 1;
    unsigned char *dst_byte = appBackUpScreen;
    int bit = 0;
    unsigned char *i;
    int j;
    int live_neighbors;

    while (src_byte < end) {
        live_neighbors = 0;
        /*
         * iterate over each of the current cells neighbors
         * to count how many of them are alive
         */
        for (i = src_byte-SCREEN_WIDTH_BYTES;
                i <= src_byte+SCREEN_WIDTH_BYTES; i+=SCREEN_WIDTH_BYTES)
            for (j = bit-1; j <= bit+1; j++)
                if (start <= i && i < end) {
                    if (j == -1 && src_byte > start_row && get_bit(src_byte-1, 7))
                        live_neighbors++;
                    else if (j == 8 && src_byte < end_row && get_bit(src_byte+1, 0))
                        live_neighbors++;
                    else if (0 <= j && j < 8 && get_bit(src_byte, j))
                        live_neighbors++;
                }
        /* the above loop will count a cell as its own neighbor, so we need to compensate */
        if (get_bit(src_byte, bit))
            live_neighbors--;
        set_bit(dst_byte, bit, new_cell(live_neighbors, get_bit(src_byte, bit)));
        bit++;
        if (bit == 8) {
            bit = 0;
            src_byte++;
            dst_byte++;
            if (src_byte > end_row) {
                start_row += SCREEN_WIDTH_BYTES;
                end_row += SCREEN_WIDTH_BYTES;
            }
        }
    }

    /* copy in the new values */
    memcpy(plotSScreen, appBackUpScreen, BUFFER_SIZE);
    /* display the updated screen */
    FastCopy();
}

int main()
{
    memset(appBackUpScreen, 0, BUFFER_SIZE);
    set_bit(appBackUpScreen + 20*SCREEN_WIDTH_BYTES, 4, true);
    set_bit(appBackUpScreen + 20*SCREEN_WIDTH_BYTES, 5, true);
    set_bit(appBackUpScreen + 20*SCREEN_WIDTH_BYTES, 6, true);
    memcpy(plotSScreen, appBackUpScreen, BUFFER_SIZE);
    FastCopy();

    take_step();
    penCol = 0;
    return 0;
}

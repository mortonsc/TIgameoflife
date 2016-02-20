#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "inc/ti84plus.h"
#include "inc/fastcopy.h"


/*
 * Code by Scott Morton 2016
 */

#define SCREEN_WIDTH_BYTES 12

void assert(int cond, const char *err)
{
    if (!cond) {
        penRow = 0;
        penCol = 0;
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

/*
 * Moves the game forward one step.
 */
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

    /* iteration vars */
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
                if (start <= i && i < end &&
                    ( (j == -1 && src_byte > start_row && get_bit(i-1, 7))
                    || (j == 8 && src_byte < end_row && get_bit(i+1, 0))
                    || (0 <= j && j < 8 && get_bit(i, j)) ) ) {
                        live_neighbors++;
                    }
        /* the above loop will count a cell as its own neighbor, so we need to compensate */
        if (get_bit(src_byte, bit)) {
            live_neighbors--;
        }
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

#define BLOCK_WIDTH 25
#define BLOCK_HEIGHT 30

/*
 * a matrix located in saveSScreen
 * it would be more honest to cast saveSScreen the right type
 * but I couldn't figure out how to do that
 */
__at 0x86EC unsigned char neighbor_matrix[BLOCK_HEIGHT][BLOCK_WIDTH];

/*
 * Fills neighbor_matrix with the number of live neighbors of each pixel in the
 * BLOCK_HEIGHT x BLOCK_WIDTH rectangle with upper-left corner (y_start, x_start)
 *
 * Part of another (better?) implementation of take_step.
 * If we had unlimited memory, we would make a new matrix of how many
 * live neighbors each cell has. We don't have enough memory for that,
 * but we can do it in steps.
 * This way requires both appBackUpScreen and saveSScreen
 */
void fill_neighbor_matrix(int y_start, int x_start)
{
    unsigned char *byte
        = plotSScreen + y_start*SCREEN_WIDTH_BYTES + x_start / 8;
    int bit = x_start % 8;

    /*
     *  these variables serve as markers
     */
    unsigned char *start_row = byte;
    int start_bit = bit;

    /* iteration vars */
    int row;
    int col;
    int i;
    int j;

    for (row = 0; row < BLOCK_HEIGHT; row++) {
        for (col = 0; col < BLOCK_WIDTH; col++) {
            if (get_bit(byte, bit)) {
                i = (row > 0) ? row-1 : 0;
                j = (col > 0) ? col-1 : 0;
                for (; i <= row+1 && i < BLOCK_HEIGHT; i++)
                    for (; j <= col+1 && j < BLOCK_WIDTH; j++)
                        neighbor_matrix[i][j]++;
                /* the above loop counts a cell as its own neighbor */
                neighbor_matrix[row][col]--;
            }
            bit++;
            if (bit == 8) {
                bit = 0;
                byte++;
            }
        }
        start_row = start_row + SCREEN_WIDTH_BYTES;
        byte = start_row;
        bit = start_bit;
    }
}

/*
 * Based on the values in neighbor_matrix, set the pixels in appBackUpScreen
 * to their new values
 * This assumes appBackUpScreen contains their current values
 * At some point this will need to be able to exclude some rows/columns
 */
void load_neighbor_matrix(int y_start, int x_start)
{
    unsigned char *byte
        = appBackUpScreen + y_start*SCREEN_WIDTH_BYTES + x_start / 8;
    int bit = x_start % 8;

    unsigned char *start_row = byte;
    int start_bit = bit;

    /* iteration vars */
    int row;
    int col;

    int num_neighbors;

    for (row = 0; row < BLOCK_HEIGHT; row++) {
        for (col = 0; col < BLOCK_WIDTH; col++) {
            num_neighbors = neighbor_matrix[row][col];
            if (num_neighbors < 2 || num_neighbors > 3)
                set_bit(byte, bit, false);
            else if (num_neighbors == 3)
                set_bit(byte, bit, true);

            if (bit == 8) {
                bit = 0;
                byte++;
            }
        }
        start_row = start_row + SCREEN_WIDTH_BYTES;
        byte = start_row;
        bit = start_bit;
    }
}

int main()
{
    int i;
    unsigned char *test_byte = appBackUpScreen + 20*SCREEN_WIDTH_BYTES;

    memset(appBackUpScreen, 0, BUFFER_SIZE);
    set_bit(test_byte, 6, true);
    set_bit(test_byte, 7, true);
    set_bit(test_byte+1, 0, true);
    memcpy(plotSScreen, appBackUpScreen, BUFFER_SIZE);
    FastCopy();

    take_step();
    GetKey();
    for (i = 0; i < 8; i++) {
        if (get_bit(test_byte, i))
            PutC('*');
        else
            PutC('0');
    }

    return 0;
}

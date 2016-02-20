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

#define BLOCK_WIDTH 25
#define BLOCK_HEIGHT 30

#define STRIP_HEIGHT 8
#define STRIP_WIDTH 96

/*
 * matrices located in saveSScreen
 * it would be more honest to cast saveSScreen to arrays
 * but I couldn't figure out how to do that
 */
__at 0x86EC unsigned char neighbor_matrix[BLOCK_HEIGHT][BLOCK_WIDTH];
__at 0x86EC unsigned char neighbor_matrix_strip[STRIP_HEIGHT][STRIP_WIDTH];


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
                i = (row > 0) ? row-1 : row;
                for (; i <= row+1 && i < BLOCK_HEIGHT; i++) {
                    j = (col > 0) ? col-1 : col;
                    for (; j <= col+1 && j < BLOCK_WIDTH; j++) {
                        neighbor_matrix[i][j]++;
                    }
                }
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
 * Fills neighbor_matrix_strip with the number of neighbors of each cell
 * along the middle strip
 * There is no x_start parameter because it always starts from x = 0
 */
void fill_neighbor_matrix_strip(int y_start)
{
    unsigned char *byte
        = plotSScreen + y_start*SCREEN_WIDTH_BYTES;
    int bit = 0;

    /*
     *  these variables serve as markers
     */
    unsigned char *start_row = byte;

    /* iteration vars */
    int row;
    int col;
    int i;
    int j;

    for (row = 0; row < STRIP_HEIGHT; row++) {
        for (col = 0; col < STRIP_WIDTH; col++) {
            if (get_bit(byte, bit)) {
                i = (row > 0) ? row-1 : row;
                for (; i <= row+1 && i < STRIP_HEIGHT; i++) {
                    j = (col > 0) ? col-1 : col;
                    for (; j <= col+1 && j < STRIP_WIDTH; j++) {
                        neighbor_matrix_strip[i][j]++;
                    }
                }
                /* the above loop counts a cell as its own neighbor */
                neighbor_matrix_strip[row][col]--;
            }
            bit++;
            if (bit == 8) {
                bit = 0;
                byte++;
            }
        }
        start_row = start_row + SCREEN_WIDTH_BYTES;
        byte = start_row;
        bit = 0;
    }
}

/*
 * Based on the values in neighbor_matrix, set the pixels in appBackUpScreen
 * to their new values along the middle strip
 * This assumes appBackUpScreen contains their current values
 */
void load_neighbor_matrix_strip(int y_start)
{
    unsigned char *byte
        = appBackUpScreen + y_start*SCREEN_WIDTH_BYTES;
    int bit = 0;

    unsigned char *start_row = byte;

    /* iteration vars */
    int row;
    int col;

    int num_neighbors;

    for (row = 0; row < STRIP_HEIGHT; row++) {
        for (col = 0; col < STRIP_WIDTH; col++) {
            num_neighbors = neighbor_matrix_strip[row][col];
            if (num_neighbors < 2 || num_neighbors > 3)
                set_bit(byte, bit, false);
            else if (num_neighbors == 3)
                set_bit(byte, bit, true);

            bit++;
            if (bit == 8) {
                bit = 0;
                byte++;
            }
        }
        start_row = start_row + SCREEN_WIDTH_BYTES;
        byte = start_row;
        bit = 0;
    }
}


int main()
{
    unsigned char *test_byte = appBackUpScreen + 20*SCREEN_WIDTH_BYTES;

    DisableAPD();

    memset(saveSScreen, 0, BUFFER_SIZE);
    memset(appBackUpScreen, 0, BUFFER_SIZE);
    set_bit(test_byte, 6, true);
    set_bit(test_byte, 7, true);
    set_bit(test_byte+1, 0, true);
    memcpy(plotSScreen, appBackUpScreen, BUFFER_SIZE);
    FastCopy();

    GetKey();

    fill_neighbor_matrix_strip(18);
    /* memcpy(plotSScreen, saveSScreen, BUFFER_SIZE); */
    load_neighbor_matrix_strip(18);
    memcpy(plotSScreen, appBackUpScreen, BUFFER_SIZE);
    FastCopy();

    GetKey();

    EnableAPD();

    return 0;
}

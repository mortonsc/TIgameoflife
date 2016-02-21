#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "inc/ti84plus.h"
#include "inc/fastcopy.h"


/*
 * Code by Scott Morton 2016
 */


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

/*
 * EXPLANATION OF ALGORITHM:
 * The basic idea is as follows: have a matrix with an element for each pixel.
 * That element is the number of live neighbor cells that pixel has.
 * To advance a step, iterate over all the pixels, and for each live one,
 * increment the matrix element for each of its neighbors. Then, to update
 * the board, check the matrix for each pixel to see how many live neighbors
 * it has, and therefore what should happen to it.
 *
 * The TI84+ doesn't have enough RAM available for this matrix. It would
 * require a byte for every pixel, which comes out to 6144 bytes. What we have
 * is two areas of contiguous memory, each containing 768 bytes, or one bit
 * per pixel. One of these blocks (appBackUpScreen) will be used to store the
 * state of the new board as it is constructed. The other (plotSScreen) is
 * used as the matrix described above, except it only contains data for a
 * portion of the screen.
 *
 * As a result, the computation is performed in parts. The screen is split
 * into 9 segments: four 30x25 pixel rectangular blocks along the top,
 * four blocks along the bottom, and a long 8x96 strip along the middle.
 * Since the new state of a cell only depends on the cells near it, we can
 * compute the * new board state one section at a time.
 *
 * The sections overlap, so that for most pixels, there is a section
 * containing both it and all of its neighbors. The exception are pixels on
 * the vertical edges in between two blocks. The overlap between two blocks
 * is only one column of pixels, so for this pixels, it is necessary
 * to pass off some information between the two blocks.
 */

#define SCREEN_WIDTH_BYTES 12

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
 * BLOCK_HEIGHT x BLOCK_WIDTH rectangle with upper-left corner (origin_y, origin_x)
 */
void fill_neighbor_matrix(int origin_y, int origin_x, bool complete_final_column)
{
    unsigned char *byte
        = plotSScreen + origin_y*SCREEN_WIDTH_BYTES + origin_x / 8;
    unsigned char mask = 0x80 >> (origin_x % 8);

    /*
     *  these variables serve as markers
     */
    unsigned char *start_row = byte;
    unsigned char start_mask = mask;

    /* iteration vars */
    int row;
    int col;
    int i;
    int j;

    for (row = 0; row < BLOCK_HEIGHT; row++) {
        for (col = 0; col < BLOCK_WIDTH; col++) {
            if (*byte & mask) {
                if (col < BLOCK_WIDTH - 1 || complete_final_column) {
                    i = (row > 0) ? row-1 : row;
                    for (; i <= row+1 && i < BLOCK_HEIGHT; i++) {
                        j = (col > 0) ? col-1 : col;
                        for (; j <= col+1 && j < BLOCK_WIDTH; j++) {
                            neighbor_matrix[i][j]++;
                        }
                    }
                    /* the above loop counts a cell as its own neighbor */
                    neighbor_matrix[row][col]--;
                } else {
                    /* only update values on the column to the left */
                    i = (row > 0) ? row-1 : row;
                    j = col - 1;
                    for (; i <= row+1 && i < BLOCK_HEIGHT; i++)
                        neighbor_matrix[i][j]++;
                }
            }
            mask >>= 1;
            if (!mask) {
                mask = 0x80;
                byte++;
                /* no need to bother with blank bytes */
                while (!(*byte) && col < BLOCK_WIDTH - 1) {
                    byte++;
                    col += 8;
                }
            }
        }
        start_row = start_row + SCREEN_WIDTH_BYTES;
        byte = start_row;
        mask = start_mask;
    }
}

/*
 * Based on the values in neighbor_matrix, set the pixels in appBackUpScreen
 * to their new values
 * The only pixels set are those whose correspond elements in neighbor_matrix
 * are within the rectangle bounded by x_start, x_end, y_start, y_end
 * (start values inclusive, end values exclusive)
 * This assumes appBackUpScreen contains their current values
 */
void load_neighbor_matrix(int origin_y, int origin_x,
        int y_start, int y_end, int x_start, int x_end)
{
    unsigned char *byte = appBackUpScreen
        + (origin_y+y_start)*SCREEN_WIDTH_BYTES + (origin_x+x_start) / 8;
    int bit = (origin_x+x_start) % 8;

    unsigned char *start_row = byte;
    int start_bit = bit;

    /* iteration vars */
    int row;
    int col;

    int num_neighbors;

    for (row = y_start; row < y_end; row++) {
        for (col = x_start; col < x_end; col++) {
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
 * There is no origin_x parameter because it always starts from x = 0
 */
void fill_neighbor_matrix_strip(int origin_y)
{
    unsigned char *byte
        = plotSScreen + origin_y*SCREEN_WIDTH_BYTES;
    int bit = 0;

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
 * doesn't set the top or bottom rows
 * This assumes appBackUpScreen contains their current values
 */
void load_neighbor_matrix_strip(int origin_y)
{
    unsigned char *byte
        = appBackUpScreen + (origin_y+1) * SCREEN_WIDTH_BYTES;
    int bit = 0;

    unsigned char *start_row = byte;

    /* iteration vars */
    int row;
    int col;

    int num_neighbors;

    for (row = 1; row < STRIP_HEIGHT-1; row++) {
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

#define STRIP_TOP 28

/*
 * Advances the game by one step
 */
void take_step()
{
    /* which block we're on */
    int block_row = 0;
    int block = 0;

    /* top-left corner of the current block */
    int origin_y = 0;
    int origin_x = 0;

    /* what rows of the block to actually print */
    /* for the upper blocks, we leave out the bottom row */
    int y_start = 0;
    int y_end = BLOCK_HEIGHT - 1;

    /* iteration vars */
    int i;
    int j;

    memset(saveSScreen, 0, BUFFER_SIZE);

    /* this loop executes twice: once for each row of blocks */
    while (block_row < 2) {
        /* first three blocks */
        while (block < 3) {
            /* update this part of the screen */
            fill_neighbor_matrix(origin_y, origin_x, false);
            load_neighbor_matrix(origin_y, origin_x,
                    y_start, y_end, 0, BLOCK_WIDTH - 1);

            /* copy over the values on the rightmost column */
            for (i = 0; i < BLOCK_HEIGHT; i++)
                neighbor_matrix[i][0] = neighbor_matrix[i][BLOCK_WIDTH-1];

            /* delete the rest */
            for (i = 0; i < BLOCK_HEIGHT; i++)
                for (j = 1; j < BLOCK_WIDTH; j++)
                    neighbor_matrix[i][j] = 0;

            /* advance to the next block */
            origin_x += BLOCK_WIDTH-1;
            block++;
        }
        /* final block */
        fill_neighbor_matrix(origin_y, origin_x, true);
        load_neighbor_matrix(origin_y, origin_x,
                y_start, y_end, 0, BLOCK_WIDTH);
        memset(saveSScreen, 0, BUFFER_SIZE);

        /* move to the second row of blocks */
        origin_y = SCREEN_HEIGHT - BLOCK_HEIGHT;
        /* don't print the top row */
        y_start = 1;
        y_end = BLOCK_HEIGHT - 1;
        origin_x = 0;
        block_row++;
        block = 0;
    }

    /* finally the strip in the middle */
    fill_neighbor_matrix_strip(STRIP_TOP);
    load_neighbor_matrix_strip(STRIP_TOP);

    memcpy(plotSScreen, appBackUpScreen, BUFFER_SIZE);
    FastCopy();
}

enum State {RUNNING, PAUSED, DONE};

int main()
{
    unsigned char sk;
    int i;

    enum State state = PAUSED;

    DisableAPD();

    memset(saveSScreen, 0, BUFFER_SIZE);
    memcpy(appBackUpScreen, plotSScreen, BUFFER_SIZE);
    FastCopy();


    while (state != DONE) {
        sk = GetCSC();
        switch (state) {
        case RUNNING:
            if (sk == skEnter)
                state = PAUSED;
            take_step();
            break;
        case PAUSED:
            if (sk == skAdd)
                state = RUNNING;
            else if (sk == skSub)
                take_step();
            else if (sk == skClear)
                state = DONE;
            break;
        }
    }

    curRow = 0;
    ClrLCDFull();

    EnableAPD();

    return 0;
}

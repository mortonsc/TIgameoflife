#include <stdbool.h>
#include <string.h>
#include "inc/ti84plus.h"
#include "inc/fastcopy.h"


/*
 * Copyright (c) 2016 Scott Morton
 * This is free software released under the MIT license.
 * For more information, see the LICENSE.txt file
 * which should be bundled with this source.
 */


/*
 * EXPLANATION OF ALGORITHM:
 * The basic idea is as follows: have a matrix with an element for each pixel.
 * That element is the number of live neighbor cells that pixel has.
 * To advance a step, iterate over all the pixels, and for each live one,
 * increment the matrix element for each of its neighbors. Then, to update
 * the board, check the matrix for each pixel to see how many live neighbors
 * it has, and therefore what should happen to it.
 *
 * The TI84+ doesn't have enough RAM available for this matrix. So instead,
 * we divide the screen up into 9 horizontal strips, and generate the matrix
 * for each of them, one at a time, so we can reuse the same memory for each
 * matrix.
 *
 */

#define SCREEN_WIDTH_BYTES 12

#define STRIP_HEIGHT 8
#define STRIP_WIDTH 96

/*
 * matrix located in appBackUpScreen
 * it would be more honest to cast appBackUpScreen to an array
 * but I couldn't figure out how to do that
 */
__at 0x9872 unsigned char neighbor_matrix[STRIP_HEIGHT][STRIP_WIDTH];

/*
 * Fills neighbor matrix with the number of live neighbors of each pixel
 * in the strip starting with top_row
 *
 * If complete_last_row is false, does not completely generate the last row
 * of the matrix. This is done in such a way that the last row can then be
 * passed as the first row to the next strip, and it will have the correct
 * final result.
 */
void generate_neighbor_matrix(int top_row, bool complete_last_row)
{
    unsigned char *byte = plotSScreen + top_row*SCREEN_WIDTH_BYTES;
    unsigned char mask = 0x80;

    int end_row = complete_last_row ? STRIP_HEIGHT : STRIP_HEIGHT - 1;

    /* iteration vars */
    int row;
    int col;
    int i;
    int j;

    for (row = 0; row < end_row; row++) {
        for (col = 0; col < STRIP_WIDTH; col++) {
            if (*byte & mask) {
                i = (row > 0) ? row-1 : row;
                for (; i <= row+1 && i < STRIP_HEIGHT; i++) {
                    j = (col > 0) ? col-1 : col;
                    for (; j <= col+1 && j < STRIP_WIDTH; j++) {
                        neighbor_matrix[i][j]++;
                    }
                }
                /* the above loop counts a cell as its own neighbor */
                neighbor_matrix[row][col]--;
            }
            mask >>= 1;
            if (!mask) {
                mask = 0x80;
                byte++;
                /* skip over empty bytes */
                while (!(*byte) && col < STRIP_WIDTH-1) {
                    byte++;
                    col += 8;
                }
            }
        }
    }

    if (complete_last_row)
        return;

    /* partial final row */
    i = STRIP_HEIGHT-2;
    for (col = 0; col < STRIP_WIDTH; col++) {
        if (*byte & mask) {
            j = (col > 0) ? col-1 : col;
            for (; j <= col+1 && j < STRIP_WIDTH; j++)
                neighbor_matrix[i][j]++;
        }
        mask >>= 1;
        if (!mask) {
            mask = 0x80;
            byte++;
            /* skip over empty bytes */
            while (!(*byte) && col < STRIP_WIDTH-1) {
                byte++;
                col += 8;
            }
        }
    }
}

/*
 * Updates the strip of plotSScreen with the given top row according to the
 * contents of the neighbor_matrix.
 *
 * If load_last_row is false, does not update the last row.
 */
void load_neighbor_matrix(int top_row, bool load_last_row)
{
    unsigned char *byte = plotSScreen + top_row*SCREEN_WIDTH_BYTES;
    unsigned char mask = 0x80;

    int end_row = load_last_row ? STRIP_HEIGHT : STRIP_HEIGHT - 1;

    /* iteration vars */
    int row;
    int col;

    int num_neighbors;

    for (row = 0; row < end_row; row++) {
        for (col = 0; col < STRIP_WIDTH; col++) {
            num_neighbors = neighbor_matrix[row][col];
            if ((*byte & mask) && (num_neighbors < 2 || num_neighbors > 3))
                *byte ^= mask;
            else if (num_neighbors == 3)
                *byte |= mask;

            mask >>= 1;
            if (!mask) {
                mask = 0x80;
                byte++;
            }
        }
    }
}

/*
 * Advances the game by one step
 */
void take_step()
{
    /* which block we're on */
    int top_row;
    int end_row = 8 * (STRIP_HEIGHT - 1);

    /* iteration vars */
    int i;
    int j;

    for (top_row = 0; top_row < end_row; top_row += STRIP_HEIGHT-1) {
        generate_neighbor_matrix(top_row, false);
        load_neighbor_matrix(top_row, false);

        for (j = 0; j < STRIP_WIDTH; j++) {
            neighbor_matrix[0][j]
                = neighbor_matrix[STRIP_HEIGHT-1][j];
        }
        for (i = 1; i < STRIP_HEIGHT; i++)
            for (j = 0; j < STRIP_WIDTH; j++)
                neighbor_matrix[i][j] = 0;
    }

    generate_neighbor_matrix(top_row, true);
    load_neighbor_matrix(top_row, true);

    FastCopy();
    memset(appBackUpScreen, 0, BUFFER_SIZE);
}

enum State {RUNNING, PAUSED, DONE};

int main()
{
    unsigned char sk;

    enum State state = PAUSED;

    memset(appBackUpScreen, 0, BUFFER_SIZE);
    FastCopy();

    while (state != DONE) {
        sk = GetCSC();
        switch (state) {
        case RUNNING:
            RunIndicatorOff();
            if (sk == skEnter)
                state = PAUSED;
            take_step();
            break;
        case PAUSED:
            RunIndicatorOn();
            if (sk == skAdd)
                state = RUNNING;
            else if (sk == skSub)
                take_step();
            else if (sk == skClear)
                state = DONE;
            break;
        }
    }

    ClrLCDFull();
    curRow = 0;

    return 0;
}

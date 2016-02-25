/*
 * An implementation of John Conway's Game of Life
 * for TI-83+ series graphing calculators.
 *
 * Copyright (C) 2016 Scott Morton
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
 */

#include <stdbool.h>
#include <string.h>
#include "lib/c_ti83p.h"

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


/*
 * Polls the user for a picture number.
 * Returns 0 if a non-numeric key is pressed.
 */
int get_pic_no()
{
    unsigned char key = CGetKey();
    switch (key) {
    case k0:
        return 10;
    case k1:
        return 1;
    case k2:
        return 2;
    case k3:
        return 3;
    case k4:
        return 4;
    case k5:
        return 5;
    case k6:
        return 6;
    case k7:
        return 7;
    case k8:
        return 8;
    case k9:
        return 9;
    default:
        return 0;
    }
}

enum State {RUNNING, PAUSED, DONE};

int main()
{
    unsigned char sk;
    int pic_no;

    enum State state = PAUSED;

    CEnable15MHz();

    memset(appBackUpScreen, 0, BUFFER_SIZE);
    FastCopy();

    while (state != DONE) {
        sk = CGetCSC();
        switch (state) {
        case RUNNING:
            if (sk == skEnter) {
                state = PAUSED;
                CRunIndicatorOn();
            }
            take_step();
            break;
        case PAUSED:
            if (sk == skAdd) {
                state = RUNNING;
                CRunIndicatorOff();
            } else if (sk == skSub) {
                take_step();
            } else if (sk == skStore) {
                /* store screen to pic */
                pic_no = get_pic_no();
                if (pic_no != 0)
                    memcpy(CCreatePic(pic_no), plotSScreen, PIC_SIZE_BYTES);
            } else if (sk == skLn) {
                /* load a pic */
                pic_no = get_pic_no();
                if (pic_no != 0) {
                    memcpy(plotSScreen, CRecallPic(pic_no), PIC_SIZE_BYTES);
                    /* pic vars don't include the last row, so clear it */
                    memset(plotSScreen + 63*SCREEN_WIDTH_BYTES,
                            0, SCREEN_WIDTH_BYTES);
                }
                FastCopy();
            } else if (sk == skClear) {
                state = DONE;
            }
            break;
        }
    }

    /* The OS seems to do this anyways, but it's good to be safe */
    CDisable15MHz();

    return 0;
}

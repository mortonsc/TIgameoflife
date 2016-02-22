#include <stdbool.h>
#include <string.h>
#include "inc/ti84plus.h"
#include "inc/fastcopy.h"


/*
 * Code by Scott Morton 2016
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

#define STRIP_HEIGHT 8
#define STRIP_WIDTH 96

/*
 * matrix located in appBackUpScreen
 * it would be more honest to cast appBackUpScreen to an array
 * but I couldn't figure out how to do that
 */
__at 0x9872 unsigned char neighbor_matrix_strip[STRIP_HEIGHT][STRIP_WIDTH];

/*
 * Fills neighbor_matrix_strip with the number of neighbors of each cell
 * along the middle strip.
 */
void generate_neighbor_matrix_strip(int top_row, bool complete_last_row)
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
                        neighbor_matrix_strip[i][j]++;
                    }
                }
                /* the above loop counts a cell as its own neighbor */
                neighbor_matrix_strip[row][col]--;
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

    /* partial final row */
    if (!complete_last_row) {
        i = STRIP_HEIGHT-2;
        for (col = 0; col < STRIP_WIDTH; col++) {
            if (*byte & mask) {
                j = (col > 0) ? col-1 : col;
                for (; j <= col+1 && j < STRIP_WIDTH; j++)
                    neighbor_matrix_strip[i][j]++;
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
}

/*
 * Based on the values in neighbor_matrix_strip, updates the cells in appBackUpScreen
 * along the middle strip
 *
 * The only pixels set are those that correspond to elements in neighbor_matrix
 * within the rectangle bounded by 0, x_end, y_start, y_end
 * (start values inclusive, end values exclusive).
 *
 */
void load_neighbor_matrix_strip(int top_row, bool load_last_row)
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
            num_neighbors = neighbor_matrix_strip[row][col];
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
        generate_neighbor_matrix_strip(top_row, false);
        load_neighbor_matrix_strip(top_row, false);

        for (j = 0; j < STRIP_WIDTH; j++) {
            neighbor_matrix_strip[0][j]
                = neighbor_matrix_strip[STRIP_HEIGHT-1][j];
        }
        for (i = 1; i < STRIP_HEIGHT; i++)
            for (j = 0; j < STRIP_WIDTH; j++)
                neighbor_matrix_strip[i][j] = 0;
    }

    generate_neighbor_matrix_strip(top_row, true);
    load_neighbor_matrix_strip(top_row, true);
    memset(appBackUpScreen, 0, BUFFER_SIZE);

    FastCopy();
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

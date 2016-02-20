#include <stdbool.h>
#include <string.h>
#include "inc/ti84plus.h"
#include "inc/fastcopy.h"

/*
 * Code by Scott Morton 2016
 */

void assert(int cond, const char *err)
{
    if (!cond) {
        VPutS(err);
        /* pause so I can actually see the error */
        GetKey();
    }
}

/*
 * Sets the bit of appBackUpScreen representing the xth pixel of the yth row
 */
void set_pixel(int y, int x, bool is_on)
{

    unsigned char *byte = appBackUpScreen + (y*SCREEN_WIDTH + x/8);
    /* ex. if x % 8 = 3, mask is 0010 0000b */
    unsigned char mask = 0x80 >> (x % 8);

    if (y < 0 || y >= SCREEN_HEIGHT || x < 0 || x >= SCREEN_WIDTH)
        return;

    if (is_on) {
        /* set the (x % 8)th bit to 1, without affecting the others */
        *byte |= mask;
    } else {
        /* set the (x % 8)th bit to 0, without affecting the others */
        *byte &= ~mask;
    }
}

/*
 * Returns the value of the bit of plotSScreen at the given position
 */
bool get_pixel(int y, int x)
{
    unsigned char *byte = plotSScreen + (y*SCREEN_WIDTH + x/8);
    /* ex. if x % 8 = 3, mask is 0010 0000b */
    unsigned char mask = 0x80 >> (x % 8);

    /* "off-screen" pixels are always off */
    if (y < 0 || y >= SCREEN_HEIGHT || x < 0 || x >= SCREEN_WIDTH)
        return false;

    return *byte & mask;
}

void take_step()
{
    /* iteration vars */
    int y;
    int x;
    int i;
    int j;
    int live_neighbors;

    /*
     * iterate over every pixel of the screen to determine it's new state
     */
    for (y = 0; y < SCREEN_HEIGHT; y++) {
        for (x = 0; x < SCREEN_WIDTH; x++) {
            live_neighbors = 0;
            for (i = -1; i <= 1; i++)
                for (j = -1; j <= 1; j++)
                    if (get_pixel(y+i, x+j))
                        live_neighbors++;
            /*
             * the loop above will count the current pixel as its own neighbor
             * so we must compensate
             */
            if (get_pixel(y, x))
                live_neighbors--;
            switch(live_neighbors) {
            case 0:
            case 1:
                /* death by loneliness */
                set_pixel(y, x, false);
                break;
            case 2:
                break;
            case 3:
                /* reproduction */
                set_pixel(y, x, true);
                break;
            default:
                /* death by overcrowding */
                set_pixel(y, x, false);
                break;
            }
        }
    }

    /* copy in the new values */
    memcpy(plotSScreen, appBackUpScreen, BUFFER_SIZE);
}

int main()
{
    return 0;
}

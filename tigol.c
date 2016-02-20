#include <stdbool.h>
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
 * Sets the bit of plotSScreen representing the xth pixel of the yth row
 */
void set_pixel(int y, int x, bool is_on)
{

    unsigned char *byte = plotSScreen + (y*SCREEN_WIDTH + x/8);
    /* ex. if x % 8 = 3, mask is 0010 0000b */
    unsigned char mask = 0x80 >> (x % 8);

    assert((0 <= y && y < SCREEN_HEIGHT), "invalid y value");
    assert((0 <= x && x < SCREEN_WIDTH), "invalid x value");

    if (is_on) {
        /* set the (x % 8)th bit to 1, without affecting the others */
        *byte |= mask;
    } else {
        /* set the (x % 8)th bit to 0, without affecting the others */
        mask &= ~mask;
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

    assert((0 <= y && y < SCREEN_HEIGHT), "invalid y value");
    assert((0 <= x && x < SCREEN_WIDTH), "invalid x value");

    return *byte & mask;
}

void take_step()
{

}

int main()
{
    return 0;
}

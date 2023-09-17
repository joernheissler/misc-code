/*
 * Brute force solver for calendar puzzle:
 *
 *  0   1   2   3   4   5   6
 ******************************
 * Jan Feb Mar Apr May Jun **** 0
 * Jul Aug Sep Oct Nov Dec **** 1
 *  1   2   3   4   5   6   7 * 2
 *  8   9  10  11  12  13  14 * 3
 * 15  16  17  18  19  20  21 * 4
 * 22  23  24  25  26  27  28 * 5
 ********* 29  30  31  ******** 6
 ******************************

 ###  ##   ##   #    ###
 ##    #   ##   ###   #
       ##     #
             ###
 # #   ##     #     #
 ###    ##       ####
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

const uint64_t ONE = 1;

#define COL_NORMAL "\x1b[0m"

static void block(uint8_t r, uint8_t g, uint8_t b)
{
    printf("\x1b[48;2;%" PRIu8 ";%" PRIu8 ";%" PRIu8 "m  " COL_NORMAL, r, g, b);
}


const char *BOARD =
    "      #"
    "      #"
    "       "
    "       "
    "       "
    "       "
    "##   ##";

struct tile {
    unsigned rows;
    uint8_t r, g, b;
    const char *bits;
};

const struct tile TILES[] = {
    {2, 0xbd, 0xee, 0xbc,
     "###"
     "## "},

    {3, 0x9e, 0xd4, 0xe4,
     "## "
     " # "
     " ##"},

    {2, 0x4a, 0x79, 0xe0,
     "##"
     "##"},

    {2, 0xaf, 0x12, 0xa0,
     "#  "
     "###"},

    {2, 0xff, 0xdb, 0x67,
     "###"
     " # "},

    {2, 0xec, 0x19, 0x21,
     "# #"
     "###"},

    {2, 0x62, 0xaf, 0x57,
     "## "
     " ##"},

    {3, 0xff, 0xac, 0xca,
     " # "
     "###"
     " # "},

    {2, 0xff, 0x92, 0x20,
     "   #"
     "####"},
};

#define NTILES (sizeof TILES / sizeof *TILES)

uint64_t board = 0;

struct variant {
    unsigned rows;
    unsigned cols;
    uint8_t r,g,b;
    uint64_t bits;
};

struct variant variants[NTILES][8];

static struct variant rotate(struct variant v)
{
    uint64_t bits = 0, b = 1;
    for (unsigned col = 0; col < v.cols; ++col) {
        for (unsigned row = 0; row < v.rows; ++row) {
            if (v.bits & (ONE << ((v.rows - row - 1) * 7 + col))) {
                bits |= b;
            }
            b <<= 1;
        }
        b <<= 7 - v.rows;
    }
    return (struct variant){v.cols, v.rows, v.r, v.g, v.b, bits};
}

static struct variant mirror(struct variant v)
{
    uint64_t bits = 0, b = 1;
    for (unsigned row = 0; row < v.rows; ++row) {
        for (unsigned col = 0; col < v.cols; ++col) {
            if (v.bits & (ONE << (row * 7 + (v.cols - col - 1)))) {
                bits |= b;
            }
            b <<= 1;
        }
        b <<= 7 - v.cols;
    }
    return (struct variant){v.rows, v.cols, v.r, v.g, v.b, bits};
}

void print_variant(struct variant v)
{
    for (unsigned row = 0; row < v.rows; ++row) {
        for (unsigned col = 0; col < v.cols; ++col) {
            if (v.bits & (ONE << (row * 7 + col))) {
                block(v.r, v.g, v.b);
                //printf("#");
            } else {
                printf("  ");
            }
        }
        puts("");
    }
    puts("");
}

static void init(void)
{
    uint64_t b = 1;

    /* convert BOARD */
    for (const char *c = BOARD; *c; ++c) {
        if (*c != ' ') {
            board |= b;
        }
        b <<= 1;
    }
    for (size_t i = 0; i < NTILES; ++i) {
        const struct tile *t = TILES + i;
        if (strlen(t->bits) % t->rows) {
            abort();
        }
        unsigned cols = strlen(t->bits) / t->rows;
        b = 1;
        const char *c = t->bits;
        uint64_t bits = 0;
        for (unsigned row = 0; row < t->rows; ++row) {
            for (unsigned col = 0; col < cols; ++col) {
                if (*c != ' ') {
                    bits |= b;
                }
                ++c;
                b <<= 1;
            }
            b <<= (7 - cols);
        }
        struct variant cur = (struct variant){t->rows, cols, t->r, t->g, t->b, bits};
        unsigned vcount = 0;
        variants[i][vcount++] = cur;
        for (int r = 0; r < 4; ++r) {
            cur = rotate(cur);
            unsigned tmp;
            for (tmp = 0; tmp < vcount; ++tmp) {
                if (variants[i][tmp].rows != cur.rows) continue;
                if (variants[i][tmp].cols != cur.cols) continue;
                if (variants[i][tmp].bits != cur.bits) continue;
                break;
            }
            if (tmp == vcount) {
                variants[i][vcount++] = cur;
            }
        }
        cur = mirror(cur);
        for (int r = 0; r < 4; ++r) {
            cur = rotate(cur);
            unsigned tmp;
            for (tmp = 0; tmp < vcount; ++tmp) {
                if (variants[i][tmp].rows != cur.rows) continue;
                if (variants[i][tmp].cols != cur.cols) continue;
                if (variants[i][tmp].bits != cur.bits) continue;
                break;
            }
            if (tmp == vcount) {
                variants[i][vcount++] = cur;
            }
        }
        while (vcount < 8) {
            variants[i][vcount++] = (struct variant){0,0,0,0,0,0};
        }
    }
}

static struct stack {
    uint16_t tile:4;
    uint16_t var:3;
    uint16_t pos:6;
} stack[NTILES];

static void solve(unsigned pos, unsigned tiles, unsigned placed)
{
    if (placed == 9) {
        // success
        uint8_t colors[49][3] = {0};
        for (struct stack *s = stack; s < stack + NTILES; ++s) {
            struct variant var = variants[s->tile][s->var];
            uint64_t tmp = var.bits << s->pos;
            for (unsigned i = 0; i < 49; ++i) {
                if (tmp & (ONE << i)) {
                    colors[i][0] = var.r;
                    colors[i][1] = var.g;
                    colors[i][2] = var.b;
                }
            }
        }
        for (unsigned i = 0; i < 49; ++i) {
            block(colors[i][0], colors[i][1], colors[i][2]);
            if (i % 7 == 6) puts("");
        }
        puts("----");
        return;
    }
    // give up
    if (pos == 40) return;

    // recurse with zero tile
    if (board & (ONE << pos)) {
        solve(pos + 1, tiles, placed);
    }

    // no tile has only one column.
    if (pos % 7 == 6) return;

    for (unsigned tile = 0; tile < NTILES; ++tile) {
        // tile already placed
        if (tiles & (1 << tile)) continue;

        tiles |= 1 << tile;

        for (struct variant *var = variants[tile]; var < 8 + variants[tile] && var->rows; ++var) {
            // tile too wide
            if (pos % 7 + var->cols > 7) continue;

            // tile too high
            if (pos / 7 + var->rows > 7) continue;

            // tile overlaps
            if (board & (var->bits << pos)) continue;

            // tile doesn't fill current pos
            if (!(board & (ONE << pos)) && ! (var->bits & 1)) continue;

            // place tile on board
            board |= var->bits << pos;

            // remember where we placed it
            stack[placed] = (struct stack){tile, var - variants[tile], pos};

            // recurse
            solve(pos + 1, tiles, placed + 1);

            // remove tile from board
            board &= ~(var->bits << pos);
        }
        tiles &= ~(1 << tile);
    }
}

int main(int argc, char **argv)
{
    /* 1..12 */
    int mon;

    /* 1..31 */
    int day;

    if (argc == 1) {
        time_t now = time(NULL);
        struct tm *tm = localtime(&now);
        mon = tm->tm_mon + 1;
        day = tm->tm_mday;
    } else if (argc == 3) {
        mon = atoi(argv[1]);
        day = atoi(argv[2]);
    } else {
        fprintf(stderr, "Usage: %s <month> <day>\n", argv[0]);
        exit(2);
    }

    if (mon < 1 || mon > 12) {
        fprintf(stderr, "Month %d out of range\n", mon);
        exit(2);
    }

    if (day < 1 || day > 31) {
        fprintf(stderr, "Day %d out of range\n", day);
        exit(2);
    }

    init();
    board |= ONE << (mon > 6 ? mon : mon - 1);
    board |= ONE << (day > 28 ? day + 15 : day + 13);

    solve(0, 0, 0);
}

#include "stdio.h"
#include "font.h"

void render_glyph(unsigned char *glyph) {
    for (int row = 0; row < 8; row++) {
        unsigned char byte = glyph[row]; // one byte per row (6px + 2 padding bits)

        for (int col = 0; col < 8; col++) {
            int bit = (byte >> (7 - col)) & 1;
            printf(bit ? "▉" : " ");
        }
        printf("\n");
    }
}

int main() {
    render_glyph(glyphs[0]);
    return 0;
}

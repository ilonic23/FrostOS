/* FrostOS font generator:
 * Create a font with all your glyphs, save it to png
 * and convert it to FrostOS compatible font header to use!
 * License? MIT, like all the project.
*/

#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

int main(int argc, char **argv) {
    if (argc != 5) {
        printf("Usage: %s <input.png> <output.h> <width> <height>\n", argv[0]);
        return 1;
    }

    int glyph_width = atoi(argv[3]);
    int glyph_height = atoi(argv[4]);
    int width, height, channels;

    unsigned char *pixels = stbi_load(argv[1], &width, &height, &channels, 4);
    if (!pixels) {
        fprintf(stderr, "Error: %s\n", stbi_failure_reason());
        return 1;
    }

    FILE *output = fopen(argv[2], "w");
    int bytes_per_glyph = (glyph_width * glyph_height + 7) / 8;
    fprintf(output, "unsigned char glyphs[][%d] = {\n", bytes_per_glyph);

    for (int glyph = 0; glyph < width / glyph_width; glyph++) {
        fprintf(output, "    { ");
        
        unsigned char current_byte = 0;
        int bits_filled = 0;

        for (int y = 0; y < glyph_height; y++) {
            for (int x = 0; x < glyph_width; x++) {
                int px_x = x + (glyph * glyph_width);
                unsigned char *p = pixels + (y * width + px_x) * 4;
                
                int is_active = (p[0] < 128); 

                if (is_active) {
                    current_byte |= (1 << (7 - bits_filled));
                }

                bits_filled++;

                if (bits_filled == 8) {
                    fprintf(output, "0x%02X, ", current_byte);
                    current_byte = 0;
                    bits_filled = 0;
                }
            }
        }
        
        if (bits_filled > 0) {
            fprintf(output, "0x%02X, ", current_byte);
        }

        fprintf(output, "},\n");
    }

    fprintf(output, "};\n");
    fclose(output);
    stbi_image_free(pixels);
    return 0;
}

/* FrostOS font generator:
 * Create a font with all your glyphs, save it to png
 * and convert it to FrostOS compatible font header to use!
 * License? MIT, like all the project.
*/

/*#include "stdio.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG // Accept only pngs
#include "stb_image.h"

int main(int argc, char **argv) {
    if (argc != 5) {
        printf("Usage: %s file_path output_path glyph_width glyph_height", argv[0]);
        return 0;
    }
    int glyph_width, glyph_height;
    sscanf(argv[3], "%d", &glyph_width);
    sscanf(argv[4], "%d", &glyph_height);

    int width, height, channels;

    unsigned char *pixels = stbi_load(argv[1], &width, &height, &channels, 4);
    if (!pixels) {
        fprintf(stderr, "Failed to load image: %s\n", stbi_failure_reason());
        return 1;
    }
    
    if (width % glyph_width != 0)
        fprintf(stderr, "Warning: The width of the image is not divisible by glyph width.\n");
    if (height != glyph_height)
        fprintf(stderr, "Warning: The height of the image is not equal to the glyph height.\n");

    FILE *output = fopen(argv[2], "wb");
    if (!output) {
        stbi_image_free(pixels);
        fprintf(stderr, "Failed to open output file: %s\n", argv[2]);
        perror("Error");
        return 2;
    }

    fprintf(output, "unsigned char glyphs[][%d] = {\n", glyph_width*glyph_height/8);

    /*for (int glyph = 0; glyph < width/glyph_width; glyph++) {
        fprintf(output, "{ ");
        for (int x = 0; x < glyph_width; x++) {
            for (int y = 0; y < glyph_height; y++) {
                unsigned char *pixel = pixels + (y * width + (x+glyph*glyph_width)) * 4;
                int r,g,b;
                r = pixel[0];
                g = pixel[1];
                b = pixel[2];

                if (r == 0 && g == 0 && b == 0) {
                    
                }
            }
        }
        fprintf(output, "}, \n");
    }
    for (int glyph = 0; glyph < width / glyph_width; glyph++) {
        fprintf(output, "    { ");

        unsigned char byte = 0;
        int bit_index = 0;

        for (int y = 0; y < glyph_height; y++) {
            for (int x = 0; x < glyph_width; x++) {
                unsigned char *pixel = pixels + (y * width + (x + glyph * glyph_width)) * 4;
                int is_black = (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0);

                // Pack bit into current byte, MSB first
                if (is_black)
                    byte |= (1 << (7 - bit_index));

                bit_index++;

                if (bit_index == 8) {
                    fprintf(output, "0b%b,\n", byte);
                    byte = 0;
                    bit_index = 0;
                }
            }
        }

        // Flush any remaining bits (if glyph_width*glyph_height is not a multiple of 8)
        if (bit_index > 0)
            fprintf(output, "0b%b,\n", byte);

        fprintf(output, "},\n");
    }

    fprintf(output, "};\n");

    fclose(output);
    stbi_image_free(pixels);
}*/
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
    fprintf(output, "// Generated font header\n");
    // Calculate required bytes per glyph
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
                
                // Simple threshold: treat as "on" if any color channel is low (black)
                // Or check if Alpha is high if it's a transparent PNG
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
        
        // Final flush for the glyph
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

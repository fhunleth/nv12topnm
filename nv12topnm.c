#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>

static void usage(const char *name)
{
    fprintf(stderr, "%s [-w <width>] [-h <height>] [-b <bytes per line>] [-l] [-o <output filename>] <input file>\n", name);
    fprintf(stderr, "Converts a raw NV12 (YCbCr 4:2:0) buffer to a PNM.\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "-l luminance only\n");
    fprintf(stderr, "-c chrominance only\n");
    fprintf(stderr, "-s swap cb and cr\n");
}

static int constrain(int low, int value, int high)
{
    if (value < low)
        return low;
    else if (value > high)
        return high;
    else
        return value;
}

int main(int argc, char *argv[])
{
    FILE *input_file;
    FILE *output_file;
    int opt;
    int width = 0;
    int height = 0;
    int bytes_per_line = 0;
    int luminance_only = 0;
    int chrominance_only = 0;
    int swap_chrominance = 0;
    int image_size;
    int luminance_size;
    const char *output_filename = 0;
    const char *input_filename = 0;
    unsigned char *buffer;
    int column;
    int row;

    while ((opt = getopt(argc, argv, "w:h:b:o:lcs")) != -1) {
        switch (opt) {
        case 'w':
            width = atoi(optarg);
            break;
        case 'h':
            height = atoi(optarg);
            break;
        case 'b':
            bytes_per_line = atoi(optarg);
            break;
        case 'o':
            output_filename = optarg;
            break;
        case 'l':
            luminance_only = 1;
            break;
        case 'c':
            chrominance_only = 1;
            break;
        case 's':
            swap_chrominance = 1;
            break;
        default:
            usage(argv[0]);
            exit(-1);
        }
    }

    if (optind < argc)
        input_filename = argv[optind];

    if (width <= 0 || height <= 0) {
        fprintf(stderr, "ERROR: Must specify width and height of input image\n");
        usage(argv[0]);
        exit(-1);

    }
    if (bytes_per_line <= 0)
        bytes_per_line = width;

    if (input_filename == 0 || strcmp(input_filename, "-") == 0)
        input_file = stdin;
    else
        input_file = fopen(input_filename, "rb");
    if (input_file == 0) {
        fprintf(stderr, "Error opening %s\n", input_filename);
        exit(-1);
    }

    if (output_filename == 0 || strcmp(output_filename, "-") == 0)
        output_file = stdout;
    else
        output_file = fopen(output_filename, "wb");
    if (output_file == 0) {
        fprintf(stderr, "Error opening %s\n", output_filename);
        exit(-1);
    }

    image_size = bytes_per_line * height * 3 / 2;
    buffer = (unsigned char *) malloc(image_size);
    if (buffer == 0) {
        fprintf(stderr, "Not enough memory\n");
        exit(-1);
    }

    if (fread(buffer, 1, image_size, input_file) != image_size) {
        fprintf(stderr, "Input file not big enough\n");
        exit(-1);
    }

    fclose(input_file);

    fprintf(output_file, "P6\n%d %d\n255\n", width, height);

    luminance_size = bytes_per_line * height;
    for (row = 0; row < height; row++) {
        for (column = 0; column < width; column++) {
            unsigned char rgb[3];
            int y, cb, cr;
            int r, g, b;

            if (!chrominance_only)
                y = buffer[row * bytes_per_line + column];
            else
                y = 128;

            if (!luminance_only) {
                cb = buffer[luminance_size + (row / 2 * bytes_per_line) + (column & ~1)] - 128;
                cr = buffer[luminance_size + (row / 2 * bytes_per_line) + (column | 1)] - 128;

                if (swap_chrominance) {
                    int tmp = cb;
                    cb = cr;
                    cr = tmp;
                }
            }

            r = y + 91881 * cr / 65536;
            g = y - (22572 * cb + 46802 * cr) / 65536;
            b = y + 116130 * cb / 65536;

            r = constrain(0, r, 255);
            g = constrain(0, g, 255);
            b = constrain(0, b, 255);

            rgb[0] = (unsigned char) r;
            rgb[1] = (unsigned char) g;
            rgb[2] = (unsigned char) b;

            fwrite(rgb, 3, 1, output_file);
        }
    }

    fclose(output_file);
}

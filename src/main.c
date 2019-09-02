/** #include <getopt.h> */
#include <stdio.h>
#include <vips/vips.h>

char* output_size;
char* output_format;

GOptionEntry options[] = {
    {"size", 's', 0, G_OPTION_ARG_STRING, &output_size, "shrink to SIZE or to WIDTHxHEIGHT",
     "SIZE"},
    {"output", 'o', G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_STRING, &output_format,
     "set output to FORMAT", "FORMAT"},
};

int main(int argc, char** argv)
{
    VipsImage* in;
    VipsImage* out;
    double mean;
    int in_width;
    int in_height;
    char* in_name;

    if (VIPS_INIT(argv[0])) {
        vips_error_exit("Unable to start VIPS");
    }
    if (argc < 3) {
        vips_error_exit("missing required parameters\nusage: %s infile outfile", argv[0]);
    }

    if (!(in = vips_image_new_from_file(argv[1], NULL))) {
        vips_error_exit(NULL);
    }

    in_name = g_path_get_basename(argv[1]);
    in_width = vips_image_get_width(in);
    in_height = vips_image_get_height(in);

    printf("Input file: %s\n", in_name);
    printf("image width = %d\n", in_width);
    printf("image height = %d\n", in_height);
    g_info("Image dims: %d x %d", in_width, in_height);

    if (vips_avg(in, &mean, NULL)) {
        vips_error_exit(NULL);
    }

    printf("mean pixel value = %g\n", mean);

    if (vips_invert(in, &out, NULL)) {
        vips_error_exit(NULL);
    }

    g_object_unref(in);

    if (vips_image_write_to_file(out, argv[2], NULL)) {
        vips_error_exit(NULL);
    }

    g_object_unref(out);

    return (0);
}

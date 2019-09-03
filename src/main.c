#include <stdio.h>
#include <vips/vips.h>

static char* output_format;
static char* output_size;

static GOptionEntry options[] = {
    {"size", 's', 0, G_OPTION_ARG_STRING, &output_size, "shrink to SIZE or to WIDTHxHEIGHT",
     "SIZE"},
    {"output", 'o', G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_STRING, &output_format,
     "set output to FORMAT", "FORMAT"},
};

int
main(int argc, char** argv)
{
    VipsImage* in;
    VipsImage* out;
    double mean;
    int in_width;
    int in_height;
    char* in_name;
    GOptionContext* context;
    GOptionGroup* main_group;
    GError* error = NULL;

    if (VIPS_INIT(argv[0])) {
        vips_error_exit("Unable to start VIPS");
    }
    /** if (argc < 3) { */
    /**     vips_error_exit("missing required parameters\nusage: %s infile outfile", argv[0]); */
    /** } */

    if (!(in = vips_image_new_from_file(argv[1], NULL))) {
        vips_error_exit(NULL);
    }

    context = g_option_context_new(" - Simple image manipulation");
    main_group = g_option_group_new(NULL, NULL, NULL, NULL, NULL);
    g_option_group_add_entries(main_group, options);
    vips_add_option_entries(main_group);
    g_option_context_set_main_group(context, main_group);

#ifdef HAVE_G_WIN32_GET_COMMAND_LINE
    if (!g_option_context_parse_strv(context, &argv, &error))
#else  /*!HAVE_G_WIN32_GET_COMMAND_LINE*/
    if (!g_option_context_parse(context, &argc, &argv, &error))
#endif /*HAVE_G_WIN32_GET_COMMAND_LINE*/
    {
        if (error) {
            fprintf(stderr, "%s\n", error->message);
            g_error_free(error);
        }

        vips_error_exit("try \"%s --help\"", g_get_prgname());
    }

    g_option_context_free(context);

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

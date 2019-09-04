#include <getopt.h>
#include <stdio.h>
#include <vips/vips.h>

static char* prog_version = "0.0.1";

static void
_dummy(const gchar* log_domain, GLogLevelFlags log_level, const gchar* message, gpointer user_data)

{
    /* Dummy does nothing */
    return;
}

int
main(int argc, char** argv)
{
    VipsImage* in = NULL;
    VipsImage* out = NULL;
    double mean;
    int in_width = 0;
    int in_height = 0;
    int verbosity = 0;
    char* in_name = NULL;

    int choice;
    static struct option long_options[] = {
        /* Argument styles: no_argument, required_argument, optional_argument */
        {"input", required_argument, NULL, 'i'}, {"output", required_argument, NULL, 'o'},
        {"verbose", optional_argument, 0, 'v'},  {"version", no_argument, 0, 'V'},
        {"help", no_argument, 0, 'h'},           {0, 0, 0, 0}};

    int option_index = 0;

    while ((choice = getopt_long(argc, argv, "Vhv::i:o:", long_options, &option_index)) != -1) {
        switch (choice) {
        case 'V':
            fprintf(stderr, "%s %s\n", argv[0], prog_version);
            return EXIT_FAILURE;
            break;
        case 'h':
            fputs("help placeholder\n", stderr);
            return EXIT_FAILURE;
            break;
        case 'v':
            if (optarg) {
                verbosity = atoi(optarg);
                break;
            }
            verbosity++;
            break;
        case 'i':
            in_name = optarg;
            break;
        case ':':
            fprintf(stderr, "%s: option `-%c' requires an argument\n", argv[0], optopt);
            break;
        case '?':
            /* getopt_long will have already printed an error */
            break;
        default:
            /* Not sure how to get here... */
            break;
        }
    }

    /* Set dummy for all levels */
    g_log_set_handler(NULL, G_LOG_LEVEL_MASK, _dummy, NULL);
    switch (verbosity) {
    case 0:
        g_log_set_handler(NULL, G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL, g_log_default_handler,
                          NULL);
        break;
    case 1:
        g_log_set_handler(NULL, G_LOG_LEVEL_INFO, g_log_default_handler, NULL);
        break;
    default:
        g_log_set_handler(NULL, G_LOG_LEVEL_DEBUG | G_LOG_LEVEL_INFO, g_log_default_handler, NULL);
        break;
    }
    /** if (verbosity) { */
    /**     setenv("G_MESSAGES_DEBUG", "all", 1); */
    /**     g_info("Verbosity level: %d", verbosity); */
    /** } */

    /* Deal with non-option arguments here */
    if (optind < argc) {
        g_info("non-option ARGV-elements: ");
        while (optind < argc) {
            g_info("%s ", argv[optind++]);
        }
    }

    if (VIPS_INIT(argv[0])) {
        vips_error_exit("Unable to start VIPS");
    }
    if (!(in = vips_image_new_from_file(in_name, NULL))) {
        vips_error_exit(NULL);
    }


    in_name = g_path_get_basename(in_name);
    in_width = vips_image_get_width(in);
    in_height = vips_image_get_height(in);
    g_debug("Input file: %s", in_name);

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

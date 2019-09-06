#include "../config.h"
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <vips/vips.h>

// Command line options
typedef struct
{
    char* input_file;
    char* output_file;
    char* output_file_suffix;
    double pct_scale;
    int width;
    int height;
    int verbosity;
    int no_op;
} options_t;


static const char* help_text =
    "Usage: cimgtool [FLAGS] [OPTIONS] <input_file> [output_file]\n"
    "\n"
    "FLAGS:\n"
    "  -h, --help             Display this help message and exit\n"
    "  -V, --version          Display program version and exit\n"
    "  -v, --verbosity=<N>    Increase console debug message verbosity\n"
    "\n"
    "OPTIONS:\n"
    "  -w, --width            Output width of image\n"
    "  -H, --height           Output height of image\n"
    "  -p, --pct-scale=<N>    Scale output to pct of original size\n"
    "  -s, --suffix=<TEXT>    Suffix to append to file name for edited file\n"
    "\n";

/**
 * @brief Dummy no-op handler for logging
 * @return void
 */
static void
_glog_dummy_handler(const gchar* log_domain, GLogLevelFlags log_level, const gchar* message,
                    gpointer user_data)

{
    return;
}

/**
 * @brief Debug print of options_t struct
 *
 * @param buf Pointer to buffer to fill
 * @param options Pointer to struct
 *
 * @return void
 */
static void
print_options_t(char* buf, size_t bufsize, options_t* options)
{
    if (!buf) return;
    snprintf(buf, bufsize,
             "Options:\n"
             "Input file:        %s\n"
             "Output file:       %s\n"
             "Output suffix:     %s\n"
             "Pct scale:         %.1f\n"
             "Width:             %d\n"
             "Height:            %d\n"
             "Verbosity:         %d\n"
             "No-op:             %d",
             options->input_file, options->output_file, options->output_file_suffix,
             options->pct_scale, options->width, options->height, options->verbosity,
             options->no_op);
}

int
parse_args(int argc, char** argv, options_t* options)
{
    int choice;
    static struct option long_options[] = {{"verbosity", optional_argument, 0, 'v'},
                                           {"version", no_argument, 0, 'V'},
                                           {"help", no_argument, 0, 'h'},
                                           {"suffix", required_argument, 0, 's'},
                                           {"pct-scale", required_argument, 0, 'p'},
                                           {"width", required_argument, 0, 'w'},
                                           {"height", required_argument, 0, 'H'},
                                           {0, 0, 0, 0}};

    int option_index = 0;

    while ((choice = getopt_long(argc, argv, "Vhvnw:H:p:s:", long_options, &option_index)) != -1) {
        switch (choice) {
        case 'V':
            fprintf(stderr, "%s %s\n", argv[0], PACKAGE_VERSION);
            return EXIT_FAILURE;
            break;
        case 'v':
            if (optarg && atoi(optarg)) {
                options->verbosity = atoi(optarg);
            } else {
                options->verbosity++;
            }
            break;
        case 'n':
            options->no_op = 1;
            break;
        case 's':
            options->output_file_suffix = optarg;
            break;
        case 'p':
            options->pct_scale = atof(optarg) / 100;
            break;
        case 'w':
            options->width = atoi(optarg);
            break;
        case 'H':
            options->height = atoi(optarg);
            break;
        case ':':
            fprintf(stderr, "%s: option `-%c' requires an argument\n", argv[0], optopt);
            break;
        case '?':
            fprintf(stderr, "%s: option `-%c' not recognized\n", argv[0], optopt);
            break;
        case 'h':
        default:
            fputs(help_text, stderr);
            exit(1);
            break;
        }
    }
    return 0;
}

int
main(int argc, char** argv)
{
    VipsImage* in = NULL;
    VipsImage* out = NULL;
    char* orig_file_name = NULL;
    char orig_file_ext[8];
    int in_width = 0;
    int in_height = 0;
    int out_width = 0;
    int out_height = 0;

    // Init command line options
    options_t options = {
        .input_file = NULL,
        .output_file = NULL,
        .output_file_suffix = "_edited",
        .pct_scale = 0,
        .width = 0,
        .height = 0,
        .verbosity = 0,
        .no_op = 0,
    };

    if (parse_args(argc, argv, &options)) {
        fprintf(stderr, "%s: error parsing command-line arguments", argv[0]);
        return 1;
    }

    // Logging
    // Set dummy handler for all levels
    // Set logging level based on verbosity flag
    g_log_set_handler(NULL, G_LOG_LEVEL_MASK, _glog_dummy_handler, NULL);
    switch (options.verbosity) {
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
    if (options.verbosity) {
        setenv("G_MESSAGES_DEBUG", "all", 1);
        g_info("Verbosity level: %d", options.verbosity);
    }

    // Deal with positional options
    options.input_file = argv[optind++];
    options.output_file = argv[optind++];

    // Debug print for options
    if (options.verbosity > 1) {
        char buf[250];
        print_options_t(buf, sizeof(buf), &options);
        g_debug("%s", buf);
    }

    if (optind < argc) {
        g_info("Additional non-option ARGV-elements: ");
        while (optind < argc) {
            g_info("%s ", argv[optind++]);
        }
    }
    if (!options.input_file) {
        fprintf(stderr, "%s: input file required\n", argv[0]);
        exit(1);
    }

    if (VIPS_INIT(argv[0])) {
        vips_error_exit("Unable to start VIPS");
    }
    if (!(in = vips_image_new_from_file(options.input_file, NULL))) {
        vips_error_exit(NULL);
    }


    orig_file_name = g_path_get_basename(options.input_file);
    g_strlcpy(orig_file_ext, strrchr(orig_file_name, '.'), sizeof(orig_file_ext));


    if (!options.output_file) {
        char bare_name[strlen(orig_file_name) - strlen(orig_file_ext) + 1];
        char new_name[strlen(options.input_file) + strlen(options.output_file_suffix)];
        g_info("Output file not supplied; using suffix '%s'", options.output_file_suffix);
        g_strlcpy(bare_name, orig_file_name, sizeof(bare_name));
        g_debug("Filename without ext: %s", bare_name);
        g_info("File extension: %s", orig_file_ext);
        sprintf(new_name, "%s%s%s", bare_name, options.output_file_suffix, orig_file_ext);
        g_info("New filename: %s", new_name);
        // options.output_file = new_name;
    }

    in_width = vips_image_get_width(in);
    in_height = vips_image_get_height(in);
    g_info("Input dims: %d x %d", in_width, in_height);

    if (options.pct_scale) {
        g_info("Scaling image");
        if (vips_resize(in, &out, options.pct_scale, NULL)) {
            vips_error_exit(NULL);
        }
    }

    out_width = vips_image_get_width(out);
    out_height = vips_image_get_height(out);

    printf("%s"
           "Input file:        %s\n"
           "Input width:       %d\n"
           "Input height:      %d\n"
           "\n"
           "Output file:       %s\n"
           "Output width:      %d\n"
           "Output height:     %d\n",
           options.no_op ? "***Display results only***\n" : "", orig_file_name, in_width, in_height,
           options.output_file, out_width, out_height);

    g_object_unref(in);

    if (!options.no_op) {
        if (vips_image_write_to_file(out, options.output_file, NULL)) {
            vips_error_exit(NULL);
        }
    }

    g_object_unref(out);

    return (0);
}

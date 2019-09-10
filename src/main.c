#include "../config.h"
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <vips/vips.h>

// Command line options
typedef struct _options_t
{
    char *input_file;
    char *output_file;
    char *output_file_suffix;
    char *file_extension;
    double pct_scale;
    int width;
    int height;
    int quality;
    int verbosity;
    int no_op;
} options_t;


static const char *help_text =
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
    "  -q, --quality=<N>      Encoding quality for JPEG images (default=85)\n"
    "  -s, --suffix=<TEXT>    Suffix to append to file name for edited file\n"
    "\n";

/**
 * @brief Dummy no-op handler for logging
 * @return void
 */
static void
_glog_dummy_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message,
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
print_options_t(char *buf, size_t bufsize, options_t *options)
{
    if (!buf) return;
    snprintf(buf, bufsize,
             "Options:\n"
             "Input file:        %s\n"
             "Input ext:         %s\n"
             "Output file:       %s\n"
             "Output suffix:     %s\n"
             "Pct scale:         %.1f\n"
             "Quality:           %d\n"
             "Width:             %d\n"
             "Height:            %d\n"
             "Verbosity:         %d\n"
             "No-op:             %d",
             options->input_file, options->file_extension, options->output_file,
             options->output_file_suffix, options->pct_scale, options->quality, options->width,
             options->height, options->verbosity, options->no_op);
}

int
parse_args(int argc, char **argv, options_t *options)
{
    int choice;
    static struct option long_options[] = {{"verbosity", optional_argument, 0, 'v'},
                                           {"version", no_argument, 0, 'V'},
                                           {"help", no_argument, 0, 'h'},
                                           {"suffix", required_argument, 0, 's'},
                                           {"pct-scale", required_argument, 0, 'p'},
                                           {"quality", required_argument, 0, 'q'},
                                           {"width", required_argument, 0, 'w'},
                                           {"height", required_argument, 0, 'H'},
                                           {0, 0, 0, 0}};

    int option_index = 0;

    while ((choice = getopt_long(argc, argv, "Vhvnw:H:p:s:q:", long_options, &option_index)) !=
           -1) {
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
        case 'q':
            options->quality = atoi(optarg);
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

/**
 * @brief Format bytes as a human-readable string.
 *
 * @param bytes_raw Bytes to format
 *
 * @return char *
 */
static char *
humanize_bytes(size_t bytes_raw)
{
    static const char units[] = {
        '\0', // not used
        'K',  // kibi ('k' for kilo is a special case)
        'M',  // mega or mebi
        'G',  // giga or gibi
        'T',  // tera or tebi
        'P',  // peta or pebi
        'E',  // exa or exbi
        'Z',  // zetta or 2**70
    };
    char suffix = 'B';
    char *s = malloc(8);
    double bytes = (double)bytes_raw;
    double factor = 1024;

    if (bytes < factor) {
        sprintf(s, "%.f%c", bytes, suffix);
        return s;
    }
    for (size_t i = 0; i < sizeof(units); ++i) {
        if (bytes < factor) {
            sprintf(s, "%3.1f%c%c", bytes, units[i], suffix);
            return s;
        }
        bytes = (bytes / factor);
    }
    // if we got here, it's a really big number!
    // return yottabytes
    sprintf(s, "%.1f%c%c", bytes, 'Y', suffix);
    return s;
}

void
init_log(options_t *opts)
{
    // Set dummy handler for all levels
    // Set logging level based on verbosity flag
    g_log_set_handler(NULL, G_LOG_LEVEL_MASK, _glog_dummy_handler, NULL);
    switch (opts->verbosity) {
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
    if (opts->verbosity) {
        setenv("G_MESSAGES_DEBUG", "all", 1);
        g_info("Verbosity level: %d", opts->verbosity);
    }
}

void
get_new_filename(options_t *opts)
{
    char *orig_file_name = NULL;

    orig_file_name = g_path_get_basename(opts->input_file);
    char bare_name[strlen(orig_file_name) - strlen(opts->file_extension) + 1];
    g_info("Output file not supplied; using suffix '%s'", opts->output_file_suffix);
    g_strlcpy(bare_name, orig_file_name, sizeof(bare_name));

    // Assign new filename to opts struct
    opts->output_file = (char *)malloc(strlen(opts->input_file) + strlen(opts->output_file_suffix));
    sprintf(opts->output_file, "%s%s%s", bare_name, opts->output_file_suffix, opts->file_extension);
}

int
main(int argc, char **argv)
{
    VipsImage *image_in = NULL;
    VipsImage *image_out = NULL;
    int in_width = 0;
    int in_height = 0;
    int out_width = 0;
    int out_height = 0;
    char *in_size_human = NULL;
    char *out_size_human = NULL;
    char *size_delta_human = NULL;
    char *in_buf;
    size_t in_buf_size = 0;
    size_t out_buf_size = 0;
    size_t size_delta = 0;
    void *out_buf;
    char encode_opts[12];

    // Init command line options
    options_t *opts = malloc(sizeof(options_t));
    opts->input_file = NULL;
    opts->file_extension = NULL;
    opts->output_file = NULL;
    opts->output_file_suffix = "_edited";
    opts->quality = 85;
    opts->pct_scale = 0;
    opts->width = 0;
    opts->height = 0;
    opts->verbosity = 0;
    opts->no_op = 0;

    if (parse_args(argc, argv, opts)) {
        fprintf(stderr, "%s: error parsing command-line arguments", argv[0]);
        free(opts);
        return 1;
    }

    init_log(opts);

    // Deal with positional options
    opts->input_file = argv[optind++];
    opts->output_file = argv[optind++];

    if (!opts->input_file) {
        fprintf(stderr, "%s: input file required\n", argv[0]);
        exit(1);
    }

    opts->file_extension = strrchr(opts->input_file, '.');

    if (!opts->output_file) get_new_filename(opts);

    if (optind < argc) {
        g_info("Additional non-option ARGV-elements: ");
        while (optind < argc) {
            g_info("%s ", argv[optind++]);
        }
    }

    if (VIPS_INIT(argv[0])) {
        vips_error_exit("Unable to start VIPS");
    }

    g_debug("Reading %s into buffer", opts->input_file);
    if (!g_file_get_contents(opts->input_file, &in_buf, &in_buf_size, NULL)) {
        vips_error_exit("error getting file %s", opts->input_file);
    }

    // Get original image details
    // Create vips image from buffer to get image metadata
    g_debug("Getting vips image from buffer");
    if (!(image_in = vips_image_new_from_buffer(in_buf, in_buf_size, NULL, NULL))) {
        vips_error_exit("error getting vips image from buffer");
    }

    in_width = vips_image_get_width(image_in);
    in_height = vips_image_get_height(image_in);

    if (opts->pct_scale) {
        g_info("Scaling image");
        opts->width = opts->pct_scale * in_width;
        opts->height = opts->pct_scale * in_height;
    }

    // Debug print for options
    if (opts->verbosity > 1) {
        char buf[300];
        print_options_t(buf, sizeof(buf), opts);
        g_debug("%s", buf);
    }

    // transform image
    if (vips_thumbnail_buffer(in_buf, in_buf_size, &image_out, opts->width, "height", opts->height,
                              NULL)) {
        vips_error_exit("error creating thumbnail");
    }

    snprintf(encode_opts, sizeof(encode_opts), "%s[Q=%d]", opts->file_extension, opts->quality);
    g_debug("Writing to buffer with suffix: %s", encode_opts);
    if (vips_image_write_to_buffer(image_out, encode_opts, &out_buf, &out_buf_size, NULL)) {
        g_object_unref(image_out);
        vips_error_exit("error creating thumbnail");
    }

    size_delta = in_buf_size - out_buf_size;
    in_size_human = humanize_bytes(in_buf_size);
    out_size_human = humanize_bytes(out_buf_size);
    size_delta_human = humanize_bytes(size_delta);

    out_width = vips_image_get_width(image_out);
    out_height = vips_image_get_height(image_out);

    printf("%s"
           "Input file:        %s\n"
           "Input width:       %d\n"
           "Input height:      %d\n"
           "Input file size:   %s\n"
           "\n"
           "Output file:       %s\n"
           "Output width:      %d\n"
           "Output height:     %d\n"
           "Output file size:  %s\n"
           "\n"
           "Size reduction:    %s\n",
           opts->no_op ? "***Display results only***\n" : "", opts->input_file, in_width, in_height,
           in_size_human, opts->output_file, out_width, out_height, out_size_human,
           size_delta_human);

    if (!opts->no_op) {
        if (opts->output_file) {
            if (!g_file_set_contents(opts->output_file, out_buf, out_buf_size, NULL)) {
                vips_error_exit("error writing '%s'", opts->output_file);
            }
        } else {
            fprintf(stderr, "error: output file not written. Invalid output file name.\n");
        }
    }

    free(in_size_human);
    free(out_size_human);
    free(size_delta_human);

    g_free(in_buf);
    g_free(out_buf);
    g_object_unref(image_out);
    g_object_unref(image_in);
    free(opts);

    vips_shutdown();
}

#include "traceroute.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

/* External Variables */
/* extern int optind; */

/* Function Prototypes */
int main(int argc, char ** argv);
void display_usage(const char * program, int code);
void display_author(const char * program, int code);
int parse_cmdln(int argc, char ** argv);

/*
        Display the Usage Information
 */
void display_usage(const char * program, int code) {
    printf("USAGE: %s [OPTIONS] <ip>\n", program);
    printf("  OPTIONS\n");
    printf("   -h,--help   display this help screen.\n");
    printf("   -a,--about  display application author information.\n");
    printf("\n");
    printf("  REQUIRED\n");
    printf("   <ip>        the IPv4 address to traceroute to.\n");
    printf("\n");

    exit(code);
}

/*
        Display the Application Author
 */
void display_author(const char * program, int code) {
    printf("AUTHOR: %s\n", AUTHOR);

    exit(code);
}

/*
        Parse the Command Line Options
 */
int parse_cmdln(int argc, char ** argv) {
    const static struct option options[] = {/* Command Line Options */
        { "help", 0, 0, 'h'},
        { "about", 0, 0, 'a'},

        { 0, 0, 0, 0}
    };

    int index; /* Index of Argument */
    int c; /* Current Option */

    /* Read all command line arguments. */
    while ((c = getopt_long(argc, argv, "ha", options, &index)) > 0) {
        switch (c) {
            case 'h':
                display_usage(argv[0], EXIT_SUCCESS);
                break;

            case 'a':
                display_author(argv[0], EXIT_SUCCESS);
                break;

            default:
                display_usage(argv[0], EXIT_FAILURE);
        }
    }
    return optind;
}

/*
        The Application Main Function
 */
int main(int argc, char ** argv) {
    int index = parse_cmdln(argc, argv);

    if ((argc != 2) || (index != 1)) {
        display_usage(argv[0], EXIT_FAILURE);
    }
    return perform_traceroute(argv[index]);
}

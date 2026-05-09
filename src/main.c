#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>


struct Request {
    __uint32_t addr;
    __uint32_t data;
    int we;
};

struct Result {
    size_t cycles;
    size_t misses;
    size_t hits;
    size_t primitiveGateCount;
};

int isStringValid(char *str, int same) {
    //TODO: Add Error when String does not fit into a long
    char *endptr;
    long value = strtol(str, &endptr, 0);
    //printf ("%s\n", endptr);
    if (*endptr != '\0') {
        fprintf(stderr,
                "Error: The input %s is not a valid parameter. Parameters have to be non-negative long numbers.\n",
                str);
        exit(0);
    } else if (same && value <= 0) {
        fprintf(stderr, "Error: The input %s is smaller than or equal to zero.\n", str);
        exit(0);
    } else if (!same && value < 0) {
        fprintf(stderr, "Error: The input %s is smaller than zero.\n", str);
        exit(0);
    } else {
        return value;
    }
}

__uint32_t stringToLong(char *str, int nn) {
    //TODO: Add Error when String does not fit into a long
    char *endptr;
    if (*str == '-' && nn) {
        fprintf(stderr, "Error: Negative number not allowed as address.\n");
        exit(0);
    }
    __uint32_t value = strtol(str, &endptr, 0);
    if (*endptr != '\0') {
        fprintf(stderr, "Error: The string %s in input csv file is not a valid number.\n", str);
        exit(0);
    } else {
        return value;
    }
}

//Splits a string by \n and returns the pointer to the head of the new string and replaces \n with \0 (termination).
char *splitNewLine(char *string) {
    long ptr = 0;
    while (1) {
        switch (*(string + ptr)) {
            case '\n': {
                *(string + ptr) = '\0';
                return string + ptr + 1;
            }
            case '\0': {
                return string + ptr;
            }
        }
        ptr++;
    }
}

//Splits a string by ; or , and returns the pointer to the head of the new string and replaces ; with \0 (termination).
char *splitSemicolon(char *string) {
    int ptr = 0;
    while (1) {
        switch (*(string + ptr)) {
            case ';': {
                *(string + ptr) = '\0';
                return string + ptr + 1;
            }
            case ',': {
                *(string + ptr) = '\0';
                return string + ptr + 1;
            }
            case '\0': {
                return string + ptr;
            }
            case '\n': {
                return string + ptr;
            }
        }
        ptr++;
    }
}

void printHelpMessage();

//Define C++ call
extern struct Result run_simulation(
        int cycles,
        int directMapped,
        unsigned cacheLines,
        unsigned cacheLineSize,
        unsigned cacheLatency,
        unsigned memoryLatency,
        size_t numRequests,
        struct Request requests[numRequests],
        const char *tracefile);


int main(int argc, char *argv[]) {
    int c;
    int digit_optind = 0;

    //Flags for the program
    //Frequency about 3000 MHz, 1 millisecond simulated
    int cycles = 3000000;
    static int directmapped = 0;
    //Cache sizes
    unsigned cachelinesize = 64;
    unsigned cachelines = 256;
    //10 cycles between L1 (4-5 cycles) and L2 Cache (12-17 cycles)
    unsigned cachelatency = 10;
    unsigned memorylatency = 200;
    char *tf = "";
    //silent tag for just returning the result
    int silent = 0;


    while (1) {
        static struct option long_options[] =
                {
                        {"cycles",         required_argument, 0,             'c'},
                        {"directmapped",   no_argument,       &directmapped, 1},
                        {"fourway",        no_argument,       &directmapped, 0},
                        {"cacheline-size", required_argument, 0,             '0'},
                        {"cachelines",     required_argument, 0,             '1'},
                        {"cache-latency",  required_argument, 0,             '2'},
                        {"memory-latency", required_argument, 0,             '3'},
                        {"tf",             required_argument, 0,             '4'},
                        {"help",           no_argument,       0,             'h'},
                        {0,                0,                 0,             0}
                };
        //option index from getopt_long
        int option_index = 0;

        c = getopt_long(argc, argv, "c:0:1:2:3:4:hs",
                        long_options, &option_index);

        //No arguments left
        if (c == -1)
            break;

        switch (c) {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;
                if (!silent) {
                    printf("option %s", long_options[option_index].name);
                    if (optarg)
                        printf(" with arg %s", optarg);
                    printf("\n");
                }
                break;


            case 'c':
                if (!silent) { printf("option -c/--cycles with value `%s'\n", optarg); }
                cycles = isStringValid(optarg, 1);
                break;

            case '0':
                if (!silent) { printf("option --cacheline-size with value `%s'\n", optarg); }
                cachelinesize = isStringValid(optarg, 1);
                break;

            case '1':
                if (!silent) { printf("option --cachelines with value `%s'\n", optarg); }
                cachelines = isStringValid(optarg, 1);
                break;

            case '2':
                if (!silent) { printf("option --cache-latency with value `%s'\n", optarg); }
                cachelatency = isStringValid(optarg, 0);
                break;

            case '3':
                if (!silent) { printf("option --memory-latency with value `%s'\n", optarg); }
                memorylatency = isStringValid(optarg, 0);
                break;

            case '4':
                if (!silent) { printf("option --tf with value `%s'\n", optarg); }
                tf = optarg;
                break;

            case 'h':
                if (!silent) { printf("option -h/--help\n"); }
                printHelpMessage();
                exit(0);

            case 's':
                silent = 1;
                break;

            case '?':
                fprintf(stderr,
                        "Error: You have used an unbound option. Please use -h to see which options are available.\n");
                exit(0);

            default:
                exit(1);
        }
    }

    //Print the Positional Argument: Else Error
    if (optind < argc) {
        if (optind + 1 == argc) {
            //printf("%s\n", argv[optind]);

            if (cycles < 0) {
                cycles = __INT_MAX__;
                if (!silent) { printf("The number of cycles does not fit into an int. It is now %d\n", cycles); }
            }
            if (!silent) {
                printf("Starting the program with following settings:\ncycles: %d, directmapped: %d, cacheline-size: %d, cachelines: %d, cache-latency: %d, memory-latency: %d, tracefile: %s\n",
                       cycles, directmapped, cachelinesize, cachelines, cachelatency, memorylatency, tf);
            }

            //Reading from csv-file
            FILE *infile = fopen(argv[optind], "r");
            struct stat finfo;
            errno = 0;
            if (fstat(fileno(infile), &finfo) != 0) {
                fprintf(stderr, "Error: Could not open the file %s, the Error number is: %d\n", argv[optind], errno);
                fclose(infile);
                exit(0);
            }
            //printf("%ld, %ld\n", finfo.st_blksize, finfo.st_size);
            char *input_string = malloc(finfo.st_size + 1);
            if (!input_string) {
                fprintf(stderr, "Error: Memory allocation failed for input string.\n");
                exit(0);
            }
            if (fread(input_string, 1, finfo.st_size, infile) < finfo.st_size) {
                fprintf(stderr, "Error: Could not complete reading from file %s", argv[optind]);
                fclose(infile);
                exit(0);
            }
            fclose(infile);
            *(input_string + finfo.st_size) = 0;
            //End of file reading
            //printf("%s\n", input_string);

            //Reading the input_string into an array of Requests
            int line = 0;
            char *nextstring;
            size_t lines = 1;
            char *ch = input_string;
            while (*ch != '\0') {
                if (*ch == '\n') lines++;
                ch++;
            }
            //Pointer at the next empty element of the Request array
            int stptr = 0;
            char *this_string = input_string;
            //Allocate memory for the Request array an check if that was successful
            struct Request *requests = malloc(lines * sizeof(struct Request));
            if (!requests) {
                fprintf(stderr, "Error: Memory allocation failed for requests.\n");
                free(input_string);
                exit(0);
            }

            //Start of CSV-Parsing
            while (1) {
                nextstring = splitNewLine(this_string);
                line++;
                int n = 0;
                int w = 0;
                __uint32_t v1, v2 = 0;
                n = sscanf(this_string, "W, %i, %i", &v1, &v2);
                if (n == 2) {
                    //printf("read1: %d, %d\n", v1, v2);
                    w = 1;
                } else {
                    n = sscanf(this_string, "R, %i", &v1);
                    if (n == 1) {
                        //printf("read2: %d\n", v1);
                    } else {
                        n = sscanf(this_string, "\"W\", \"%i\", \"%i\" ", &v1, &v2);
                        w = 1;
                        if (n == 2) {
                            //printf("read3: %d, %d\n", v1, v2);
                        } else {
                            n = sscanf(this_string, "\"R\", \"%i\" ", &v1);
                            if (n == 1) {
                                //printf("read4: %d\n", v1);
                            } else {
                                fprintf(stderr,
                                        "CSV format error at line %d. Expected format: <R/W>,<address>,<value (only for W)>.\n",
                                        line);
                                free(input_string);
                                free(requests);
                                exit(0);
                            }
                        }
                    }
                }

                /*char* element1 = this_string;
                char* element2 = splitSemicolon(element1);
                char* element3 = splitSemicolon(element2);
                printf("%s; %s; %s :: %s\n", element1, element2, element3, nextstring);
                //Check if strings are empty, if 1 is W or R and then empty
                if (*element1 == '\0' || *element2 == '\0' || !(*element1 == 'W' || *element1 == 'R') || *(element1 + 1) != '\0' || (*element1 == 'W' && *element3 == '\0') || (*element1 == 'R' && *element3 != '\0')) {
                  fprintf(stderr, "The input csv file has the wrong format in line %d. The format is:\n Either R for Read or W for Write as the first Element\n A adress in the second element\n A value in the third Element only if the Request is a Write otherwise leave it blank\n The Elements are seperated by semicolons. There mustn't be any additional characters in any line\n", line);
                  exit(0);
                }
                if (*element1 == 'W') {
                  struct Request request = {
                    stringToLong(element2, 1),
                    stringToLong(element3, 0),
                    1
                  };
                  //*(requests + stptr++ * sizeof(struct Request)) = request;
                  requests[stptr++] = request;
                } else {
                  struct Request request = {
                    stringToLong(element2, 1),
                    0,
                    0
                  };
                  //*(requests + stptr++ * sizeof(struct Request)) = request;
                  requests[stptr++] = request;
                }*/
                if (w) {
                    struct Request request = {
                            v1,
                            v2,
                            1
                    };
                    requests[stptr++] = request;
                } else {
                    struct Request request = {
                            v1,
                            0,
                            0
                    };
                    requests[stptr++] = request;
                }
                if (*nextstring == '\0') {
                    break;
                }
                this_string = nextstring;
            }
            //End of CSV-Parsing

            //printf("Finished reading csv. Request 1 is: %d, %d, %d\n", requests[0].addr, requests->data, requests->we);

            //Do the C++ call for simulation
            free(input_string);
            struct Result result = run_simulation(cycles, directmapped, cachelines, cachelinesize, cachelatency,
                                                  memorylatency, lines, requests, tf);
            //printf("Successfully ran the simulation.\n");


            double hitrate = 100 * (double) result.hits / (double) lines;
            printf("Simulation Result:\n"
                   "  Cycles: %zu\n"
                   "  Cache Misses: %zu\n"
                   "  Cache Hits: %zu\n"
                   "  Hitrate: %f%%\n"
                   "  Primitive Gate Count: %zu\n", result.cycles, result.misses, result.hits, hitrate,
                   result.primitiveGateCount);
            //Eventually free the memory
            free(requests);
        } else {
            fprintf(stderr,
                    "Error: The program has found to many positional arguments. Only the name of the input file should be given the program as a positional argument.\n");
        }
    } else {
        fprintf(stderr,
                "Error: The program is missing the file name of the input file. It is supposed to be given the program as a positional argument.\n");
    }

    //./main --directmapped -c 1234 --tf=tester --cacheline-size 10 anywhereCSV --memory-latency 100 --cache-latency 1 --cachelines 1000 --help

}

void printHelpMessage() {
    printf("Cache Simulator for GRA\n"
           "Usage: ./systemcc [OPTIONS] [INPUT_FILE]\n"
           "Options:\n"
           "  --cycles, -c <num>       Number of cycles\n"
           "  --directmapped           Use direct mapped cache\n"
           "  --fourway                Use four-way set associative cache\n"
           "  --cacheline-size <num>   Cache line size\n"
           "  --cachelines <num>       Number of cache lines\n"
           "  --cache-latency <num>    Cache latency\n"
           "  --memory-latency <num>   Memory latency\n"
           "  --tf <file>              Trace file\n"
           "  --help, -h               Display this help message\n"
           "  -s                       Mute all non-necessary outputs on stdout\n");
}
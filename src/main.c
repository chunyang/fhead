#include <stdio.h>
#include <stdlib.h>

#include "summary.h"
#include "print.h"

#define VERBOSE 0

void print_summary(fit_summary* summary);

int main(int argc, const char *argv[])
{
    fit_summary *summary;

    /* Check arguments */
    if (argc < 2) {
        printf("Usage: %s FIT-FILE\n", argv[0]);
        return EXIT_SUCCESS;
    }

    summary = summarize(argv[1]);

    if (summary) {
        print_summary(summary);
        destroy_summary(summary);
    }

    return EXIT_SUCCESS;
}

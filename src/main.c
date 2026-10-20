#include <stdio.h>

#include "http.h"

int main(int argc, char **argv)
{
    serve(argc > 1 ? argv[1] : "www", stdin, stdout);
    fflush(stdout);
    return 0;
}

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdbool.h>




extern "C" int libs_main(int argc, char *argv[])
{
    printf("This is not an app, but a library\n");
    return EXIT_SUCCESS;
}

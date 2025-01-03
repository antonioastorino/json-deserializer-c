#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h> /* gettimeofday */

#include "json_deserializer.h"
#include "json_deserializer.c"

#ifndef TEST
int main()
{
    LOG_INFO("Nothing to run.\n");
    return 0;
}
#else  /* TEST defined */
int main()
{
    test_json_deserializer();
    test_logger();
    return 0;
}
#endif /* TEST */

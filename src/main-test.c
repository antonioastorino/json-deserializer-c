#include "header.h"

int main()
{
#if TEST == 1
    logger_init(NULL /* log file is stdout */, NULL /* error file is stderr */);
    test_class_json();
    test_converter();
    test_class_string();
#endif /* TEST == 1 */
}

#include "class_json.c"
#include "class_string.c"
#include "converter.c"
#include "common.c"
#include "my_memory.c"


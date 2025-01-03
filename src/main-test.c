#include "class_json.h"
#include "converter.h"
#include "class_string.h"

int main()
{
#if TEST == 1
    logger_init(NULL /* log file is stdout */, NULL /* error file is stderr */);
    test_class_json();
    test_converter();
    test_class_string();
#endif /* TEST == 1 */
}

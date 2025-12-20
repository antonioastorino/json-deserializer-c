typedef enum
{
    ERR_FATAL = -1,
    ERR_ALL_GOOD,
    ERR_INVALID,
    ERR_NULL,
    ERR_PARSE_STRING_TO_INT,
    ERR_PARSE_STRING_TO_LLU,
    ERR_PARSE_STRING_TO_DOUBLE,
    ERR_EMPTY_STRING,
    ERR_JSON_INVALID,
    ERR_JSON_MISSING_ENTRY,
    ERR_TYPE_MISMATCH,
} Error;

#define is_err(_expr) ((_expr) != ERR_ALL_GOOD)
#define is_ok(_expr) ((_expr) == ERR_ALL_GOOD)

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LEVEL_TRACE 5
#define LEVEL_DEBUG 4
#define LEVEL_INFO 3
#define LEVEL_WARNING 2
#define LEVEL_ERROR 1
#define LEVEL_NO_LOGS 0
#ifndef LOG_LEVEL
#define LOG_LEVEL LEVEL_TRACE
#endif /* LOG_LEVEL */

// Ensure that errors are not printed to stderr during tests as they would cause the unit test
// to fail.

#ifdef TEST
#define log_out stdout
#define log_err stdout
void test_logger(void);
#else /* TEST */
#define log_out get_log_out_file()
#define log_err get_log_err_file()
#endif /* TEST */

#define return_on_err(_expr)                            \
    {                                                   \
        Error _res = _expr;                             \
        if (_res != ERR_ALL_GOOD)                       \
        {                                               \
            LOG_WARNING("Error propagated from here."); \
            return _res;                                \
        }                                               \
    }

#if LOG_LEVEL > LEVEL_NO_LOGS
#define DATE_TIME_STR_LEN 32
void logger_init(const char*, const char*);

FILE* get_log_out_file(void);
FILE* get_log_err_file(void);

void get_date_time(char* date_time_str);

#define log_formatter(out_or_err, TYPE, fmt, ...)  \
    char date_time_str[DATE_TIME_STR_LEN];         \
    get_date_time(date_time_str);                  \
    pthread_mutex_lock(&log_##out_or_err##_mutex); \
    fprintf(                                       \
        log_##out_or_err,                          \
        "[%5s] <%d> %s %s:%d | " fmt "\n",         \
        #TYPE,                                     \
        getpid(),                                  \
        date_time_str,                             \
        __FILENAME__,                              \
        __LINE__ __VA_OPT__(, ) __VA_ARGS__);      \
    fflush(log_##out_or_err);                      \
    pthread_mutex_unlock(&log_##out_or_err##_mutex);

#define PRINT_SEPARATOR()                                                       \
    {                                                                           \
        char date_time_str[DATE_TIME_STR_LEN];                                  \
        get_date_time(date_time_str);                                           \
        pthread_mutex_lock(&log_out_mutex);                                     \
        fprintf(log_out, "------- <%d> %s -------\n", getpid(), date_time_str); \
        pthread_mutex_unlock(&log_out_mutex);                                   \
    }

#else /* LOG_LEVEL > LEVEL_NO_LOGS */
#define get_date_time(something)
#define PRINT_SEPARATOR()
#endif /* LOG_LEVEL > LEVEL_NO_LOGS */

#if LOG_LEVEL >= LEVEL_ERROR
#define LOG_ERROR(fmt, ...) {log_formatter(err, ERROR, fmt, __VA_ARGS__)}

#define LOG_PERROR(fmt, ...) {log_formatter(err, ERROR, fmt "`%s` | ", __VA_ARGS__ __VA_OPT__(, ) strerror(errno))}
#else /* TEST */
#define LOG_ERROR(...)
#endif /* TEST */

#if LOG_LEVEL >= LEVEL_WARNING
#define LOG_WARNING(fmt, ...) {log_formatter(err, WARN, fmt, __VA_ARGS__)}
#else
#define LOG_WARNING(...)
#endif

#if LOG_LEVEL >= LEVEL_INFO
#define LOG_INFO(fmt, ...) {log_formatter(out, INFO, fmt, __VA_ARGS__)}
#else
#define LOG_INFO(...)
#endif

#if LOG_LEVEL >= LEVEL_DEBUG
#define LOG_DEBUG(fmt, ...) {log_formatter(out, DEBUG, fmt, __VA_ARGS__)}
#else
#define LOG_DEBUG(...)
#endif

#if LOG_LEVEL >= LEVEL_TRACE
#define LOG_TRACE(fmt, ...) {log_formatter(out, TRACE, fmt, __VA_ARGS__)}
#else
#define LOG_TRACE(...)
#endif

void wrap_free(char** var);
// ---------- ASSERT ----------
#ifdef TEST

void ASSERT_(bool, const char*, const char*, int);
void ASSERT_OK_(Error, const char*, const char*, int);
void ASSERT_ERR_(Error, const char*, const char*, int);

void ASSERT_EQ_long_long(long long, long long, const char*, const char*, int);
void ASSERT_EQ_llu(unsigned long long, unsigned long long, const char*, const char*, int);
void ASSERT_EQ_uint(size_t, size_t, const char*, const char*, int);
void ASSERT_EQ_bool(bool v1, bool v2, const char*, const char*, int);
void ASSERT_EQ_double(double, double, const char*, const char*, int);
void ASSERT_EQ_char_p(const char*, const char*, const char*, const char*, int);

void ASSERT_NE_long_long(long long, long long, const char*, const char*, int);
void ASSERT_NE_llu(uint32_t, uint32_t, const char*, const char*, int);
void ASSERT_NE_uint(size_t, size_t, const char*, const char*, int);
void ASSERT_NE_bool(bool v1, bool v2, const char*, const char*, int);
void ASSERT_NE_double(double, double, const char*, const char*, int);
void ASSERT_NE_char_p(const char*, const char*, const char*, const char*, int);

#define PRINT_BANNER()                                     \
    printf("\n");                                          \
    for (size_t i = 0; i < strlen(__FUNCTION__) + 12; i++) \
    {                                                      \
        printf("=");                                       \
    }                                                      \
    printf("\n-- TEST: %s --\n", __FUNCTION__);            \
    for (size_t i = 0; i < strlen(__FUNCTION__) + 12; i++) \
    {                                                      \
        printf("=");                                       \
    }                                                      \
    printf("\n");                                          \
    size_t test_counter_ = 0;

#define ASSERT(value, message) ASSERT_(value, message, __FILE__, __LINE__)
#define ASSERT_OK(value, message) ASSERT_OK_(value, message, __FILE__, __LINE__)
#define ASSERT_ERR(value, message) ASSERT_ERR_(value, message, __FILE__, __LINE__)
// clang-format off
#define ASSERT_EQ(value_1, value_2, message)      \
    _Generic((value_1),                           \
        int               : ASSERT_EQ_long_long,  \
        long              : ASSERT_EQ_long_long,  \
        long long         : ASSERT_EQ_long_long,  \
        uint8_t           : ASSERT_EQ_llu,        \
        uint16_t          : ASSERT_EQ_llu,        \
        uint32_t          : ASSERT_EQ_llu,        \
        size_t            : ASSERT_EQ_uint,       \
        unsigned long long: ASSERT_EQ_llu,        \
        bool              : ASSERT_EQ_bool,       \
        float             : ASSERT_EQ_double,     \
        double            : ASSERT_EQ_double,     \
        char*             : ASSERT_EQ_char_p,     \
        const char*       : ASSERT_EQ_char_p      \
    )(value_1, value_2, message, __FILE__, __LINE__)

#define ASSERT_NE(value_1, value_2, message)      \
    _Generic((value_1),                           \
        int           : ASSERT_NE_long_long,      \
        long          : ASSERT_NE_long_long,      \
        long long     : ASSERT_NE_long_long,      \
        uint8_t       : ASSERT_NE_llu,            \
        uint16_t      : ASSERT_NE_llu,            \
        uint32_t      : ASSERT_NE_llu,            \
        size_t        : ASSERT_NE_uint,           \
        bool          : ASSERT_NE_bool,           \
        float         : ASSERT_NE_double,         \
        double        : ASSERT_NE_double,         \
        char*         : ASSERT_NE_char_p,         \
        const char*   : ASSERT_NE_char_p          \
    )(value_1, value_2, message, __FILE__, __LINE__)

// clang-format on

#define PRINT_TEST_TITLE(title) printf("\n------- T:%lu < %s > -------\n", ++test_counter_, title);
#endif /* TEST */
#define PRINT_PASS_MESSAGE(message) printf("> \x1B[32mPASS\x1B[0m\t %s\n", message)

#define PRINT_FAIL_MESSAGE_(message, filename, line_number)   \
    fprintf(stderr, "> \x1B[31mFAIL\x1B[0m\t %s\n", message); \
    fprintf(stderr, "> Err - Test failed.\n%s:%d : false assertion\n", filename, line_number)

#define PRINT_FAIL_MESSAGE_EQ(message, filename, line_number) \
    fprintf(stderr, "> \x1B[31mFAIL\x1B[0m\t %s\n", message); \
    fprintf(stderr, "> Err - Test failed.\n%s:%d : left != right\n", filename, line_number)

#define PRINT_FAIL_MESSAGE_NE(message, filename, line_number) \
    fprintf(stderr, "> \x1B[31mFAIL\x1B[0m\t %s\n", message); \
    fprintf(stderr, "> Err - Test failed.\n%s:%d : left == right\n", filename, line_number)

typedef long long int json_int_t;
typedef unsigned long long json_uint_t;
typedef double json_decimal_t;
typedef bool json_bool_t;

typedef struct JsonItem JsonItem;
typedef struct JsonValue JsonValue;
typedef struct JsonArray JsonArray;

typedef enum
{
    VALUE_ROOT,
    VALUE_UNDEFINED,
    VALUE_INT,
    VALUE_BOOL,
    VALUE_LLU,
    VALUE_DOUBLE,
    VALUE_STR,
    VALUE_ARRAY,
    VALUE_ITEM,
    VALUE_INVALID,
} ValueType;

typedef struct JsonValue
{
    ValueType value_type;
    union
    {
        json_int_t value_int;            // leaf json_int_t
        json_uint_t value_llu;           // leaf json_uint_t
        json_decimal_t value_double;     // leaf json_decimal_t
        json_bool_t value_bool;          // leaf json_bool_t
        const char* value_char_p;        // leaf c-string
        struct JsonItem* value_child_p;  // another item
        struct JsonArray* value_array_p; // the first item of an array
    };
} JsonValue;

typedef struct JsonItem
{
    const char* key_p;
    json_uint_t index; // For arrays only
    JsonValue value;
    struct JsonItem* parent;
    struct JsonItem* next_sibling;
} JsonItem;

typedef struct JsonObj
{
    char* json_string;
    JsonItem root;
} JsonObj;

Error JsonObj_new(const char*, JsonObj*);
void JsonObj_destroy(JsonObj*);
void JsonObj_get_tokens(char*);

// Created to have a symmetry between GET_VALUE and GET_ARRAY_VALUE
Error invalid_request(const JsonArray*, size_t, const JsonArray**);

// clang-format off
#define OBJ_GET_VALUE_h(suffix, out_type)                                                          \
    Error obj_get_##suffix(const JsonObj*, const char*, out_type);
    OBJ_GET_VALUE_h(value_char_p, const char**)
    OBJ_GET_VALUE_h(value_child_p, JsonItem**)
    OBJ_GET_VALUE_h(value_array_p, JsonArray**)

#define OBJ_GET_NUMBER_h(suffix, out_type)                                                         \
    Error obj_get_##suffix(const JsonObj*, const char*, out_type);
    OBJ_GET_VALUE_h(value_int, json_int_t*)
    OBJ_GET_VALUE_h(value_llu, json_uint_t*)
    OBJ_GET_VALUE_h(value_double, json_decimal_t*)
    OBJ_GET_VALUE_h(value_bool, json_bool_t*)

#define GET_VALUE_h(suffix, out_type) Error get_##suffix(const JsonItem*, const char*, out_type);
    GET_VALUE_h(value_char_p, const char**)
    GET_VALUE_h(value_child_p, JsonItem**)
    GET_VALUE_h(value_array_p, JsonArray**)

#define GET_NUMBER_h(suffix, out_type) Error get_##suffix(const JsonItem*, const char*, out_type);
    GET_VALUE_h(value_int, json_int_t*)
    GET_VALUE_h(value_llu, json_uint_t*)
    GET_VALUE_h(value_double, json_decimal_t*)
    GET_VALUE_h(value_bool, json_bool_t*)

#define GET_ARRAY_VALUE_h(suffix, out_type)                                                        \
    Error get_array_##suffix(const JsonArray*, size_t, out_type);
    GET_ARRAY_VALUE_h(value_char_p, const char**)
    GET_ARRAY_VALUE_h(value_int, json_int_t*)
    GET_ARRAY_VALUE_h(value_llu, json_uint_t*)
    GET_ARRAY_VALUE_h(value_double, json_decimal_t*)
    GET_ARRAY_VALUE_h(value_bool, json_bool_t*)
    GET_ARRAY_VALUE_h(value_child_p, JsonItem**)

#define Json_get(json_stuff, needle, out_p)                    \
    _Generic ((json_stuff),                                    \
        JsonObj*: _Generic((out_p),                            \
            const char**    : obj_get_value_char_p,            \
            json_int_t*     : obj_get_value_int,               \
            unsigned long * : obj_get_value_llu,               \
            json_uint_t*    : obj_get_value_llu,               \
            json_decimal_t* : obj_get_value_double,            \
            json_bool_t*    : obj_get_value_bool,              \
            JsonItem**      : obj_get_value_child_p,           \
            JsonArray**     : obj_get_value_array_p            \
            ),                                                 \
         JsonItem*: _Generic((out_p),                          \
            const char**    : get_value_char_p,                \
            json_int_t*     : get_value_int,                   \
            unsigned long * : get_value_llu,                   \
            json_uint_t*    : get_value_llu,                   \
            json_decimal_t* : get_value_double,                \
            json_bool_t*    : get_value_bool,                  \
            JsonItem**      : get_value_child_p,               \
            JsonArray**     : get_value_array_p                \
            ),                                                 \
        JsonArray*: _Generic((out_p),                          \
            const char**    : get_array_value_char_p,          \
            json_int_t*     : get_array_value_int,             \
            json_uint_t*    : get_array_value_llu,             \
            json_decimal_t* : get_array_value_double,          \
            json_bool_t*    : get_array_value_bool,            \
            JsonItem**      : get_array_value_child_p,         \
            JsonArray**     : invalid_request                  \
            )                                                  \
        )(json_stuff, needle, out_p)
// clang-format on

#ifdef TEST
                                                                                void test_json_deserializer(void);
#endif

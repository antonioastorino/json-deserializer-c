void ASSERT_(bool value, const char* message, const char* filename, int line_number)
{
    if (value)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_(message, filename, line_number);
        fprintf(stderr, "The value is `false`\n");
    }
}

void ASSERT_OK_(Error result, const char* message, const char* filename, int line_number)
{
    if (is_ok(result))
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_(message, filename, line_number);
        fprintf(stderr, "The value is `false`\n");
    }
}

void ASSERT_ERR_(Error result, const char* message, const char* filename, int line_number)
{
    if (is_err(result))
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_(message, filename, line_number);
        fprintf(stderr, "The value is `false`\n");
    }
}

void ASSERT_EQ_long_long(
    long long value_1,
    long long value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (value_1 == value_2)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_EQ(message, filename, line_number);
        fprintf(stderr, "Left : `%lld`\nRight: `%lld`\n", value_1, value_2);
    }
}

void ASSERT_EQ_llu(
    unsigned long long value_1,
    unsigned long long value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (value_1 == value_2)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_EQ(message, filename, line_number);
        fprintf(stderr, "Left : `%llu`\nRight: `%llu`\n", value_1, value_2);
    }
}

void ASSERT_EQ_uint(
    size_t value_1,
    size_t value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (value_1 == value_2)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_EQ(message, filename, line_number);
        fprintf(stderr, "Left : `%lu`\nRight: `%lu`\n", value_1, value_2);
    }
}

void ASSERT_EQ_bool(
    bool value_1,
    bool value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (value_1 == value_2)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_EQ(message, filename, line_number);
        fprintf(
            stderr,
            "Left : `%s`\nRight: `%s`\n",
            value_1 ? "true" : "false",
            value_2 ? "true" : "false");
    }
}

void ASSERT_EQ_double(
    double value_1,
    double value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (value_1 == value_2)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_EQ(message, filename, line_number);
        fprintf(stderr, "Left : `%lf`\nRight: `%lf`\n", value_1, value_2);
    }
}

void ASSERT_EQ_char_p(
    const char* value_1,
    const char* value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (value_1 == value_2)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else if (value_1 == NULL || value_2 == NULL || strcmp(value_1, value_2))
    {
        PRINT_FAIL_MESSAGE_NE(message, filename, line_number);
        fprintf(stderr, "Left : `%s`\nRight: `%s`\n", value_1, value_2);
    }
    else
    {
        PRINT_PASS_MESSAGE(message);
    }
}

void ASSERT_NE_long_long(
    long long value_1,
    long long value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (value_1 != value_2)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_NE(message, filename, line_number);
        fprintf(stderr, "Left : `%lld`\nRight: `%lld`\n", value_1, value_2);
    }
}

void ASSERT_NE_uint(
    size_t value_1,
    size_t value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (value_1 != value_2)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_NE(message, filename, line_number);
        fprintf(stderr, "Left : `%lu`\nRight: `%lu`\n", value_1, value_2);
    }
}

void ASSERT_NE_bool(
    bool value_1,
    bool value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (value_1 != value_2)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_NE(message, filename, line_number);
        fprintf(
            stderr,
            "Left : `%s`\nRight: `%s`\n",
            value_1 ? "true" : "false",
            value_2 ? "true" : "false");
    }
}

void ASSERT_NE_double(
    double value_1,
    double value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (value_1 == value_2)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_NE(message, filename, line_number);
        fprintf(stderr, "Left : `%lf`\nRight: `%lf`\n", value_1, value_2);
    }
}

void ASSERT_NE_char_p(
    const char* value_1,
    const char* value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (value_1 != value_2 || value_1 == NULL || value_2 == NULL || strcmp(value_1, value_2))
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_NE(message, filename, line_number);
        fprintf(stderr, "Left : `%s`\nRight: `%s`\n", value_1, value_2);
    }
}

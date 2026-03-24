# JSON deserializer in C

## Overview

This library implements a **hand-written JSON deserializer in C** with no external dependencies. It parses a JSON string into a tree of `JsonItem` nodes which can then be queried using a clean, type-safe macro API. All types, macros, and public declarations live in `json_deserializer.h`; the implementation lives in `json_deserializer.c`.

The **public API is exactly three functions**:

```c
Error JsonObj_new(const char* json_string_p, JsonObj* out_json_obj_p);
void  JsonObj_destroy(JsonObj* json_obj_p);
// Json_get is a macro &mdash; see below
```

Everything else in the `.c` file is internal.

---

## Public API

### `JsonObj_new`

```c
Error JsonObj_new(const char* json_string_p, JsonObj* out_json_obj_p);
```

Parses a JSON string and populates a caller-provided `JsonObj`. Returns `ERR_ALL_GOOD` on success or an `Error` code on failure. The `JsonObj` must outlive any values retrieved from it, because string values are pointers into the internal buffer it owns.

### `JsonObj_destroy`

```c
void JsonObj_destroy(JsonObj* json_obj_p);
```

Frees all memory owned by the `JsonObj`, including the internal string buffer and all `JsonItem` nodes.

### `Json_get` &mdash; The Query Macro

```c
Json_get(json_stuff, needle, out_p)
```

This is the primary retrieval interface. It is a **C11 `_Generic` macro** that dispatches to the correct typed getter function based on the types of `json_stuff` and `out_p` at compile time. No casting or manual function selection is required.

| `json_stuff` type | `needle` type | `out_p` type | What it does |
|---|---|---|---|
| `JsonObj*` | `const char*` key | `const char**` | Get a string from the root object |
| `JsonObj*` | `const char*` key | `json_int_t*` | Get a signed integer |
| `JsonObj*` | `const char*` key | `json_uint_t*` | Get an unsigned integer |
| `JsonObj*` | `const char*` key | `json_decimal_t*` | Get a double |
| `JsonObj*` | `const char*` key | `json_bool_t*` | Get a boolean |
| `JsonObj*` | `const char*` key | `JsonItem**` | Get a nested object |
| `JsonObj*` | `const char*` key | `JsonArray**` | Get an array |
| `JsonItem*` | `const char*` key | *(any of above)* | Same, but starting from a nested item |
| `JsonArray*` | `size_t` index | *(any of above)* | Get an element from an array by index |

Example usage:

```c
JsonObj obj;
JsonObj_new("{\"name\":\"Alice\",\"age\":30}", &obj);

const char* name = NULL;
json_int_t  age  = 0;
Json_get(&obj, "name", &name);  // dispatches to obj_get_value_char_p
Json_get(&obj, "age",  &age);   // dispatches to obj_get_value_int

JsonObj_destroy(&obj);
```

For nested objects:
```c
JsonItem* person = NULL;
Json_get(&obj, "person", &person);  // get sub-object as JsonItem*

const char* city = NULL;
Json_get(person, "city", &city);    // drill into the sub-object
```

For arrays:
```c
JsonArray* scores = NULL;
Json_get(&obj, "scores", &scores);  // get array

json_int_t first = 0;
Json_get(scores, (size_t)0, &first); // index into array
```

All three forms of `Json_get` return an `Error` value.

---

## Type System

### `Error` enum

```c
typedef enum {
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
```

Two convenience macros make error checking ergonomic:

```c
#define is_err(_expr) ((_expr) != ERR_ALL_GOOD)
#define is_ok(_expr)  ((_expr) == ERR_ALL_GOOD)
```

And a propagation macro for internal use:

```c
#define return_on_err(_expr) { Error _res = _expr; if (_res != ERR_ALL_GOOD) { ...; return _res; } }
```

### Scalar type aliases

```c
typedef long long int        json_int_t;
typedef unsigned long long   json_uint_t;
typedef double               json_decimal_t;
typedef bool                 json_bool_t;
```

These aliases insulate the rest of the codebase from the underlying primitive types, and are the types used in `Json_get` dispatch.

### `ValueType` enum

```c
typedef enum {
    VALUE_ROOT, VALUE_UNDEFINED, VALUE_INT, VALUE_BOOL,
    VALUE_LLU, VALUE_DOUBLE, VALUE_STR, VALUE_ARRAY,
    VALUE_ITEM, VALUE_INVALID,
} ValueType;
```

Used in `JsonValue` to tag which union member is active.

### `JsonValue`

```c
typedef struct JsonValue {
    ValueType value_type;
    union {
        json_int_t        value_int;
        json_uint_t       value_llu;
        json_decimal_t    value_double;
        json_bool_t       value_bool;
        const char*       value_char_p;
        struct JsonItem*  value_child_p;
        struct JsonArray* value_array_p;
    };
} JsonValue;
```

A tagged union covering all JSON value types. The `value_type` tag must always be checked before reading the union.

### `JsonItem`

```c
typedef struct JsonItem {
    const char*      key_p;        // pointer into the JSON string buffer (object key)
    json_uint_t      index;        // position within an array
    JsonValue        value;        // the tagged-union value
    struct JsonItem* parent;       // parent node
    struct JsonItem* next_sibling; // next peer in object or array
} JsonItem;
```

Each node in the parsed tree. Siblings are linked list nodes; children are reached via `value.value_child_p`. `key_p` points directly into the (mutated) input buffer.

### `JsonArray`

```c
typedef struct JsonArray {
    struct JsonItem* element;
} JsonArray;
```

A thin wrapper that points to the first element `JsonItem` of an array. It only exists to give the `_Generic` dispatch in `Json_get` a distinct type to pattern-match on &mdash; the actual data is in the `JsonItem` chain.

### `JsonObj`

```c
typedef struct JsonObj {
    char*    json_string;  // owned, heap-allocated copy of the input (mutated in place)
    JsonItem root;         // dummy sentinel root node
} JsonObj;
```

The top-level container. It owns both the string buffer (which backs all string values) and the root node (stack-allocated as part of the struct). The root's `parent` pointer is set to itself as a sentinel; its `value_type` is `VALUE_ROOT`, which protects it from being overwritten during parsing.

---

## Internal Implementation

### Preprocessing: `_strip_whitespace_malloc`

Allocates a copy of the input with all whitespace outside quoted strings removed. This normalises the JSON before parsing.

### Validation: `_generate_tokens_malloc` + `_validate_tokens`

A two-step first pass: extract only the structural tokens (`{`, `}`, `[`, `]`, `,`, `:`, `"`) into a compact string, then walk it to verify `{}`/`[]` bracket balance using two counters. Returns `ERR_JSON_INVALID` on mismatch. This is a fast sanity check before the full parse begins.

### In-place string termination: `_terminate_str`

Rather than copying string values, this function scans forward from a `"` character until it finds a closing `"` followed by a delimiter, then **writes `\0` directly over the closing quote**. The `JsonItem.key_p` and `value.value_char_p` fields then point into the modified buffer. This is why string values are zero-copy &mdash; and why the `JsonObj` must stay alive while any values are in use.

### Core parser: `_deserialize`

A single-pass iterative loop over the whitespace-stripped string. Uses `curr_item_p` and the parent/sibling pointers to navigate the tree without recursion. The main dispatch uses `_get_value_type` to classify what's at the current read position, then handles each case:

- **`}`/`]`** &mdash; walks up to `curr_item_p->parent`
- **`[`** &mdash; creates a child item and marks the current item as `VALUE_ARRAY`
- **`,` in array context** &mdash; creates a new sibling, incrementing its `index`
- **`KEY`** (`{"` or `,"`) &mdash; creates a child or sibling item, extracts the key via `_terminate_str`, advances past `:`
- **Numbers** &mdash; scans into a buffer, dispatches to `sscanf` as `double`, `long long`, or `unsigned long long` depending on the presence of `.` or `-`; includes an overflow check for the unsigned case
- **`true`/`false`** &mdash; advances 4 or 5 characters, sets `VALUE_BOOL`
- **Strings** &mdash; points into the buffer past `"`, calls `_terminate_str` to null-terminate in place
- **`{}`** &mdash; skips the empty object

### Typed getters (X-macros)

All retrieval functions are generated by four X-macro families, with their declarations generated by matching macros in the `.h` file:

| Macro family | Generates |
|---|---|
| `GET_VALUE_c` / `GET_VALUE_h` | `get_value_*()` &mdash; search by key from a `JsonItem*` |
| `OBJ_GET_VALUE_c` / `OBJ_GET_VALUE_h` | `obj_get_value_*()` &mdash; search by key from a `JsonObj*` |
| `GET_NUMBER_c` | Same as above but with implicit numeric coercions (e.g. int-double) |
| `GET_ARRAY_VALUE_c` / `GET_ARRAY_VALUE_h` | `get_array_value_*()` &mdash; search by index in a `JsonArray*` |

`Json_get` is the single macro that routes to all of these based on compile-time types. Because `_Generic` resolves at compile time, there is no runtime overhead from the dispatch.

---

## Logging

The header defines six log levels (`TRACE`, `DEBUG`, `INFO`, `WARNING`, `ERROR`, `NO_LOGS`) controlled by the compile-time `LOG_LEVEL` flag. Log macros expand to `fprintf` calls that include timestamp, PID, filename, and line number, protected by `pthread` mutexes for thread safety. In test builds (`-DTEST`), all output is redirected to `stdout` to avoid polluting `stderr` and tripping test failure detection.

---

## Testing Infrastructure

When compiled with `-DTEST`, the header exposes:

- `ASSERT(value, message)` &mdash; boolean assertion
- `ASSERT_OK` / `ASSERT_ERR` &mdash; `Error` value assertions
- `ASSERT_EQ(v1, v2, message)` / `ASSERT_NE(v1, v2, message)` &mdash; type-safe equality/inequality checks, themselves implemented via `_Generic`, dispatching to the correct typed function for `char`, `int`, `long long`, `unsigned long long`, `bool`, `double`, and `char*`
- `PRINT_BANNER()` &mdash; prints a decorated test function header
- `PRINT_TEST_TITLE(title)` &mdash; labels individual sub-tests with an incrementing counter
- `test_json_deserializer()` &mdash; the main test entry point

---

## Summary

| Aspect | Approach |
|---|---|
| Public API surface | 2 functions + 1 macro: `JsonObj_new`, `JsonObj_destroy`, `Json_get` |
| Type dispatch | C11 `_Generic` in `Json_get` &mdash; fully compile-time, zero runtime cost |
| Memory model | One malloc for input string; nodes malloced individually |
| String storage | Zero-copy: `\0` written in-place, `JsonItem` holds raw pointer |
| Tree structure | Intrusive linked list (parent + next_sibling pointers in each node) |
| Parsing strategy | Single-pass iterative scan; no recursion |
| Validation | Token-only bracket balance check before full parse |
| Getter generation | X-macros expand into typed functions in both `.h` and `.c` |
| Error handling | `Error` enum returned from all functions; `is_ok`/`is_err` helpers |
| Thread safety | Logging only; the parser itself is not thread-safe across shared objects |

The design prioritises **minimal memory overhead, zero-copy string handling, and a clean type-safe call site**, at the cost of mutating the input buffer and O(n) key lookups via sibling-list traversal.

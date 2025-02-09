# JSON deserializer in C 
## General description

This library is part of the project [mylibc](https://github.com/antonioastorino/mylibc).
It allows to convert a valid JSON string into an structure.

The only limitation that I am aware of is that every JSON string passed to the parser needs to start with a '{', hence excluding JSON objects representing arrays of stuff, i.e., starting with '['.
The test file `test/assets/test_json_vector.json` will not work and is currently unused.
A simple workaround could be wrapping the JSON string with an object, hence by prepending `{some_dummy_name=` and appending `}` to the string.

I apologize for the misuse of the word "class" but my original intention was to create an OOP-like library.

## Examples
Plenty of examples are provided in the `test_class_json()`, contained in `src/class_json.c`.
Here is one:

```c
JsonObj json_obj;
const char* value_str;  // Used for retrieving a value of type const char*
size_t value_uint;      // Used for retrieving a value of type size_t
const char* json_char_p = " {\"key\": \"value string\", \"sibling\": 56}";

JsonObj_new(json_char_p, &json_obj)          // Should return ERR_ALL_GOOD;
Json_get(&json_obj, "key", &value_str);      // value_str should point to "value string"
Json_get(&json_obj, "sibling", &value_uint); // value_uint is 56
JsonObj_destroy(&json_obj);                  // Don't leak memory!
```

## How to build and test

```bash
./bin/makeMakefile.sh
./bin/run.sh test
```

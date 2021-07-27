#include <stdio.h>
#include <string.h>
#include "unity/src/unity.h"
#include "common.h"
#include "../examples/toml.h"

# define ASSERT_TOML(entry, input, code, output) do { \
    P4_Grammar* grammar = P4_CreateTomlGrammar(); \
    P4_Source* source = P4_CreateSource((input), (entry)); \
    TEST_ASSERT_EQUAL_MESSAGE((code), P4_Parse(grammar, source), "unexpected parse grammar return code"); \
    P4_Node* root = P4_GetSourceAst(source); \
    FILE *f = fopen("check.json","w"); \
    P4_JsonifySourceAst(grammar, f, root); \
    fclose(f); \
    P4_String s = read_file("check.json"); \
    printf("%s\n%s\n", input, s); \
    free(s); \
    P4_DeleteSource(source); \
    P4_DeleteGrammar(grammar); \
} while (0);

void test_valid(void) {
    ASSERT_TOML(1, "abc = true # comment", P4_Ok, "[]");
    ASSERT_TOML(1,
        "abc = true\n"
        "xyz = false", P4_Ok, "[]");
    ASSERT_TOML(1,
        "a.b.c = true\n"
        "x.y.z = false", P4_Ok, "[]");
    ASSERT_TOML(1, "abc = \"xyz\"", P4_Ok, "[]");
    ASSERT_TOML(1, "abc = \"\"", P4_Ok, "[]");
    ASSERT_TOML(1, "abc = \"a\\u0031\\U00000032\\n\"", P4_Ok, "[]");
    ASSERT_TOML(1, "\"abc\" = \"\"", P4_Ok, "[]");
    ASSERT_TOML(1, "\"a\\u0031c\" = \"\"", P4_Ok, "[]");
    ASSERT_TOML(1, "a.\"b\".c = \"\"", P4_Ok, "[]");
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_valid);
    return UNITY_END();
}

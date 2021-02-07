#include "unity/src/unity.h"
#include "common.h"

/*
 * Rules:
 *  ENTRY = R1
 *  R1 = "HELLO"
 * Input:
 *  "HELLO"
 * Output:
 *   ENTRY:
 *     R1: "HELLO"
 */
P4_PRIVATE(void) test_match_reference_successfully(void) {
# define ENTRY  1
# define R1     2
    P4_Grammar* grammar = P4_CreateGrammar();
    TEST_ASSERT_NOT_NULL(grammar);
    TEST_ASSERT_EQUAL(
        P4_Ok,
        P4_AddReference(grammar, ENTRY, R1)
    );
    TEST_ASSERT_EQUAL(
        P4_Ok,
        P4_AddLiteral(grammar, R1, "HELLO", true)
    );

    P4_Source* source = P4_CreateSource("HELLO", ENTRY);
    TEST_ASSERT_NOT_NULL(source);
    TEST_ASSERT_EQUAL(
        P4_Ok,
        P4_Parse(grammar, source)
    );

    P4_Token* token = P4_GetSourceAst(source);

    TEST_ASSERT_NOT_NULL(token);
    TEST_ASSERT_EQUAL(token->expr, P4_GetGrammarRule(grammar, ENTRY));
    autofree P4_String tokenstr = P4_CopyTokenString(token);
    TEST_ASSERT_EQUAL_STRING("HELLO", tokenstr);

    TEST_ASSERT_NOT_NULL(token->head);
    TEST_ASSERT_EQUAL(token->head, token->tail);
    TEST_ASSERT_EQUAL(token->head->expr, P4_GetGrammarRule(grammar, R1));
    autofree P4_String r1str = P4_CopyTokenString(token->head);
    TEST_ASSERT_EQUAL_STRING("HELLO", r1str);

    P4_DeleteSource(source);
    P4_DeleteGrammar(grammar);
}

/*
 * Rules:
 *  ENTRY = R1
 * Input:
 *  "HELLO"
 * Error:
 *  P4_NameError
 * Output:
 *  NULL
 */
P4_PRIVATE(void) test_match_reference_not_defined_raise_name_error(void) {
# define ENTRY  1
# define R1     2
    P4_Grammar* grammar = P4_CreateGrammar();
    TEST_ASSERT_NOT_NULL(grammar);
    TEST_ASSERT_EQUAL(
        P4_Ok,
        P4_AddReference(grammar, ENTRY, R1)
    );

    P4_Source* source = P4_CreateSource("HELLO", ENTRY);
    TEST_ASSERT_NOT_NULL(source);
    TEST_ASSERT_EQUAL(
        P4_NameError,
        P4_Parse(grammar, source)
    );

    P4_Token* token = P4_GetSourceAst(source);

    TEST_ASSERT_NULL(token);

    P4_DeleteSource(source);
    P4_DeleteGrammar(grammar);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_match_reference_successfully);
    RUN_TEST(test_match_reference_not_defined_raise_name_error);

    return UNITY_END();
}

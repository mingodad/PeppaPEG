# ifndef P4_LANG_PEPPA_H
# define P4_LANG_PEPPA_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include "../peppapeg.h"

P4_Error P4_P4GenEval(P4_Token* token, void* result);

# define SLICE_LEN(s) ((s)->stop.pos - (s)->start.pos)

typedef enum {
    P4_P4GenGrammar         = 1,
    P4_P4GenRule            = 2,
    P4_P4GenRuleDecorators  = 3,
    P4_P4GenRuleName        = 4,
    P4_P4GenExpression,
    P4_P4GenPrimary,
    P4_P4GenDecorator,
    P4_P4GenIdentifier,
    P4_P4GenLiteral,
    P4_P4GenRange,
    P4_P4GenReference,
    P4_P4GenPositive,
    P4_P4GenNegative,
    P4_P4GenSequence,
    P4_P4GenChoice,
    P4_P4GenBackReference,
    P4_P4GenRepeat,
    P4_P4GenRepeatOnceOrMore,
    P4_P4GenRepeatZeroOrMore,
    P4_P4GenRepeatZeroOrOnce,
    P4_P4GenRepeatMin,
    P4_P4GenRepeatMax,
    P4_P4GenRepeatExact,
    P4_P4GenRepeatMinMax,
    P4_P4GenNumber,
    P4_P4GenChar,
    P4_P4GenWhitespace,
} P4_P4GenRuleID;

P4_Grammar* P4_CreateP4GenGrammar ();
P4_String   P4_P4GenKindToName(P4_RuleID);

P4_Grammar* P4_CreateP4GenGrammar () {
    P4_Grammar* grammar = P4_CreateGrammar();
    P4_Error err = 0;

    if (P4_Ok != P4_AddChoiceWithMembers(grammar, P4_P4GenNumber, 2,
        P4_CreateLiteral("0", true),
        P4_CreateSequenceWithMembers(2,
            P4_CreateRange('1', '9', 1),
            P4_CreateZeroOrMore(P4_CreateRange('0', '9', 1))
        )
    ))
        goto finalize;

    if (P4_Ok != P4_SetGrammarRuleFlag(grammar, P4_P4GenNumber, P4_FLAG_SQUASHED | P4_FLAG_TIGHT))
        goto finalize;

    if (P4_Ok != P4_AddChoiceWithMembers(grammar, P4_P4GenChar, 4,
        P4_CreateRange(0x20, 0x21, 1), /* Can't be 0x22: double quote " */
        P4_CreateRange(0x23, 0x5b, 1), /* Can't be 0x5c: escape leading \ */
        P4_CreateRange(0x5d, 0x10ffff, 1),
        P4_CreateSequenceWithMembers(2,
            P4_CreateLiteral("\\", true),
            P4_CreateChoiceWithMembers(9,
                P4_CreateLiteral("\"", true),
                P4_CreateLiteral("/", true),
                P4_CreateLiteral("\\", true),
                P4_CreateLiteral("b", true),
                P4_CreateLiteral("f", true),
                P4_CreateLiteral("n", true),
                P4_CreateLiteral("r", true),
                P4_CreateLiteral("t", true),
                P4_CreateSequenceWithMembers(2,
                    P4_CreateLiteral("u", true),
                    P4_CreateRepeatExact(
                        P4_CreateChoiceWithMembers(3,
                            P4_CreateRange('0', '9', 1),
                            P4_CreateRange('a', 'f', 1),
                            P4_CreateRange('A', 'F', 1)
                        ), 4
                    )
                )
            )
        )
    ))
        goto finalize;

    if (P4_Ok != P4_SetGrammarRuleFlag(grammar, P4_P4GenChar, P4_FLAG_SQUASHED | P4_FLAG_TIGHT))
        goto finalize;

    if (P4_Ok != P4_AddSequenceWithMembers(grammar, P4_P4GenLiteral, 3,
        P4_CreateLiteral("\"", true),
        P4_CreateZeroOrMore(P4_CreateReference(P4_P4GenChar)),
        P4_CreateLiteral("\"", true)
    ))
        goto finalize;

    if (P4_Ok != P4_SetGrammarRuleFlag(grammar, P4_P4GenLiteral, P4_FLAG_SQUASHED | P4_FLAG_TIGHT))
        goto finalize;

    if (P4_Ok != P4_AddSequenceWithMembers(grammar, P4_P4GenRange, 6,
        P4_CreateLiteral("[", true),
        P4_CreateReference(P4_P4GenChar),
        P4_CreateLiteral("-", true),
        P4_CreateReference(P4_P4GenChar),
        P4_CreateZeroOrOnce(P4_CreateSequenceWithMembers(2,
            P4_CreateLiteral("..", true),
            P4_CreateReference(P4_P4GenNumber)
        )),
        P4_CreateLiteral("]", true)
    ))
        goto finalize;

    if (P4_Ok != P4_AddSequenceWithMembers(grammar, P4_P4GenReference, 2,
        P4_CreateChoiceWithMembers(3,
            P4_CreateRange('a', 'z', 1),
            P4_CreateRange('A', 'Z', 1),
            P4_CreateLiteral("_", true)
        ),
        P4_CreateZeroOrMore(
            P4_CreateChoiceWithMembers(4,
                P4_CreateRange('a', 'z', 1),
                P4_CreateRange('A', 'Z', 1),
                P4_CreateRange('0', '9', 1),
                P4_CreateLiteral("_", true)
            )
        )
    ))
        goto finalize;

    if (P4_Ok != P4_SetGrammarRuleFlag(grammar, P4_P4GenReference, P4_FLAG_SQUASHED | P4_FLAG_TIGHT))
        goto finalize;

    if (P4_Ok != P4_AddSequenceWithMembers(grammar, P4_P4GenPositive, 2,
        P4_CreateLiteral("&", true),
        P4_CreateReference(P4_P4GenPrimary)
    ))
        goto finalize;

    if (P4_Ok != P4_AddSequenceWithMembers(grammar, P4_P4GenNegative, 2,
        P4_CreateLiteral("!", true),
        P4_CreateReference(P4_P4GenPrimary)
    ))
        goto finalize;

    if (P4_Ok != P4_AddLiteral(grammar, P4_P4GenRepeatOnceOrMore, "+", true))
        goto finalize;

    if (P4_Ok != P4_AddLiteral(grammar, P4_P4GenRepeatZeroOrMore, "*", true))
        goto finalize;

    if (P4_Ok != P4_AddLiteral(grammar, P4_P4GenRepeatZeroOrOnce, "?", true))
        goto finalize;

    if (P4_Ok != P4_AddSequenceWithMembers(grammar, P4_P4GenRepeatMin, 4,
        P4_CreateLiteral("{", true),
        P4_CreateReference(P4_P4GenNumber),
        P4_CreateLiteral(",", true),
        P4_CreateLiteral("}", true)
    ))
        goto finalize;

    if (P4_Ok != P4_AddSequenceWithMembers(grammar, P4_P4GenRepeatMax, 4,
        P4_CreateLiteral("{", true),
        P4_CreateLiteral(",", true),
        P4_CreateReference(P4_P4GenNumber),
        P4_CreateLiteral("}", true)
    ))
        goto finalize;

    if (P4_Ok != P4_AddSequenceWithMembers(grammar, P4_P4GenRepeatMinMax, 5,
        P4_CreateLiteral("{", true),
        P4_CreateReference(P4_P4GenNumber),
        P4_CreateLiteral(",", true),
        P4_CreateReference(P4_P4GenNumber),
        P4_CreateLiteral("}", true)
    ))
        goto finalize;

    if (P4_Ok != P4_AddSequenceWithMembers(grammar, P4_P4GenRepeatExact, 3,
        P4_CreateLiteral("{", true),
        P4_CreateReference(P4_P4GenNumber),
        P4_CreateLiteral("}", true)
    ))
        goto finalize;

    if (P4_Ok != P4_AddSequenceWithMembers(grammar, P4_P4GenRepeat, 2,
        P4_CreateReference(P4_P4GenPrimary),
        P4_CreateZeroOrOnce(
            P4_CreateChoiceWithMembers(7,
                P4_CreateReference(P4_P4GenRepeatOnceOrMore),
                P4_CreateReference(P4_P4GenRepeatZeroOrMore),
                P4_CreateReference(P4_P4GenRepeatZeroOrOnce),
                P4_CreateReference(P4_P4GenRepeatExact),
                P4_CreateReference(P4_P4GenRepeatMinMax),
                P4_CreateReference(P4_P4GenRepeatMin),
                P4_CreateReference(P4_P4GenRepeatMax)
            )
        )
    ))
        goto finalize;

    if (P4_Ok != P4_SetGrammarRuleFlag(grammar, P4_P4GenRepeat, P4_FLAG_NON_TERMINAL))
        goto finalize;

    if (P4_Ok != P4_AddChoiceWithMembers(grammar, P4_P4GenPrimary, 6,
        P4_CreateReference(P4_P4GenLiteral),
        P4_CreateReference(P4_P4GenRange),
        P4_CreateSequenceWithMembers(2,
            P4_CreateReference(P4_P4GenReference),
            P4_CreateNegative(P4_CreateLiteral("=", true))
        ),
        P4_CreateReference(P4_P4GenPositive),
        P4_CreateReference(P4_P4GenNegative),
        P4_CreateSequenceWithMembers(3,
            P4_CreateLiteral("(", true),
            P4_CreateReference(P4_P4GenChoice),
            P4_CreateLiteral(")", true)
        )
    ))
        goto finalize;

    if (P4_Ok != P4_SetGrammarRuleFlag(grammar, P4_P4GenPrimary, P4_FLAG_LIFTED))
        goto finalize;

    if (P4_Ok != P4_AddJoin(grammar, P4_P4GenChoice, "/", P4_P4GenSequence))
        goto finalize;

    if (P4_Ok != P4_SetGrammarRuleFlag(grammar, P4_P4GenChoice, P4_FLAG_NON_TERMINAL))
        goto finalize;

    if (P4_Ok != P4_AddOnceOrMore(grammar, P4_P4GenSequence,
                P4_CreateReference(P4_P4GenRepeat)))
        goto finalize;

    if (P4_Ok != P4_SetGrammarRuleFlag(grammar, P4_P4GenSequence, P4_FLAG_NON_TERMINAL))
        goto finalize;

    if (P4_Ok != P4_AddReference(grammar, P4_P4GenExpression, P4_P4GenChoice))
        goto finalize;

    if (P4_Ok != P4_SetGrammarRuleFlag(grammar, P4_P4GenExpression, P4_FLAG_LIFTED))
        goto finalize;

    if (P4_Ok != P4_AddReference(grammar, P4_P4GenRuleName, P4_P4GenReference))
        goto finalize;

    if (P4_Ok != P4_SetGrammarRuleFlag(grammar, P4_P4GenRuleName, P4_FLAG_SQUASHED))
        goto finalize;

    if (P4_Ok != P4_AddSequenceWithMembers(grammar, P4_P4GenDecorator, 2,
        P4_CreateLiteral("@", true),
        P4_CreateChoiceWithMembers(6,
            P4_CreateLiteral("squashed", true),
            P4_CreateLiteral("scoped", true),
            P4_CreateLiteral("spaced", true),
            P4_CreateLiteral("lifted", true),
            P4_CreateLiteral("tight", true),
            P4_CreateLiteral("nonterminal", true)
        )
    ))
        goto finalize;

    if (P4_Ok != P4_AddZeroOrMore(grammar, P4_P4GenRuleDecorators,
            P4_CreateReference(P4_P4GenDecorator)
    ))
        goto finalize;

    if (P4_Ok != P4_AddSequenceWithMembers(grammar, P4_P4GenRule, 5,
        P4_CreateReference(P4_P4GenRuleDecorators),
        P4_CreateReference(P4_P4GenRuleName),
        P4_CreateLiteral("=", true),
        P4_CreateReference(P4_P4GenExpression),
        P4_CreateLiteral(";", true)
    ))
        goto finalize;

    if (P4_Ok != P4_AddOnceOrMore(grammar, P4_P4GenGrammar, P4_CreateReference(P4_P4GenRule)))
        goto finalize;

    if (P4_Ok != P4_AddChoiceWithMembers(grammar, P4_P4GenWhitespace, 4,
        P4_CreateLiteral(" ", true),
        P4_CreateLiteral("\t", true),
        P4_CreateLiteral("\r", true),
        P4_CreateLiteral("\n", true)
    ))
        goto finalize;

    if (P4_Ok != P4_SetGrammarRuleFlag(grammar, P4_P4GenWhitespace,
                P4_FLAG_LIFTED | P4_FLAG_SPACED))
        goto finalize;

    return grammar;

finalize:
    P4_DeleteGrammar(grammar);
    return NULL;
}

void* P4_P4GenConcatRune(void *str, P4_Rune chr, size_t n) {
  char *s = (char *)str;

  if (0 == ((P4_Rune)0xffffff80 & chr)) {
    /* 1-byte/7-bit ascii (0b0xxxxxxx) */
    if (n < 1) {
      return 0;
    }
    s[0] = (char)chr;
    s += 1;
  } else if (0 == ((P4_Rune)0xfffff800 & chr)) {
    /* 2-byte/11-bit utf8 code point (0b110xxxxx 0b10xxxxxx) */
    if (n < 2) {
      return 0;
    }
    s[0] = 0xc0 | (char)((chr >> 6) & 0x1f);
    s[1] = 0x80 | (char)(chr & 0x3f);
    s += 2;
  } else if (0 == ((P4_Rune)0xffff0000 & chr)) {
    /* 3-byte/16-bit utf8 code point (0b1110xxxx 0b10xxxxxx 0b10xxxxxx) */
    if (n < 3) {
      return 0;
    }
    s[0] = 0xe0 | (char)((chr >> 12) & 0x0f);
    s[1] = 0x80 | (char)((chr >> 6) & 0x3f);
    s[2] = 0x80 | (char)(chr & 0x3f);
    s += 3;
  } else { /* if (0 == ((int)0xffe00000 & chr)) { */
    /* 4-byte/21-bit utf8 code point (0b11110xxx 0b10xxxxxx 0b10xxxxxx 0b10xxxxxx) */
    if (n < 4) {
      return 0;
    }
    s[0] = 0xf0 | (char)((chr >> 18) & 0x07);
    s[1] = 0x80 | (char)((chr >> 12) & 0x3f);
    s[2] = 0x80 | (char)((chr >> 6) & 0x3f);
    s[3] = 0x80 | (char)(chr & 0x3f);
    s += 4;
  }

  return s;
}

P4_String   P4_P4GenKindToName(P4_RuleID id) {
    switch (id) {
        case P4_P4GenNumber: return "number";
        case P4_P4GenChar: return "char";
        case P4_P4GenLiteral: return "literal";
        case P4_P4GenRange: return "range";
        case P4_P4GenReference: return "reference";
        case P4_P4GenPositive: return "positive";
        case P4_P4GenNegative: return "negative";
        case P4_P4GenChoice: return "choice";
        case P4_P4GenSequence: return "sequence";
        case P4_P4GenRepeat: return "repeat";
        case P4_P4GenRepeatOnceOrMore: return "onceormore";
        case P4_P4GenRepeatZeroOrMore: return "zeroormore";
        case P4_P4GenRepeatZeroOrOnce: return "zerooronce";
        case P4_P4GenRepeatMin: return "repeatmin";
        case P4_P4GenRepeatMax: return "repeatmax";
        case P4_P4GenRepeatMinMax: return "repeatminmax";
        case P4_P4GenRepeatExact: return "repeatexact";
        case P4_P4GenRuleName: return "name";
        case P4_P4GenRuleDecorators: return "decorators";
        case P4_P4GenDecorator: return "decorator";
        case P4_P4GenRule: return "rule";
        case P4_P4GenGrammar: return "grammar";
        default: return "<unknown>";
    }
}

P4_Error P4_P4GenEvalFlag(P4_Token* token, P4_ExpressionFlag *flag) {
    P4_String token_str = token->text + token->slice.start.pos; /* XXX: need slice api. */
    size_t token_len = SLICE_LEN(&token->slice); /* XXX: need slice api. */

    if (memcmp("@squashed", token_str, token_len) == 0)
        *flag = P4_FLAG_SQUASHED;
    else if (memcmp("@scoped", token_str, token_len) == 0)
        *flag = P4_FLAG_SCOPED;
    else if (memcmp("@spaced", token_str, token_len) == 0)
        *flag = P4_FLAG_SPACED;
    else if (memcmp("@lifted", token_str, token_len) == 0)
        *flag = P4_FLAG_LIFTED;
    else if (memcmp("@tight", token_str, token_len) == 0)
        *flag = P4_FLAG_TIGHT;
    else if (memcmp("@nonterminal", token_str, token_len) == 0)
        *flag = P4_FLAG_NON_TERMINAL;
    else {
        *flag = 0; return P4_ValueError;
    }

    return P4_Ok;
}

P4_Error P4_P4GenEvalRuleFlags(P4_Token* token, P4_ExpressionFlag* flag) {
    P4_Token* child = NULL;
    P4_ExpressionFlag child_flag = 0;
    P4_Error err = P4_Ok;
    for (child = token->head; child != NULL; child = child->next) {
        if ((err = P4_P4GenEvalFlag(child, &child_flag)) != P4_Ok) {
            *flag = 0;
            return err;
        }
        *flag |= child_flag;
    }
    return P4_Ok;
}

P4_Error P4_P4GenEvalNumber(P4_Token* token, size_t* num) {
    P4_String s = P4_CopyTokenString(token);

    if (s == NULL)
        return P4_MemoryError;

    *num = atol(s);
    free(s);

    return P4_Ok;
}

size_t P4_P4GenCopyRune(P4_String text, size_t start, size_t stop, P4_Rune* rune) {
    char ch0 = text[start];
    if (ch0 == '\\') {
        char ch1 = text[start+1];
        switch (ch1) {
            case 'b': *rune = 0x8; return 2;
            case 't': *rune = 0x9; return 2;
            case 'n': *rune = 0xa; return 2;
            case 'f': *rune = 0xc; return 2;
            case 'r': *rune = 0xd; return 2;
            case '"': *rune = 0x22; return 2;
            case '/': *rune = 0x2f; return 2;
            case '\\': *rune = 0x5c; return 2;
            case 'u': {
                char chs[5] = {0, 0, 0, 0, 0};
                memcpy(chs, text + start + 2, 4);
                *rune = strtoul(chs, NULL, 16);
                return 6;
            }
            default: return 0;
        }
    } else {
        return P4_ReadRune(text+start, rune);
    }
}

P4_Error P4_P4GenEvalChar(P4_Token* token, P4_Rune* rune) {
    size_t size = P4_P4GenCopyRune(
        token->text, token->slice.start.pos, token->slice.stop.pos, rune
    );
    if (size == 0) return P4_ValueError;
    return P4_Ok;
}

P4_Error P4_P4GenEvalLiteral(P4_Token* token, P4_Expression** expr) {
    size_t len = SLICE_LEN(&token->slice) - 2; /* remove quotes */
    if (len < 0)
        return P4_ValueError;

    size_t i = 0,
           size = 0;
    P4_Error err = 0;
    P4_Rune rune = 0;
    P4_String lit = malloc((len+1) * sizeof(char)),
              cur = lit;

    if (lit == NULL)
        return P4_MemoryError;

    for (i = token->slice.start.pos+1; i < token->slice.stop.pos-1; i += size) {
        size = P4_P4GenCopyRune(token->text, i, token->slice.stop.pos-1, &rune);
        if (size == 0) {
            err = P4_ValueError;
            goto finalize;
        }
        cur = P4_P4GenConcatRune(cur, rune, size);
    }
    *cur = '\0';

    *expr = P4_CreateLiteral(lit, true);

finalize:
    free(lit);
    return err;
}

P4_Error P4_P4GenEvalRange(P4_Token* token, P4_Expression** expr) {
    P4_Error err = P4_Ok;
    P4_Rune lower = 0, upper = 0;

    if ((err = P4_P4GenEvalChar(token->head, &lower)) != P4_Ok)
        return err;

    if ((err = P4_P4GenEvalChar(token->tail, &upper)) != P4_Ok)
        return err;

    if (lower > upper || lower == 0 || upper == 0)
        return P4_ValueError;

    *expr = P4_CreateRange(lower, upper, 1);
    if (*expr == NULL)
        return P4_MemoryError;

    return P4_Ok;
}

size_t P4_P4GenGetChildrenCount(P4_Token* token) {
    P4_Token* child = token->head;

    size_t    child_count = 0;
    while (child != NULL) {
        child_count++; child = child->next;
    }

    return child_count;
}

P4_Error P4_P4GenEvalMembers(P4_Token* token, P4_Expression* expr) {
    size_t i = 0;
    P4_Token* child = token->head;
    P4_Error err = P4_Ok;
    while (child != NULL) {
        P4_Expression* child_expr = NULL;
        if ((err = P4_P4GenEval(child, &child_expr)) != P4_Ok)
            return err;
        P4_SetMember(expr, i, child_expr);
        child = child->next;
        i++;
    }
    return P4_Ok;
}

P4_Error P4_P4GenEvalSequence(P4_Token* token, P4_Expression** expr) {
    P4_Error  err = P4_Ok;

    if ((*expr = P4_CreateSequence(P4_P4GenGetChildrenCount(token))) == NULL) {
        err = P4_MemoryError;
        goto finalize;
    }

    if ((err = P4_P4GenEvalMembers(token, *expr)) != P4_Ok)
        goto finalize;

    return P4_Ok;

finalize:
    P4_DeleteExpression(*expr);
    return err;
}

P4_Error P4_P4GenEvalChoice(P4_Token* token, P4_Expression** expr) {
    P4_Error  err = P4_Ok;

    if ((*expr = P4_CreateChoice(P4_P4GenGetChildrenCount(token))) == NULL) {
        err = P4_MemoryError;
        goto finalize;
    }

    if ((err = P4_P4GenEvalMembers(token, *expr)) != P4_Ok)
        goto finalize;

    return P4_Ok;

finalize:
    P4_DeleteExpression(*expr);
    return err;
}

P4_Error P4_P4GenEvalPositive(P4_Token* token, P4_Expression** expr) {
    P4_Error        err = P4_Ok;
    P4_Expression*  ref = NULL;

    if ((err = P4_P4GenEval(token->head, &ref)) != P4_Ok) {
        return err;
    }

    if (ref == NULL)
        return P4_MemoryError;

    if ((*expr = P4_CreatePositive(ref)) == NULL) {
        err = P4_MemoryError;
        goto finalize;
    }

    return P4_Ok;

finalize:
    P4_DeleteExpression(ref);
    P4_DeleteExpression(*expr);
    return err;
}

P4_Error P4_P4GenEvalNegative(P4_Token* token, P4_Expression** expr) {
    P4_Error        err = P4_Ok;
    P4_Expression*  ref = NULL;

    if ((err = P4_P4GenEval(token->head, &ref)) != P4_Ok) {
        return err;
    }

    if (ref == NULL)
        return P4_MemoryError;

    if ((*expr = P4_CreateNegative(ref)) == NULL) {
        err = P4_MemoryError;
        goto finalize;
    }

    return P4_Ok;

finalize:
    P4_DeleteExpression(ref);
    P4_DeleteExpression(*expr);
    return err;
}

P4_Error P4_P4GenEvalRepeat(P4_Token* token, P4_Expression** expr) {
    P4_Error        err = P4_Ok;
    P4_Expression*  ref = NULL;
    size_t          min = 0, max = SIZE_MAX;

    if ((err = P4_P4GenEval(token->head, &ref)) != P4_Ok) {
        return err;
    }

    if (ref == NULL)
        return P4_MemoryError;

    switch (token->head->next->rule_id) {
        case P4_P4GenRepeatZeroOrMore: min = 0; max = SIZE_MAX; break;
        case P4_P4GenRepeatZeroOrOnce: min = 0; max = 1; break;
        case P4_P4GenRepeatOnceOrMore: min = 1; max = SIZE_MAX; break;
        case P4_P4GenRepeatMin:
            if ((err = P4_P4GenEvalNumber(token->head->next->head, &min)) != P4_Ok)
                goto finalize;
            break;
        case P4_P4GenRepeatMax:
            if ((err = P4_P4GenEvalNumber(token->head->next->head, &max)) != P4_Ok)
                goto finalize;
            break;
        case P4_P4GenRepeatMinMax:
            if ((err = P4_P4GenEvalNumber(token->head->next->head, &min)) != P4_Ok)
                goto finalize;
            if ((err = P4_P4GenEvalNumber(token->head->next->tail, &max)) != P4_Ok)
                goto finalize;
            break;
        case P4_P4GenRepeatExact:
            if ((err = P4_P4GenEvalNumber(token->head->next->head, &min)) != P4_Ok)
                goto finalize;
            max = min;
            break;
        default:
            err = P4_ValueError;
            goto finalize;
    }

    if (min > max) {
        err = P4_ValueError;
        goto finalize;
    }


    if ((*expr = P4_CreateRepeatMinMax(ref, min, max)) == NULL) {
        err = P4_MemoryError;
        goto finalize;
    }

    return P4_Ok;

finalize:
    P4_DeleteExpression(ref);
    P4_DeleteExpression(*expr);
    return err;
}

P4_Error P4_P4GenEvalRuleName(P4_Token* token, P4_String* result) {
    size_t i = 0;
    size_t len = SLICE_LEN(&token->slice); /* remove quotes */
    if (len <= 0)
        return P4_ValueError;

    *result = malloc((len+1) * sizeof(char));
    if (*result == NULL)
        return P4_MemoryError;

    memcpy(*result, token->text + token->slice.start.pos, len);
    (*result)[len] = '\0';

    return P4_Ok;
}

P4_Error P4_P4GenEvalReference(P4_Token* token, P4_Expression** result) {
    P4_Error err = P4_Ok;
    P4_String reference = NULL;

    if ((err = P4_P4GenEvalRuleName(token, &reference)) != P4_Ok)
        return err;

    if (reference == NULL)
        return P4_ValueError;

    /* We can't know the ref_id at this stage.
     * So, let's just simply create a placeholder.
     */
    if ((*result = P4_CreateReference(SIZE_MAX)) == NULL) {
        err = P4_MemoryError;
        goto finalize;
    }

    /* The string reference is set.
     * When the full grammar is evaluated, we will refresh all ref_ids.
     */
    (*result)->reference = reference;

    return P4_Ok;

finalize:
    if (reference)
        free(reference);

    return err;
}

P4_Error P4_P4GenEvalGrammarRule(P4_Token* token, P4_Expression** result) {
    P4_String           rule_name = NULL;
    P4_ExpressionFlag   rule_flag = 0;
    P4_Token*           child     = NULL;
    P4_Error            err       = P4_Ok;

    *result = NULL;

    for (child = token->head; child != NULL; child = child->next) {
        switch (child->rule_id) {
            case P4_P4GenRuleDecorators:
                err = P4_P4GenEvalRuleFlags(child, &rule_flag);
                break;
            case P4_P4GenRuleName:
                err = P4_P4GenEvalRuleName(child, &rule_name);
                break;
            default:
                err = P4_P4GenEval(child, result);
                break;
        }
        if (err != P4_Ok)
            goto finalize;
    }

    (*result)->name = rule_name;
    (*result)->flag = rule_flag;

    return P4_Ok;

finalize:
    if (rule_name) free(rule_name);
    if (*result) P4_DeleteExpression(*result);
    return err;
}

P4_Error P4_P4GenSetReferenceIDs(P4_Grammar* grammar, P4_Expression* expr) {
    if (expr == NULL)
        return P4_NullError;

    size_t i = 0;
    P4_Error err = P4_Ok;
    switch (expr->kind) {
        case P4_Positive:
        case P4_Negative:
            if (expr->ref_expr)
                err = P4_P4GenSetReferenceIDs(grammar, expr->ref_expr);
            break;
        case P4_Sequence:
        case P4_Choice:
            for (i = 0; i < expr->count; i++)
                if (expr->members[i])
                    if ((err = P4_P4GenSetReferenceIDs(grammar, expr->members[i])) != P4_Ok)
                        return err;
            break;
        case P4_Repeat:
            if (expr->repeat_expr)
                err = P4_P4GenSetReferenceIDs(grammar, expr->repeat_expr);
            break;
        case P4_Reference:
        {
            if (expr->reference == NULL)
                return P4_NullError;

            P4_Expression* ref = P4_GetGrammarRuleByName(grammar, expr->reference);
            if (expr == NULL)
                return P4_NameError;

            expr->ref_id = ref->id;
            break;
        }
        default:
            break;
    }

    return err;
}

P4_Error P4_P4GenEvalGrammar(P4_Token* token, P4_Grammar** result) {
    P4_Error    err = P4_Ok;
    size_t      i = 0;
    size_t      count = P4_P4GenGetChildrenCount(token);

    if ((*result = P4_CreateGrammar()) == NULL) {
        err = P4_MemoryError;
        goto finalize;
    }

    P4_Token* child = NULL;
    P4_RuleID id = 1;
    for (child = token->head; child != NULL; child = child->next) {
        P4_Expression* rule = NULL;

        if ((err = P4_P4GenEvalGrammarRule(child, &rule)) != P4_Ok) {
            goto finalize;
        }

        if ((err = P4_AddGrammarRule(*result, id, rule)) != P4_Ok) {
            goto finalize;
        }

        id++;
    }

    for (i = 0; i < (*result)->count; i++) {
        if ((err = P4_P4GenSetReferenceIDs(*result, (*result)->rules[i])) != P4_Ok) {
            goto finalize;
        }
    }

finalize:
    if (err)
        P4_DeleteGrammar(*result);

    return err;
}

P4_Error P4_P4GenEval(P4_Token* token, void* result) {
    P4_Error err = P4_Ok;
    switch (token->rule_id) {
        case P4_P4GenDecorator:
            return P4_P4GenEvalFlag(token, result);
        case P4_P4GenRuleDecorators:
            return P4_P4GenEvalRuleFlags(token, result);
        case P4_P4GenNumber:
            return P4_P4GenEvalNumber(token, result);
        case P4_P4GenChar:
            return P4_P4GenEvalChar(token, result);
        case P4_P4GenLiteral:
            return P4_P4GenEvalLiteral(token, result);
        case P4_P4GenRange:
            return P4_P4GenEvalRange(token, result);
        case P4_P4GenSequence:
            return P4_P4GenEvalSequence(token, result);
        case P4_P4GenChoice:
            return P4_P4GenEvalChoice(token, result);
        case P4_P4GenPositive:
            return P4_P4GenEvalPositive(token, result);
        case P4_P4GenNegative:
            return P4_P4GenEvalNegative(token, result);
        case P4_P4GenRepeat:
            return P4_P4GenEvalRepeat(token, result);
        case P4_P4GenReference:
            return P4_P4GenEvalReference(token, result);
        case P4_P4GenGrammar:
            return P4_P4GenEvalGrammar(token, result);
        default: return P4_ValueError;
    }
    return P4_Ok;
}

P4_String P4_P4GenStringifyExpression(P4_Expression* expr) {
    P4_String result = NULL;
    switch( expr->kind ) {
        case P4_Literal:
        {
            size_t size = strlen(expr->literal) + 2 + 1;
            if (expr->sensitive) size++;
            result = malloc(size * sizeof(char));

            char* start = expr->sensitive ? "i\"" : "\"";
            char* stop = "\"";

            sprintf(result, "%s%s%s", start, expr->literal, stop);
            return result;
        }
        case P4_Range:
        {
            /* TBD */
        }
        /* TBD */
        default:
            return strdup("<unknown>");
    }
}

#ifdef __cplusplus
}
#endif

# endif

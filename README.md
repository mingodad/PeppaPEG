# Peppa PEG

Ultra lightweight PEG Parser in ANSI C. ✨ 🐷 ✨.

## Hello, There!

Want to parse a programming language using PEG? Use Peppa PEG!

## Usage

### Copy `peppapeg.h` / `peppapeg.c`.

Peppa PEG has one header file and one C file, so you can easily add
it to your project by copying files "peppapeg.h" and "peppapeg.c".

Peppa PEG assumes your project is ANSI C (C89, or C90) compatible.

Once copied, add include macro and start using the library!

```
#include "peppapeg.h"
```

### Basic Usage

All of the data structures and functions provided by Peppa PEG
start with `P4_`. ;) \
Just count how many `P`s there are in the name `Peppa PEG`!

Struct `P4_Grammar` represents a grammar of many PEG rules.

You can create such a data structure by `P4_CreateGrammar()`.

```c
P4_Grammar* grammar = P4_CreateGrammar();
```

Let's create our very first rule by calling `P4_CreateLiteral`.
The literal rule can match "Hello World" only, as the parameter `true`
indicates the rule being case sensitive.

```c
P4_Expression* rule = P4_CreateLiteral("Hello World", false);
```

Once done, associate the rule with an integer ID.

```c
# define HW_ID 1

P4_AddGrammarRule(grammar, HW_ID, rule);
```

Next, you need to wrap up the source string in to a data structure `P4_Source`.
When creating the `P4_Source` object, you also specify the rule ID.
This will decides which rule to apply to the source when parsing.

```c
P4_Source*  source = P4_CreateSource("Hello World", HW_ID);
```

Now the stage is setup. All you need to do is to parse it and check the result.

```c
if (P4_Parse(grammar, source) != P4_Ok) {
    printf("Error: Parse failed.\n");
    exit(1);
}
```

Function `P4_GetSourceAst` gives you the root node of the AST.
You may traverse the AST and do your homework now.

```c
P4_Token* root = P4_GetSourceAst(source);
// traverse: root->next, root->head, root->tail
```

Once done, don't forget clean things up.
`P4_DeleteSource` deletes the source along with all the ast tokens.
`P4_DeleteGrammar` deletes the grammar along with all the rule expressions.

```c
P4_DeleteSource(source);
P4_DeleteGrammar(grammar);
```

## Example

A more complete code for the [Basic Usage](#basic-usage) is like below.

```c
#include <stdio.h>
#include "peppapeg.h"

# define HW_ID 1

int main(int argc, char* argv[]) {
    P4_Grammar* grammar = P4_CreateGrammar();
    if (grammar == NULL) {
        printf("Error: CreateGrammar: MemoryError.\n");
        return 1;
    }

    P4_Expression* rule = P4_CreateLiteral("HelloWorld", false);
    if (rule == NULL) {
        printf("Error: CreateLiteral: MemoryError.\n");
        return 1;
    }

    if (P4_AddGrammarRule(grammar, SIMPLE_LITERAL, rule) != P4_Ok) {
        printf("Error: AddGrammarRule: MemoryError.\n");
        return 1;
    }

    P4_Source*  source = P4_CreateSource("HelloWorld", HW_ID);
    if (source == NULL) {
        printf("Error: CreateSource: MemoryError.\n");
        return 1;
    }

    if (P4_Parse(grammar, source) != P4_Ok) {
        printf("Error: Parse: ErrCode[%lu] Message[%s]\n",
            P4_GetError(source),
            P4_GetErrorMessage(source)
        );
        return 1;
    }

    P4_Token*   root = P4_GetSourceAst(source);
    printf("root span: [%lu %lu]\n", root->slice.i, root->slice.j);

    P4_DeleteSource(source);
    P4_DeleteGrammar(grammar);
    return 1;
}
```

# Peppy Packing Peppa PEG!

Made with ❤️  by [Ju Lin](https://github.com/soasme).

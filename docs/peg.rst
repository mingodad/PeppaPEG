.. _peg:

Peppa PEG Specification
========================

Objectives
----------

Peppa PEG aims to be a PEG dialect that's easy to use. Peppa PEG is designed to describe a formal language by extending the original version of PEG with some already-commonly-used symbols and notations. Peppa PEG should be easy to parse source code into an abstract syntax tree for a wide variety of languages.

Spec
-----

* Peppa PEG is case-sensitive.
* A Peppa PEG grammar must be a valid UTF-8 encoded Unicode document.

Comment
-------

A hash symbol following by any characters til end of line marks a comment.

.. code-block::

    comment = "#" (!"\n" .)* "\n"?;

Comments are ignored and can be marked between rules.
This is a simple way of including additional information in parallel with the specifications.

For example,

.. code-block::

    # This is a full-line comment.

    rule = "# This is not a comment."; # This is a comment at the end of a line.

Rule Naming
------------

The name of a rule is a sequence of characters, beginning with an alphabet or underscore, and followed by a combination of alphabets, digits, or underscores (_).

.. code-block::

    name = ([a-z]/[A-Z]/"_") ([a-z]/[A-Z]/[0-9]/"_")*;

Rule names are case sensitive.

For example, the rule names listed below are valid:

* `rule`
* `DIGITS`
* `oct_int`

while these are invalid:

* `0to9`
* `ml-string`

Rule Form
---------

The primary building block of a Peppa PEG document is the rule.

A rule is defined by the following sequence:

.. code-block::

    rule = decorator* name "=" expression ";"

where one or more rule decorators may be prefixed before rule names, rule names are on the left-hand side of the equals sign and expressions are one the right-hand side. Rules always ends up with a semicolon (;). The key, equal sign, expressions, and semicolon can be broken over multiple lines. Whitespace is ignored around rule names and between expressions.

For example:

.. code-block::

    rule = rule2 / "expr4" / "expr5" / "expr6";

    @lifted
    rule2 = "expr1"
          / "expr2"
          / "expr3"
          ;

Unspecified expressions are invalid.

.. code-block::

    rule = ; # INVALID!

Expression Precedence
---------------------

Rule expressions have the following precedence from highest to lowest:

* Choice
* Sequence
* Repeat
* Primary

Since choice has the highest precedence, a rule expression always starts interpreting as a choice of one alternatives:

.. code-block::

    expression = choice;

Primary rule expressions must have one of the following types.

* Literal
* Insensitive
* Range
* Reference
* Back Reference
* Positive
* Negative
* Dot
* Cut
* Grouping

.. code-block::

    primary = literal
            / insensitive
            / range
            / (reference !"=")
            / back_reference
            / positive
            / negative
            / dot
            / cut
            / "(" choice ")"
            ;

Grouping notion () is strongly advised which will avoid misinterpretation by casual readers. For example,

.. code-block::

    foobar = (foo  / bar) / (goo / par);

Literal
-------

The literal matches an exact same string surrounded by double quotes.

For example,

.. code-block::

    greeting = "hello world";

Unicode is supported:

.. code-block::

    greeting = "你好，世界";

Emoji can be encoded via Unicode so it is supported:

.. code-block::

    greeting = "Peppa 🐷";

You can encode ASCII characters via `\\x` followed by 2 hex digits.

.. code-block::

    greeting = "\x48\x65\x6c\x6c\x6f, world";

You can encode Unicode characters via `\\u` followed by 4 hex digits or `\\U` followed by 8 hex digits.
The escape codes must be valid Unicode `scalar values <https://unicode.org/glossary/#unicode_scalar_value>`_.

.. code-block::

    greeting = "\u4f60\u597D, world\U0000000c";

Range
------

Range **matches a single character in range**.

In this example, any character between `'0'` to `'9'` can match.

.. code-block::

    digits = [0-9];

The lower and upper character of the range can be not only ASCII characters but also UTF-8 code points.
The syntax can be `\\uXXXX` or `\\uXXXXXXXX`.

.. code-block::

    digits = [\u4e00-\u9fff];

A small trick to match any character is to specify the range from `\\u0001` to `\\U0010ffff`,
which are the minimum and the maximum code point in UTF-8 encoding.

.. code-block::

    any = [\u0001-\U0010ffff];

The value of lower must be less or equal than the upper.

.. code-block::

    // INVALID
    any = [\U0010ffff-\u0001];

Range supports an optional `stride` to skip certain amount of characters in the range.
In this example, only odd number between `'0'` to `'9'` can match.

.. code-block::

    digits = [0-9..2];

Range also supports certain unicode character sets,  such as `C`, `Cc`, `Cf`, `Co`, `Cs`,
`Ll`, `Lm`, `Lo`, `Lt`, `Lu`, `L`, `Nd`, `Nl`, `No`, `N`, etc.
When the library is built with `-DENABLE_UNISTR=On`, it can support even more unicode general categories and unicode properties, such as `Id_Start`, `Id_Continue`, `Other_Id_Start`, `Other_Id_Continue`, `White space`, etc.
They're wrapped via `\\p{}`, for example:

* unicode_letter: a Unicode code point classified as "Letter" (Ll+Lm+Lo+Lt+Lu).
* unicode_digit: a Unicode code point classified as "Number, decimal digit"(Nd).

.. code-block::

    unicode_letter = [\p{L}];
    unicode_digit  = [\p{Nd}];

Dot
---

Single dot `.` can match any UTF-8 code point. It's a syntax sugar for `[\\u0001-\\U0010ffff]`.

.. code-block::

    any = .;

Sequence
--------

Sequence **matches a sequence of sub-expressions in order**.

When parsing, the first sequence member is attempted. If succeeds, the second is attempted, so on and on.
If any one of the attempts fails, the match fails.

For example:

.. code-block::

    greeter = "Hello" " " "world";


Choice
-------

Choice **matches one of the sub-expression.**

When parsing, the first sequence member is attempted. If fails, the second is attempted, so on and on.
If any one of the attempts succeeds, the match succeeds. If all attempts fail, the match fails.

For example:

.. code-block::

   greeter = "Hello World" / "你好，世界" / "Kia Ora";

Reference
---------

Reference **matches a string based on the referenced grammar rule**.

For example, `greeter` is just a reference rule in `greeting`. When matching `greeting`, it will use the referenced grammar rule `greeter` first, e.g. `"Hello" / "你好"`, then match " world".

.. code-block::

    greeting = greeter " world";
    greeter  = "Hello" / "你好";

The order of defining a rule does not matter.

.. code-block::

    greeter  = "Hello" / "你好";
    greeting = greeter " world";

One should ensure all references must have corresponding rule defined, otherwise, the parse will fail with :c:enum:`P4_MatchError`.

Back Reference
--------------

Back Reference **matches an exact same string as previously matched in the sequence**.

For example, \\0 matches whatever `quote` has matched, thus `"abc"` or `'abc'` are valid. But `"abc'` or `'abc"` are invalid.

.. code-block::

    str = quote chars \0;
    quote = "\"" / "'";
    chars = [a-z]*;

Back Reference starts with a back slash, followed by a number. The number is zero-based and cannot be a number greater than or equal to the index of itself.


Insensitive
-----------

Insensitive operator starts with "i" and followed by a literal or back reference.

.. code-block::

    insensitive = "i" (literal / back_reference);

For example,

Given the following rule, back reference \\0 is case-insensitive. Hence, both `a=A` and `a=a` are valid.

.. code-block::

    rule = [a-z] "=" i\0;

Given the following rule, literal "hello world" is case-insensitive. Hence, both `ì` and `Ì` are valid.

.. code-block::

    rule = i"ì";

Positive
--------

Positive **tests if the sub-expression matches**.

Positive attempts to match the sub-expression. If succeeds, the test passes. Positive does not "consume" any text.

Positive can be useful in limiting the possibilities of the latter member in a Sequence. In this example, the Sequence expression must start with "Hello", e.g. "Hello World", "Hello WORLD", "Hello world", etc, will match but "HELLO WORLD" will not match.

.. code-block::

    greeting = &"Hello" i"hello world";

Negative
--------

Negative **tests if the sub-expression does not match**.

Negative expects the sub-expression doesn't match. If fails, the test passes. Negative does not "consume" any text.

Negative can be useful in limiting the possiblities of the latter member in a Sequence. In this example, the Sequence expression must not start with "Hello", e.g. "HELLO World", "hello WORLD", "hello world", etc, will match but "Hello World" will not match.

.. code-block::

    greeting = !"Hello" i"hello world";

Repetition
----------

Operators `+`, `*`, `?` and `{}` followed by an expression indicates repetition.

The full form of repetition is:

.. code-block::

    repeat = primary (onceormore / zeroormore / zerooronce / repeatexact / repeatminmax / repeatmin / repeatmax)?;

1. Plus (`+`) matches string one or more times.

.. code-block::

    onceormore = "+";

For example,

.. code-block::

    number = [0-9]+;

2. Asterisk (`*`) matches string zero or more times.

.. code-block::

    zeroormore = "*";

For example,

.. code-block::

    number = [0-9] [1-9]*;

3. Question (`?`) matches string one or more times.

.. code-block::

    zerooronce = "?";

For example,

.. code-block::

    number = [0-9] "."?;

4. `{cnt}` matches exactly `cnt` occurrences of an expression, where cnt is a decimal value.

.. code-block::

    repeatexact = "{" number "}";

For example,

.. code-block::

   unicode = "\U" ([0-9] / [a-f] / [A-F]){8};

5. `{min,max}` matches an expression of at least `min` occurrences and at most `max` occurrences, where min and max are decimal values.

.. code-block::

    repeatminmax = "{" number "," number "}";

For example,

.. code-block::

   hex = "\u{" ([0-9] / [a-f] / [A-F]){1,6} "}";

6. `{min,}` matches an expression of at least `min` occurrences, where min is a decimal value.

.. code-block::

    repeatmin = "{" number "," "}";

For example,

.. code-block::

    above_hundred = [1-9] [1-9]{2,};

7. `{,max}` matches an expression of at most `max` occurrences, where max is a decimal value.

.. code-block::

    repeatmax = "{" "," number "}";

For example,

.. code-block::

   below_thousand = [0-9]{,3};

Cut
---

Cut is a decorator written as "@cut". It always succeeds, but cannot be backtracked.
It's used to prevent unwanted backtracking, e.g. to prevent excessive choice options.

Backtracking means if e1 in `rule = e1 / e2;` fails, the parser returns the last position where e1 started, and tries e2.
If there is a `@cut` in e1, any failure after the cutting point will cause rule failed immediately.
Cut ensures the parse sticks to the current rule, even if it fails to parse.
See ideas `1 <http://ceur-ws.org/Vol-1269/paper232.pdf>`_, `2 <https://news.ycombinator.com/item?id=20503245>`_.

For example, considering the grammar below first,

.. code-block::

    value = array / null;
    array = "[" "]";
    null  = "null";

Given input "[", it attempts matching array first. After failed, it will try null next. At last, value match is failed.

Let's add a cut operator:

.. code-block::

    value = array / null;
    array = "[" @cut "]";
    null  = "null";

Given input "[", it attempts matching array first. After failed, value match is failed immediately.

Given input "null", it attempts matching array first. It fails before `@cut` and then failed matching array. Parser then match "null" successfully.

Decorators
----------

Decorators are characters @ followed by some selected keywords.
Valid decorators include: `@spaced`, `@squashed`, `@scoped`, `@tight`, `@lifted` and `@nonterminal`.

.. code-block::

    decorator = "@" ("squashed" / "scoped" / "spaced" / "lifted" / "nonterminal");

For example,

.. code-block::

    @spaced @lifted
    ws = " " / "\t" / "\n";

:seealso: :c:enum:`P4_FLAG_SPACED`, :c:enum:`P4_FLAG_SQUASHED`, :c:enum:`P4_FLAG_SCOPED`, :c:enum:`P4_FLAG_TIGHT`, :c:enum:`P4_FLAG_LIFTED`, :c:enum:`P4_FLAG_NON_TERMINAL`.

@spaced
```````

If a rule has `@spaced` decorator, it will be auto-inserted in between every element of sequences and repetitions.

For example, my sequence can match "helloworld", "hello world", "hello  \t  \n world", etc.

.. code-block::

    my_sequence = "hello" "world";

    @spaced
    ws = " " / "\t" / "\n";

@tight
```````

If a sequence or repetition rule has `@tight` decorator, no `@spaced` rules will be applied.

For example, my_another_sequence can only match "helloworld".

.. code-block::

    my_another_sequence = "hello" "world";

    @spaced
    ws = " " / "\t" / "\n";

@lifted
```````

If a rule has `@lifted` decorator, its children will replace the parent.

In this example, the parsed tree has no node mapping to primary rule, but rather either digit or char.

.. code-block::

    @lifted
    primary = digit / char;

    number = [0-9];
    char   = [a-z] / [A-Z];

@nonterminal
````````````

If a rule has `nonterminal` decorator, and it has only one single child node, the child node will replace the parent node.

If it produces multiple children, this decorator has no effect.

In this example,

.. code-block::

    @lifted
    add = number ("+" number)?;

    number = [0-9];

If we feed the input "1", the tree is like:

.. code-block::

    Number(0,1)

If we feed the input "1+1", the tree is like:

.. code-block::

    Add(0,3)
        Number(0,1)
        Number(1,3)

@squashed
`````````

If a rule has `@squashed` decorator, its children will be trimmed.

In this example, the rule `float` will drop all `number` nodes, leaving only one single node in the ast.

.. code-block::

    @squashed
    float = number ("." number)?;

    number = [0-9];

@scoped
```````

Ignore all the decorators set by upstream rules.

For example, despite `greeting2` set to not using spaced rule `ws`, `greeting` can still apply to `ws` since it's under its own scope.

.. code-block::

    @tight
    greeting2 = greeting greeting;

    @scoped
    greeting = "hello" "world";

    @spaced
    ws = " ";

Cheatsheet
----------

.. list-table:: Cheatsheet
   :header-rows: 1

   * - Syntax
     - Meaning
   * - `foo = ...;`
     - grammar rule
   * - `@lifted foo = ...;`
     - drop node
   * - `@spaced foo = ...;`
     - mark as space
   * - `@squashed foo = ...;`
     - ignore children
   * - `@tight foo = ...;`
     - ignore spaced rules
   * - `@non_terminal foo = ...;`
     - ignore single child node
   * - `@scoped foo = ...;`
     - cancle effects
   * - `"literal"`
     - exact match
   * - `"\x0d\x0a"`
     - exact match by using ascii digits
   * - `"\u4f60\u597D"`
     - exact match utf-8 characters
   * - `i"literal"`
     - case-insensitive match
   * - `[a-z]`
     - range
   * - `[0-9..2]`
     - range with stride
   * - `[\\u0001-\\U0010ffff]`
     - range using unicode runes
   * - `[\\p{L}]`
     - range using unicode categories
   * - `.`
     - any character
   * - `foo bar`
     - sequence
   * - `foo / bar`
     - choice
   * - `\0`
     - back reference
   * - `&foo`
     - positive
   * - `!foo`
     - negative
   * - `@cut`
     - prevent unwanted backtracking
   * - `foo*`
     - zero or more
   * - `foo+`
     - once or more
   * - `foo?`
     - optional
   * - `foo{m,}`
     - repeat at least m times
   * - `foo{,n}`
     - repeat at most n times
   * - `foo{m,n}`
     - repeat between m-n times
   * - `foo{m}`
     - repeat exact n times
   * - `# IGNORE`
     - comment

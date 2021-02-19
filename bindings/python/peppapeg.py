from _peppapeg import lib, ffi

#: The version of python-peppapeg.
__VERSION__ = '0.1.0'

#: The version of peppapeg clib.
__CLIB_VERSION__ = ffi.string(lib.P4_Version()).decode('utf-8')

## Low Level Interfaces


### Utils

def _to_string(buffer):
    if buffer == ffi.NULL or buffer.size == 0:
        return ''

    s = ffi.string(buffer.data, buffer.size)
    return s.decode('utf-8', 'ignore')

### Errors

class P4Error(Exception):
    pass

class MatchError(P4Error):
    pass


def raise_error(source=None, err=0, errmsg=""):
    if source:
        err = lib.P4_GetError(source)
        errmsg = to_string(lib.P4_GetErrorMessage(source))

    if err == lib.P4_MatchError:
        raise MatchError()

    elif err == lib.P4_MemoryError:
        raise MemoryError()

    else:
        raise P4Error(f'err code={err}')

### Expression

class Expression:
    """Base class for Expression."""

    def __del__(self):
        if self._expr:
            lib.P4_DeleteExpression(self._expr)

class Literal(Expression):
    """Literal Expression."""

    def __init__(self, literal):
        self.literal = literal
        self._expr = lib.P4_CreateLiteral(
            ffi.new('char[]', literal.encode('utf-8')),
            True,
        )

### Token

class Token:

    def __init__(self, slice, expr, children):
        self.slice = slice
        self.expr = expr
        self.children = children

    def __repr__(self):
        return f'Token({self.slice.start}..{self.slice.stop})'

    @classmethod
    def clone(cls, ctoken):
        i = ctoken.slice.i
        j = ctoken.slice.j
        _slice = slice(i, j)

        _expr = ctoken.expr

        _children = []
        tmp = ctoken.head
        while tmp != ffi.NULL:
            _children.append(cls.clone(tmp))
            tmp = tmp.next

        return cls(_slice, _expr, _children)

### Grammar

def create_expr(f, *args, **kwargs):
    """Create an expression using """
    expr = f(*args, **kwargs)

    if expr == ffi.NULL:
        raise MemoryError('Failed to create expression.')

    return expr

def add_grammar_rule(grammar, id, expr):
    err = lib.P4_AddGrammarRule(grammar, int(id), expr)

    if err != lib.P4_Ok:
        raise_error(err=err, errmsg=f'Failed to add grammar rule for {id}')

def create_literal(literal, sensitive=True):
    return create_expr(
        lib.P4_CreateLiteral,
        literal.encode('utf-8'),
        bool(sensitive)
    )

def create_range(lower, upper):
    return create_expr(
        lib.P4_CreateRange,
        ord(lower),
        ord(upper)
    )

def set_members(expr, *members):
    for index, member in enumerate(members):
        err = lib.P4_SetMember(expr, index, member)
        if err != lib.P4_Ok:
            raise_error(err=err, errmsg="Failed to add sequence member.")

def create_sequence(*members):
    expr = create_expr(
        lib.P4_CreateSequence,
        len(members)
    )
    set_members(expr, *members)
    return expr

def create_choice(*members):
    expr = create_expr(
        lib.P4_CreateChoice,
        len(members)
    )
    set_members(expr, *members)
    return expr

def create_reference(id):
    return create_expr(
        lib.P4_CreateReference,
        int(id)
    )

class Grammar:

    def __init__(self):
        self.names = {}
        self._grammar = lib.P4_CreateGrammar()

    def __del__(self):
        lib.P4_DeleteGrammar(self._grammar)

    def add_literal(self, id, literal, sensitive=True):
        add_grammar_rule(
            self._grammar,
            int(id),
            self.literal(literal, sensitive)
        )

    def literal(self, literal, sensitive=True):
        return create_literal(literal, sensitive)

    def add_range(self, id, lower, upper):
        add_grammar_rule(
            self._grammar,
            int(id),
            self.range(lower, upper)
        )

    def range(self, lower, upper):
        return create_range(lower, upper)

    def add_sequence(self, id, *members):
        add_grammar_rule(
            self._grammar,
            int(id),
            self.sequence(*members)
        )

    def sequence(self, *members):
        return create_sequence(*members)

    def add_choice(self, id, *members):
        add_grammar_rule(
            self._grammar,
            int(id),
            self.choice(*members)
        )

    def choice(self, *members):
        return create_choice(*members)

    def add_reference(self, id, ref_id):
        add_grammar_rule(
            self._grammar,
            int(id),
            self.reference(ref_id)
        )

    def reference(self, id):
        return create_reference(id)

    def parse(self, input, entry=None):
        _id = int(entry or 1)
        source = lib.P4_CreateSource(input.encode('utf-8'), _id)

        if source == ffi.NULL:
            raise MemoryError()

        try:

            err = lib.P4_Parse(self._grammar, source)

            if err != lib.P4_Ok:
                raise_error(err=err, errmsg=source.errmsg)

            ast = lib.P4_GetSourceAst(source)

            return (
                None
                if ast == ffi.NULL
                else Token.clone(ast)
            )

        finally:
            lib.P4_DeleteSource(source)

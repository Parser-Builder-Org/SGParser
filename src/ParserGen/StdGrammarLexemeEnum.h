// Filename:  StdGrammarLexemeEnum.h
// Content:   Standard grammar lexemes enumeration
// Provided AS IS under MIT License; see LICENSE file in root folder.

enum StdGrammarLexemeEnum
{
    SGL_TokenError,
    SGL_TokenEOF,

    SGL_bad_char,
    SGL_number_int,
    SGL_string_cons,
    SGL_identifier,

    SGL_macro,

    SGL_expression,
    SGL_ignore,
    SGL_push,
    SGL_pop,
    SGL_goto,

    SGL_prec,
    SGL_left,
    SGL_right,
    SGL_nonassoc,

    SGL_production,
    SGL_shifton,
    SGL_reduceon,
    SGL_reduce,
    SGL_action,
    SGL_error,
    SGL_error_backtrack,

    SGL_comma,
    SGL_semicolon,
    SGL_question,
    SGL_and,
    SGL_or,
    SGL_curlyopen,
    SGL_curlyclose,
    SGL_lparen,
    SGL_rparen,
    SGL_greaterthan,
    SGL_arrow,

    SGL_quote_cons,
};

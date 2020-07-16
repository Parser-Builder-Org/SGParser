//////////////////////////// TermEnum ////////////////////////////

enum class TermEnum
{
    /*0*/ TokenError,
    /*1*/ TokenEOF,
    /*2*/ Number,
    /*3*/ Plus,
    /*4*/ Minus,
    /*5*/ Multiply,
    /*6*/ Divide,
    /*7*/ Power,
    /*8*/ Assign,
    /*9*/ OpeningParenthesis,
    /*10*/ ClosingParenthesis,
    /*11*/ Identifier,
    /*12*/ EndOfLine
};

constexpr char const* const StringifyEnumTermEnum[] =
{
    /*0*/ "TokenError",
    /*1*/ "TokenEOF",
    /*2*/ "Number",
    /*3*/ "Plus",
    /*4*/ "Minus",
    /*5*/ "Multiply",
    /*6*/ "Divide",
    /*7*/ "Power",
    /*8*/ "Assign",
    /*9*/ "OpeningParenthesis",
    /*10*/ "ClosingParenthesis",
    /*11*/ "Identifier",
    /*12*/ "EndOfLine"
};

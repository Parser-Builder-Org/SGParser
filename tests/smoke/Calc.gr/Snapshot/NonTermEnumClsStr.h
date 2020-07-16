//////////////////////////// NonTermEnum ////////////////////////////

enum class NonTermEnum
{
    /*0*/ unit,
    /*1*/ clause,
    /*2*/ expression,
    /*3*/ assignment
};

constexpr char const* const StringifyEnumNonTermEnum[] =
{
    /*0*/ "unit",
    /*1*/ "clause",
    /*2*/ "expression",
    /*3*/ "assignment"
};

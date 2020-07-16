// Filename:  RegExprProdEnum.h
// Content:   Regular expressions production terms enumeration
// Provided AS IS under MIT License; see LICENSE file in root folder.

enum RegExprProductions 
{
    RE_Accept,
    RE_RegExpConcat,
    RE_RegExp,
    RE_AOr,
    RE_A,
    RE_BStar,
    RE_BPlus,
    RE_BQuestion,
    RE_B,
    RE_CParen,
    RE_CChar,
    RE_CDot,
    RE_CGroupSet,
    RE_CNotGroupSet,
    RE_CCharSet,
    RE_GroupSetChar,
    RE_GroupSetCharGroupSet,
    RE_GroupSetCharList,
    RE_GroupSetCharListGroupSet,
    RE_CharSet,
    RE_CharSetChar
};

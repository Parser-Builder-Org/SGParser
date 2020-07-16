bool ParseHandler::Reduce(Parse<StackElement> &parse, unsigned productionID)
{
    switch (productionID)
    {
        // unit -> unit clause 
        case PE_Unit:
            break;

        // unit -> clause 
        case PE_Oneliner:
            break;

        // clause -> expression 
        case PE_ExpressionClause:
            break;

        // clause -> assignment 
        case PE_AssignmentClause:
            break;

        // clause -> 'eol' 
        case PE_EmptyClause:
            break;

        // expression -> 'number' 
        case PE_Number:
            break;

        // expression -> 'identifier' 
        case PE_Identifier:
            break;

        // expression -> '-' expression 
        case PE_Negation:
            break;

        // expression -> '(' expression ')' 
        case PE_Expression:
            break;

        // expression -> expression '+' expression 
        case PE_Addition:
            break;

        // expression -> expression '-' expression 
        case PE_Substruction:
            break;

        // expression -> expression '*' expression 
        case PE_Multiplication:
            break;

        // expression -> expression '/' expression 
        case PE_Division:
            break;

        // expression -> expression '^' expression 
        case PE_Exponentiation:
            break;

        // assignment -> 'identifier' ':=' expression 
        case PE_Assignment:
            break;

        // assignment -> 'identifier' ':=' assignment 
        case PE_Replication:
            break;

    }
    return 1u;
}

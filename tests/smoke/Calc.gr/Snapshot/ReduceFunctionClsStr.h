// This is a generated file.
// Copyright is in `Calc.gr` - see that file for details.

namespace XC
{

bool ParseHandler::Reduce(Parse<StackElement> &parse, unsigned productionID)
{
    switch (static_cast<ProductionEnum>(productionID))
    {
        // unit -> unit clause 
        case ProductionEnum::Unit:
            break;

        // unit -> clause 
        case ProductionEnum::Oneliner:
            break;

        // clause -> expression 
        case ProductionEnum::ExpressionClause:
            break;

        // clause -> assignment 
        case ProductionEnum::AssignmentClause:
            break;

        // clause -> 'eol' 
        case ProductionEnum::EmptyClause:
            break;

        // expression -> 'number' 
        case ProductionEnum::Number:
            break;

        // expression -> 'identifier' 
        case ProductionEnum::Identifier:
            break;

        // expression -> '-' expression 
        case ProductionEnum::Negation:
            break;

        // expression -> '(' expression ')' 
        case ProductionEnum::Expression:
            break;

        // expression -> expression '+' expression 
        case ProductionEnum::Addition:
            break;

        // expression -> expression '-' expression 
        case ProductionEnum::Substruction:
            break;

        // expression -> expression '*' expression 
        case ProductionEnum::Multiplication:
            break;

        // expression -> expression '/' expression 
        case ProductionEnum::Division:
            break;

        // expression -> expression '^' expression 
        case ProductionEnum::Exponentiation:
            break;

        // assignment -> 'identifier' ':=' expression 
        case ProductionEnum::Assignment:
            break;

        // assignment -> 'identifier' ':=' assignment 
        case ProductionEnum::Replication:
            break;

    }
    return true;
}

} // namespace XC

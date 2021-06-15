// Filename:  CalcParser.h
// Content:   Calculator parser definition
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_SAMPLE_CALC_CALC_PARSER_H
#define INC_SGPARSER_SAMPLE_CALC_CALC_PARSER_H

#include "StdStreamAdapter.h"
#include "Parser.h"

#include "DFA.h"
#include "ParseTable.h"
#include "ProdEnum.h"

#include <fstream>
#include <map>
#include <sstream>
#include <vector>
#include <cmath>

namespace Calc 
{

// Parser used to parse text written in calculator grammar
class CalcParser final : public SGParser::ParseHandler<SGParser::ParseStackGenericElement>
{
public:
    using Number      = float;
    using NumberSet   = std::vector<Number>;
    using Identifier  = std::string;
    using VariableMap = std::map<Identifier, Number>;

private:
    DFA         automata;
    ParseTable  table;
    NumberSet   numbers;
    VariableMap variables;

public:
    CalcParser() {
        automata.Create(CalcDFA);
        table.Create(CalcParseTable);
    }

    void Evaluate(std::string const& text) {
        std::stringstream sstream{text};
        Evaluate(sstream);
    }

    void Evaluate(std::istream& stream) {
        StdStreamAdapter                input{stream};
        DFATokenizer<GenericToken>      tokenizer{&automata, &input};
        Parse<ParseStackGenericElement> parser;
        
        if (parser.Create(&table, &tokenizer) == false)
            throw std::runtime_error("failed to create parser");

        numbers.clear();

        parser.DoParse(*this);
    }

    NumberSet const& GetEvaluatedNumbers() const noexcept { return numbers; }

    // Overriden parse handler reduce function
    bool Reduce(Parse<ParseStackGenericElement>& parse, unsigned productionID) override {
        switch (static_cast<ProductionEnum>(productionID)) {
            case ProductionEnum::Number:
                translateNumber(parse[0].Str.data());
                break;
            case ProductionEnum::Addition:
            case ProductionEnum::Substruction:
            case ProductionEnum::Multiplication:
            case ProductionEnum::Division:
            case ProductionEnum::Exponentiation:
                translateBinaryOperation(static_cast<ProductionEnum>(productionID));
                break;
            case ProductionEnum::Negation:
                translateUnaryOperation(static_cast<ProductionEnum>(productionID));
                break;
            case ProductionEnum::Assignment:
            case ProductionEnum::Replication:
                translateAssignment(parse[0].Str.data());
                break;
            case ProductionEnum::Identifier:
                translateIdentifier(parse[0].Str.data());
                break;
            default:
                break;
        }
        return true;
    }

private:
    Number parseNumber(char const* text) {
        std::stringstream stream{text};
        Number            number = 0.0f;
        stream >> number;
        return number;
    }

    void translateNumber(char const* text) { storeNumber(parseNumber(text)); }

    void translateUnaryOperation(ProductionEnum operation) {
        auto const operand = extractNumber();
        auto const result  = evaluate(operation, operand);
        storeNumber(result);
    }

    void translateBinaryOperation(ProductionEnum operation) {
        auto const secondOperand = extractNumber();
        auto const firstOperand  = extractNumber();
        auto const result        = evaluate(operation, firstOperand, secondOperand);
        storeNumber(result);
    }

    void translateAssignment(char const* identifier) { variables[identifier] = extractNumber(); }

    void translateIdentifier(char const* identifier) {
        checkVariable(identifier);
        storeNumber(variables[identifier]);
    }

    void checkVariable(char const* identifier) const {
        if (variables.find(identifier) == variables.end()) {
            using namespace std;
            throw runtime_error(string{"Undefined variable "} + identifier);
        }
    }

    void storeNumber(Number number) { numbers.push_back(number); }

    Number extractNumber() {
        if (numbers.empty())
            return 0.0f;
        else {
            auto const number = numbers.back();
            numbers.pop_back();
            return number;
        }
    }

    Number evaluate(ProductionEnum operation, Number number) const {
        switch (operation) {
            case ProductionEnum::Negation:
                return -number;
            default:
                return 0.0f;
        }
    }

    Number evaluate(ProductionEnum operation, Number first, Number second) const {
        switch (operation) {
            case ProductionEnum::Addition:
                return first + second;
            case ProductionEnum::Substruction:
                return first - second;
            case ProductionEnum::Multiplication:
                return first * second;
            case ProductionEnum::Division:
                return first / second;
            case ProductionEnum::Exponentiation:
                return std::pow(first, second);
            default:
                return 0.0f;
        }
    }
};

} // namespace Calc

#endif // INC_SGPARSER_SAMPLE_CALC_CALC_PARSER_H

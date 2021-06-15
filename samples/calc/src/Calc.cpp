// Filename:  Calc.cpp
// Content:   Calculator sample main file
// Provided AS IS under MIT License; see LICENSE file in root folder.

#include "CalcParser.h"

#include <iostream>
#include <iterator>

int main(int argc, char const* argv[]) {
    using namespace std;
    using namespace Calc;
    try {
        CalcParser parser;
        if (argc > 1) {
            fstream fileStream{argv[1]};
            parser.Evaluate(fileStream);
        } else {
            parser.Evaluate(cin);
        }
        auto const& numbers = parser.GetEvaluatedNumbers();
        copy(numbers.begin(), numbers.end(), ostream_iterator<CalcParser::Number>(cout, " "));
        cout << endl;
        return 0;
    } catch (exception const& failure) {
        cerr << failure.what() << endl;
        return 1;
    }
}

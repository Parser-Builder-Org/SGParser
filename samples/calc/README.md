<!-- 
Filename:  README.md
Content:   Readme for SGParser sample Calc project
Provided AS IS under MIT License; see LICENSE file in root folder.
-->
# Simple calculator

This sample shows how to build a simple calculator by using the parser.
The calculator has a simple grammar that allows us to use four basic math operations plus exponentiation, as shown below. It also supports negative numbers and floating-point numbers.
```c
1 + 2
1/2 + 3/4
1^2 - 3*4
-1.2 + 3
```

As seen from the examples above, the supported math operations have precedence, and the grammar respects it.
It is also possible to use parentheses to group operations.
```c
(1 + 2) * 3
(1 + 2) / (3 + 4)
((1 + 2)*3 - 4)/5
```

To make our sample geekier, we added variable support.
It is possible to store the evaluated result of an expression into a named variable and use it in later expressions.
And you can assign several variables in one expression.
```c
a := 1
b := 2
c := 3
d := b^2 - 4*a*c
a := b := c := 2^32
```

As you noticed, the calculator understands a "program" written as a set of expressions separated by a new line. Each expression gets evaluated and has a result. The result of the whole program is the result of its last expression.

The calculator takes a file specified as its command-line argument, reads its content, evaluates expressions from it, and prints the result out into the standard output stream. When no file is specified, it reads program content from the standard input stream.
```bat
calc program
```
```bat
echo 1 + 2 - 3 | calc
```

## Grammar

The calculator sample grammar is defined in one text file named [Calc.gr].
The alternative grammar for a calculator (based on Russian language) is defined in one text file named [Calc_rus.gr]. This later grammar illustrates the fact that identifier characters can be Unicode.

The grammar file consists of three sections: [expression], [production], and [precedence].

The [expression] section defines a set of terminal tokens that the tokenizer can generate. These are described by regular expressions are given a label.

```
%expression calc

'[0-9]*\.?[0-9]+'   Number, 'number';

'\+'    Plus,       '+';
'\-'    Minus,      '-';
```

Our grammar defines only one type of expression, which can handle numbers, math operation symbols, parentheses, identifiers, and assignment symbols. Each line under the expression statement defines a lexeme. Lexeme consists of a regular expression at the left part, label, and a name that can be used later in grammar productions where a lexeme is expected.

[precedence] is an optional section that determines the order for the operators.

```
%prec // from lowest to highest

%right  ':=' ;
%left   '+', '-' ;
%left   '*', '/' ;
%right  '^' ;
```

Each line under the precedence block contains operator lexemes with equal precedence. It also defines the associativity of the operators. We use the lexeme name defined earlier in the grammar under the expression block to refer the lexeme.

Here, exponentiation has the highest precedence, followed by multiplication and division, and finally the addition and subtraction. Assignment should be performed only after the expression is evaluated, we give the assignment operator the lowest precedence.

The [production] section consists of definitions of syntactic blocks of a program.

```
%production unit

Unit        unit -> unit clause ;
Oneliner    unit -> clause ;

ExpressionClause    clause -> expression ;
AssignmentClause    clause -> assignment ;
EmptyClause         clause -> 'eol' ;

Number          expression -> 'number' ;
Identifier      expression -> 'identifier' ;
```

Here we define a single starting production `unit` and how it is get composed from other productions.

Each production is given a nonterminal identifier name such as a `unit`, `clause`, or `expression` and a label such as a `Number` or `AssignmentClause` on the left-hand side. The label is used by the parser generator to generate case labels that you can switch on within the `Reduce` function when the production is reduced. On the right-hand side, you define the syntactical structure of the production.

All lines with the same nonterminal identifier define a set of possible alternatives for the production. Accordingly, the expression on the right can contain can a number or an lexeme identifiers, terminal labels, or it can just be an empty line.

## Code Structure

The simple calculator sample is consists of the [main program], the [parse handler], and the [stream adapter].

The role of the [main program] is to handle command-line arguments and determine the input source.
As was mentioned earlier, we can specify a file or the standard input stream as the input source. The [stream adapter] is used by the parser for reading input text. It adapts the standard library input stream to the parser interface.

Parsing process requires a parse table, DFA table, and production tokens enumeration used by the [parse handler]. These files get generated from the [Calc.gr] file by the `sgyacc` tool.

The following code snippet shows a set of actions required to initialize the parser and parse a text.

```c++
std::string text = // ... Text to parse

DFA automata;
automata.Create(StaticDFA)

ParseTable table;
table.Create(StaticParseTable)

MemBufferInputStream            input{text};
DFATokenizer<GenericToken>      tokenizer{&automata, &input};
Parse<ParseStackGenericElement> parser;
ParseHandler                    handler;
parser.DoParse(handler);
```

The `StaticDFA` and `StaticParseTable` are defined in files generated by the parser `sgyacc` executable.
The `ParseHandler` is the class implements `SGParser::ParseHandler<SGParser::ParseStackGenericElement>` and overrides its `Reduce` function.

```c++
bool ParseHandler::Reduce(Parse<ParseStackGenericElement>& parse, unsigned productionID) final {
    switch (static_cast<ProductionEnum>(productionID)) {
        // ...
    }
    return true;
}
```

## Build

To build the sample we have to have the *parser library* and the *sgyacc* program built.

We also need the parse table, DFA table and production enumerations generated. This is done by running the [generate] script (or [generate_rus] for alternative grammar). The *generate* script runs the *sgyacc* program with the following command-line arguments.
```bat
sgyacc Calc.gr --lalr -enumclasses -enumstrings -dfa +classname:"CalcDFA" -prodenum -pt +classname:"CalcParseTable"
```
Here `-lalr` refers to the fact that we use Look-Ahead Left-to-Right parse tables; `-dfa` requests DFA tables to be generated, `-prodenum` stands for production tokens enumeration generation, and `-pt` instructs the *sgyacc* to generate the parse table.

All these files get stored in a folder named *generated*. After generating the tables, you can build the sample.

If you change the grammar you have to re-generate the files after you made the changes.
It can be done by simply running the `generate` script from `samples/calc` directory.
By default it uses `x64 debug` version of the `sgyacc` executable.
You can specify different architecture for the generate script.
```
generate --arch x64
generate --arch win32
```
It's also possible to choose configuration.
```
generate --config debug
generate --config release
```

If you change the grammar you have to re-generate the files after you made the changes.
It can be done by simply running the `generate` script from `samples/calc` directory.
By default it uses `x64 debug` version of the `sgyacc` executable.
You can specify different architecture for the generate script.
```
generate --arch x64
generate --arch win32
```
It's also possible to choose configuration.
```
generate --config debug
generate --config release
```

## Principle

The calculator uses the parser set to use the generated parse table and deterministic finite automata.
The parser translates an input file written in the [grammar] into a parse tree.
Then it calls [reduce function] defined by the calculator [parse handler] while visiting the tree.

The parser calls the [reduce function] for each production token.
It does that in the order when operands go before an operation that is performed on them.

[Back to samples](../#samples)

[Back to parser](../../../../#simple-grammar-parser)

[Calc.gr]:         src/Calc.gr
[Calc_rus.gr]:     src/Calc_rus.gr
[expression]:      src/Calc.gr#L6
[generate]:        generate.bat
[generate_rus]:    generate_rus.bat
[grammar]:         src/Calc.gr
[main program]:    src/Calc.cpp
[parse handler]:   src/CalcParser.hpp
[precedence]:      src/Calc.gr#L26
[production]:      src/Calc.gr#L33
[reduce function]: src/CalcParser.hpp#L65
[stream adapter]:  src/StdStreamAdapter.hpp

<!-- 
Filename:  SGYacc_Grammar_Notes.md
Content:   SGYacc Grammar Notes document
Provided AS IS under MIT License; see LICENSE file in root folder.
-->
# SGYacc Grammar Notes


## `%action` and `%cleanup` keywords

If an action needs to be executed in between token shifts, this can be achieved with the use of `%action` keyword, as follows:
```
Label  Declarator  ->  'id' %action(OpenNamespace) '{'
```
This notation is a shortcut for defining an OpenNamespace nonterminal and a production deriving an empty entry. For this reason, `%action` entry is treated as nonterminal for all parsing purposes. The parser will implement the above statement as follows:
```
Label            Declarator       ->  'id' [OpenNamespace] '{';
[OpenNamespace]  [OpenNamespace]  ->  ;
```
When the action is executed, the stack position will point to its location in production. For the above case, `'id'` can be accessed with index -1, as in `parse[-1]`. Because actions add new productions to the grammar, they can generate conflicts just as other productions would.

Cleanup actions can be added to productions, they will be executed during error recovery before the symbol following the `%cleanup` statement is flushed from the stack. Cleanup actions will only be executed if their corresponding symbol was ever shifted on the stack. They do not get executed when enclosing production is reduced.
```
Label  Declarator  ->  'id' %cleanup(CloseNamespace) %action(OpenNamespace) '{';
```
When cleanup action is executed, stack points to the symbol right after the `%cleanup` action; which is `[OpenNamespace]` symbol in the above example. Note that if `%cleanup` was to follow the action, then the cleanup action would've referred to `'}'` terminal, not `[OpenNamespace]`. Cleanup actions do not add any productions to the grammar.

Note: `%cleanup()` does not seem to be implemented.


## `'|'` or operator in productions

The vertical bar `'|'` operator can be used in productions to allow for either one of several symbols to be matched. Operand on either side should be a single symbol. Any number of consecutive `'|'` operators can be combined to give more than two options, as follows:
```
Label   Specifier   ->  'register' | 'auto' | 'mutable';
```
or
```
Label2  Specifier2  ->  'register' | 'auto' ';';
```
The first production consists of a single symbol, which is either one of `'register'`, `'auto'`, or `'mutable'`. The second production derives `'register'` or `'auto'`, followed by semicolon `';'`.
` '|'` operator binds to only one symbol at a time on each side (one can say it has higher precedence than concatenation, defined by white space).

The `'|'` operator is implemented by defining additional productions that correspond to the same label. The second statement will in effect be expanded to:
```
Label2  Specifier2  ->  'register' ';';
Label2  Specifier2  ->  'auto' ';';
```
Note that both productions will be reported as the same reduction, so it is up to the user code to figure out which production it was.


## Multiple starting productions

SGYacc grammar can have any number of starting productions. Starting productions are declared after the `%production` keyword. They can appear on separate lines in different `%production` statements or can be separated by commas within the same statement. The first starting production defined in the file is always the default one.
```
%production DefaultStart, Start1, Start2
```
or
```
%production DefaultStart
%production Start1, Start2
```
Productions defined in the grammar file will be saved in the ProductionEnum, entries of which can then be passed to the `parse::SetStartingProduction(int)` function to set the starting production. This function will change the starting state to the state that corresponds to the requested production, provided that the stack size is 1 (no data shifted on the stack, as at the beginning of parse).


## User-Code controlled reductions

A production can derive one of several options on the left-hand side, as follows:
```
Label  TypeName      |
       NamespaceName |
       Id_Expression   ->  Identifier;
```
This syntax defines a user-controlled reduction. For all purposes, the parser will treat the above production as
```
Label  TypeName        ->  Identifier;
```
However, it will define two extra nonterminals, `NamespaceName` and `Id_Expression`. Whenever the production is reduced, the code that is associated with the `Label` is free to substitute `NamespaceName` or `Id_Expression` instead of `TypeName` for the resulting nonterminal symbol. The actions that a parser takes from that point on will then depend on what nonterminal was picked as a result of the reduction. The nonterminal the parser will pick can be set with `Parse::SetReduceNonterminal()` function, with the default value being the first one declared.

This allows for a clean implementation of many programming language constructs. In particular, an identifier can be looked up in the symbol table to determine which parsing action to take based on its type.


## User-controlled error recovery

The nonterminal on the left-hand-side can be defined to generate a 'named' error (the name of an error corresponds to the name of the nonterminal), as follows:
```
ErrLabel  NonTerminal %error(NamedError)  ->  Identifier;
```
Whenever a reduction by the `ErrLabel` as defined above occurs, an error will be generated (instead of the nonterminal).  The parser will then skip tokens as expected, and unwind the stack until it finds a named error terminal declared as follows:
```
ProdLabel  MyProd  ->  %error(NamedError) ';';
```
Then the named error token will be shifted onto the stack. Note that user-defined errors can not be matched by standard `%error` terminal, they can only be matched by the named `%error()` constructs with the same name.

Error handling has a special token recording scheme associated with it. Whenever an `%error` token, either named or unnamed, is encountered as a possible lookahead in a certain state, token recording starts. Once that state is reduced, token recording ends, and the recorded tokens are forgotten. If an error occurs, however, the recorded token stream is passed to the error handler and can be used for further processing.

User-defined error productions can be used together with user-controlled reductions, as follows:
```
Label  TypeName      |
       NamespaceName |
       Id_Expression |
       ThrowUnknownId %error(UnknownId)  ->  Identifier;
```
In this case, if the user code decides to reduce by `UnknownId`, a named error will be generated. This error will then be matched with the corresponding named `%error` token down the stack. A user then has an option of saving the corresponding token stream and re-parsing it when the identifier declaration is found, for example.


## Recovering from 'ambiguous' situations with `%error(%try)`

When more then one parse is valid, several parses can be tried:
```
Label  Entry  ->  Declaration;
Label  Entry  ->  %error(%try Statement);
```
Here, `Declaration` will be parsed first. If that fails with an unnamed error, `Statement` is parsed with the same token stream (which is automatically recorded by the parser when needed).

The `%try` construct takes a nonterminal as a parameter. The nonterminal will be tried if an error occurs (this will correspond to a different state). In the above example, `Statement` should have an internal `%error` handler to prevent a possibility of infinite retry loops.




Tokenizer has to convert identifiers to their corresponding tokens (such as `'type_name'`) whenever it encounters the identifier it knows.


## `%shifton(statement)`/`%reduceon(statement)` – Shift/reduce conflict resolution

The `%shifton` and `%reduceon` statements can appear before a terminal on the right-hand side of the production to quiet and resolve shift/reduce conflicts.  As an example, if/else statement can be resolved to a shift this way:
```
PvmStatementIf      statement  ->  'if' '(' expression ')' statement;
PvmStatementIfElse  statement  ->  'if' '(' expression ')' statement                              
                                   %shifton(statement) 'else' statement;
```

Here, we would get a shift/reduce warning as follows unless we had a `%shifton()` before `'else'`:
```
Warning : YC0010W - (Ln:610) State 371: S/R conflict on 'else', between statement/statement
```
Note that such conflicts usually default to shift, but this allows us to quiet the warning and instruct what action to take.



## Defining scopes & their entries

Because of nested scopes, identifiers have to be entered into the scope as soon as possible to be looked up. For this reason, inherited attributes have to be used to open up scopes.

So, 
```
'namespace' 'identifier' OpenNamespace '{'
```
syntax will be used. Then all identifiers declared from that point on will go into the 'current namespace'. Cleanup action (on an error) has to make sure to close the corresponding scope.

Every scope has a set of declarations that can appear in it; these declarations get added directly to that scope. In the case of an unknown identifier, however, the parse may not be able to continue.

This will be recorded as an 'unknown' entry in the given scope, with following associated attributes:
-      token stream (tokens this entry contained, that's were skipped)
-      dependency (an identifier that caused a parse to fail, can be nested in another scope)
-      a production that will have to be applied to parse it

So, as parse proceeds, the unknown entries are entered into the scope tree just as normal entries would be. Note that dependency can be located in a few different places, (such as a type in a declaration or a declarator).

Once a dependency declaration is found, a dependency can be resolved. This is done by re-parsing the 'unknown' entry's stream of tokens; starting with a given production. For this to work, the parser has to support multiple starting points – a starting point will be declared for any production that may have to be parsed individually.

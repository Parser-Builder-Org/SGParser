<!--
Filename:  README.md
Content:   Readme for parser smoke test runner
Provided AS IS under MIT License; see LICENSE file in root folder.
-->
# Parser Smoke tests

The idea is to run the Parser against supported grammars and check its correctness by comparing parsing results with snapshots.

## Usage

### On Microsoft Windows

To run the tests from the repository root simply run one of the following commands:
```bat
.\tests\smoke\run
```
```bat
.\tests\smoke\run --config release
```
```bat
.\tests\smoke\run --arch x64
```
```bat
.\tests\smoke\run --config release --arch x64
```
Also, compiler can be specified by using `--compiler` option:
```bat
.\tests\smoke\run --arch x64 --config release --compiler clang++
```

No output will be shown in case if the tests are passed.

When the parsing result is different from expected you will see an output like the following:
```
C:\xdev-initial\SGParser\tests\smoke\run.ps1 : Difference found for C.ycc in 
StaticParseTable.h
InputObject                                    SideIndicator
-----------                                    -------------
UInt16 StaticParseTable_ActionTable[517][94] = =>
UInt StaticParseTable_ActionTable[517][94] =   <=
At line:1 char:1
+ C:\xdev-initial\Test\Parser\C\run.p ...
+ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    + CategoryInfo          : NotSpecified: (:) [Write-Error], WriteErrorException
    + FullyQualifiedErrorId : Microsoft.PowerShell.Commands.WriteErrorException,run.ps1
```

When an error is not related to grammar you will see an output like the following:
```
C:\xdev-initial\SGParser\tests\smoke\run.ps1 : Failed to parse Cx.ycc
Error   : FL0001E - Failed to open the user grammar file - 'Cx.ycc' 
1 error(s), 0 warning(s)
At line:1 char:1
+ C:\xdev-initial\Test\Parser\C\run.p ...
+ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    + CategoryInfo          : NotSpecified: (:) [Write-Error], WriteErrorException
    + FullyQualifiedErrorId : Microsoft.PowerShell.Commands.WriteErrorException,run.ps1
```

### On Linux and macOS

To run the tests from the repository root simply run one of the following commands:
```sh
./tests/smoke/run.sh
```
```sh
./tests/smoke/run.sh --config release
```
Also, compiler can be specified by using `--compiler` option:
```sh
./tests/smoke/run.sh --compiler clang++
```

In case if the tests are passed you will see the following:
```
Testing Calc.gr SUCCESS
Testing CmdLineParser.gr SUCCESS
```

When the parsing result is different from expected you will see an output like the following:
```
Testing Calc.gr FAIL
Difference found for Calc.gr in ReduceFunction.h
--- tests/smoke/Calc.gr/Snapshot/ReduceFunction.h
+++ tests/smoke/Calc.gr/Output/ReduceFunction.h
@@ -1,4 +1,4 @@
-UInt    ParseHandler::Reduce(Parse<StackElement> &parse, UInt productionID)
+unsigned    ParseHandler::Reduce(Parse<StackElement> &parse, unsigned productionID)
 {
     switch (productionID)
     {
FAIL
Difference found for Calc.gr in ReduceFunctionClsStr.h
--- tests/smoke/Calc.gr/Snapshot/ReduceFunctionClsStr.h
+++ tests/smoke/Calc.gr/Output/ReduceFunctionClsStr.h
@@ -1,4 +1,4 @@
-UInt    ParseHandler::Reduce(Parse<StackElement> &parse, UInt productionID)
+unsigned    ParseHandler::Reduce(Parse<StackElement> &parse, unsigned productionID)
 {
     switch (static_cast<>(productionID))
     {
```

When there is a problem in the grammar itself you will see something like the following output:
```
Testing Calc.gr FAIL
Error   : YC0031E - (Ln:7, Col:1) Syntax error in expression. Unexpected 'bug'

1 error(s), 0 warning(s)
```

And when the error is not related to a grammar the output will be like the following:
```
Testing Bug.gr FAIL
Error   : FL0001E - Failed to open the user grammar file - 'tests/Bug.gr' 

1 error(s), 0 warning(s)
```

## Structure

There is the main script and a bunch of subdirectories each for particular grammar.

Each grammar directory contains the `Snapshot` directory with generated files which are etalons for the correctness checking.
It also contains the `run.ps1` file which is used by the main script and is not intended for the direct run.

### The list of grammars

The main script uses a list of grammars to be checked. By modifying the list we can run the tests only against particular grammars. The list is in `grammar.lst` file.

### Skip a grammar test

To exclude a particular grammar from the testing you need to modify the grammar list.

You can either delete a line from it or comment a line out by adding the `#` symbol at its beginning.

[Back to tests](../#parser-tests)

[Back to parser](../../../../#simple-grammar-parser)

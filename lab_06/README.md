# Topic: Parser & Building an Abstract Syntax Tree

### Course: Formal Languages & Finite Automata
### Author: Mihail Pancenco

---

## Overview

Parsing is the syntactic analysis stage that comes after lexical analysis. A parser receives a stream of tokens and checks whether they follow the grammar of the language. While doing that, it can also build a hierarchical representation of the program.

An **Abstract Syntax Tree (AST)** stores only the meaningful structure of the text. It ignores details that are useful for reading characters, but not useful for later analysis, such as punctuation that only separates constructs.

---

## Theory

A compiler front-end usually works in two connected stages:

- **Lexer** - converts raw text into tokens.
- **Parser** - converts tokens into a syntactic structure.

In this laboratory the lexer uses **regular expressions** to recognize the token type. This fits the theory because token classes such as identifiers, numbers, strings, keywords, and operators are regular languages.

The parser is implemented as a **recursive-descent parser**. Each parsing function corresponds to one syntactic construct of the DaQuVa language used in laboratory work 3:

- database declarations,
- function declarations,
- assignments,
- return statements,
- output statements,
- merge statements,
- scan/filter/newTable/findLocalDuplicates expressions,
- function calls,
- arrays and member access.

---

## Objectives

1. Get familiar with parsing and how it can be programmed.
2. Get familiar with the concept of an Abstract Syntax Tree.
3. Add a `TokenType` enum for lexical token categories.
4. Use regular expressions to identify token types.
5. Implement AST data structures for the text processed in laboratory work 3.
6. Implement a parser that extracts syntactic information from the input text.

---

## Implementation description

The project is implemented in **C++** and is fully encapsulated in this folder. It does not depend on files from previous laboratories.

### TokenType enum

The token categories are represented with an enum:

```cpp
enum class TokenType {
    My, Db, Function, Return, Where, And, Or,
    Identifier, Number, String,
    Arrow, Assign, Eq, Ne, Lt, Le, Gt, Ge,
    End
};
```

The actual code contains all DaQuVa keywords and punctuation used by the sample program.

### Regex lexer

The lexer stores token rules as pairs of `TokenType` and `std::regex`:

```cpp
struct TokenRule {
    TokenType type;
    std::regex pattern;
};
```

The rules are ordered so that longer operators (`->`, `>=`, `==`) and keywords are recognized before shorter operators and identifiers. Whitespace and `#` comments are skipped, while line and column are preserved for readable parser errors.

### AST data structure

The AST uses a compact tree node:

```cpp
struct AstNode {
    std::string type;
    std::string value;
    std::vector<AstNode> children;
};
```

This is enough to represent statements and expressions without creating unnecessary classes for each node kind.

### Parser

The parser is recursive-descent. The main entry point is:

```cpp
AstNode parseProgram();
```

It repeatedly parses statements until the end of the token stream. Expression parsing handles DaQuVa constructs such as:

- `scan table."field" using tool "tool_name"`
- `filter data where score >= 80`
- `findLocalDuplicates table columns cols using tool toolName`
- `newTable "name" from source addColumns [ ... ]`
- `functionName(arg1, arg2)`

---

## Usage

Build and run from this folder:

```bash
g++ main.cpp -std=c++23 -o main.out
./main.out
```

The default input file is:

```bash
input.dqv
```

A custom input file can also be passed:

```bash
./main.out another_file.dqv
```

---

## Results

The program first prints the token stream and then the AST. A shortened AST excerpt looks like:

```text
`-- Program
    |-- DatabaseDeclaration
    |   `-- StringLiteral: "postgresql://localhost:5432/customers?user=admin&password=secret"
    |-- FunctionDeclaration: validateEmail
    |   |-- Parameters
    |   |   |-- Parameter: tableName
    |   |   `-- Parameter: threshold
    |   `-- Body
    |       |-- Assignment: scanned
    |       |   `-- ScanExpression
    |       |-- Assignment: risky
    |       |   `-- FilterExpression
    |       `-- ReturnStatement
    |           `-- Identifier: risky
```

The AST shows the hierarchy of the input program clearly: declarations contain bodies, assignments contain expressions, filters contain conditions, and function calls contain arguments.

---

## Conclusions

This laboratory extends the lexer from laboratory work 3 into a small compiler front-end. Regular expressions are used for lexical analysis, while the parser extracts the syntactic structure and builds an AST. The final output is readable and can be used as a base for later semantic analysis or interpretation.

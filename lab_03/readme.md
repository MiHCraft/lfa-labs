# Lexer & Scanner Implementation

### Course: Formal Languages & Finite Automata
### Author: Mihail Pancenco  

---

## Overview

The term **lexer** comes from lexical analysis which, in turn, represents the process of extracting lexical tokens from a string of characters. There are several alternative names for the mechanism called lexer, for example tokenizer or scanner. The lexical analysis is one of the first stages used in a compiler/interpreter when dealing with programming, markup or other types of languages.

The tokens are identified based on some rules of the language and the products that the lexer gives are called lexemes. So basically the lexer is a stream of lexemes. Now in case it is not clear what's the difference between lexemes and tokens, there is a big one. The lexeme is just the byproduct of splitting based on delimiters, for example spaces, but the tokens give names or categories to each lexeme. So the tokens don't retain necessarily the actual value of the lexeme, but rather the type of it and maybe some metadata.

---

## Theory

A **lexer** (scanner/tokenizer) is a finite-state machine that converts a raw character stream into a sequence of **tokens**. Because every token pattern in a programming/DSL language is a **regular language**, a lexer can be implemented directly as a **nondeterministic finite automaton (NFA)** — exactly the same model we studied in the previous laboratory (Regular Grammar → Finite Automaton conversion).

The implementation follows two fundamental principles:
- **Maximal munch** (longest-match rule) — the lexer always prefers the longest possible valid token.
- **Priority resolution** — when several tokens end at the same position, the one with the highest priority wins (keywords > identifiers/literals > operators).

This guarantees deterministic, unambiguous tokenisation even for languages with overlapping patterns (e.g. `==` vs `=`, keywords vs identifiers).

---

## Objectives

1. Understand what lexical analysis is.
2. Get familiar with the inner workings of a lexer/scanner/tokenizer.
3. Implement a sample lexer and show how it works.

---

## Implementation description

The project is implemented in **C++** and reuses the `FiniteAutomaton` class from the previous laboratory. The code is structured into logical components that mirror a classic compiler front-end.

### FiniteAutomaton class (reused from previous lab)

The same NFA implementation we built for grammar-to-automaton conversion is now used as the core engine of the lexer.

```cpp
class FiniteAutomaton { /* states, transitions, start, finals */ };
```

### Lexer class

Encapsulates the NFA and performs tokenisation with maximal munch + priority resolution. It also handles whitespace, newlines, and single-line comments (`#`).

```cpp
class Lexer {
public:
    void tokenise(const std::string& input);
private:
    const FiniteAutomaton& fa;
    const std::map<std::string, TokenType>& finalToToken;
};
```

Key features:
- Skips whitespace and `#` comments.
- Uses the classic longest-match algorithm with on-the-fly priority selection.
- Reports line/column information for every token (essential for later parser error messages).

### Token definition & priority

```cpp
enum TokenType { /* 20+ keywords, ID, NUMBER, STRING, operators... */ };

int tokenPriority(TokenType t);   // keywords = 10, literals = 5, operators = 1
std::string tokenTypeName(TokenType t);
```

### Automaton construction – buildDaQuVaAutomaton()

All token patterns are compiled into a single NFA at startup using helper lambdas:

```cpp
std::pair<FiniteAutomaton, std::map<std::string, TokenType>> buildDaQuVaAutomaton();
```

- Keywords (`my`, `db`, `function`, `where`, `allowDanger`, …) → dedicated state chains.
- Identifiers → letter/underscore + alphanum/underscore loop.
- Numbers → digit loop.
- Strings → `"` … `"` (any printable character except quote).
- Single- and multi-character operators (`->`, `==`, `!=`, `<=`, `>=`, etc.).
- Punctuation (`;`, `(`, `)`, `[`, `]`, etc.).

The resulting automaton is exactly the one that would be produced by converting a regular grammar of DaQuVa tokens (as we learned in the previous lab).

---

## Main execution logic

```cpp
int main(int argc, char* argv[]) {
    auto [fa, finalMap] = buildDaQuVaAutomaton();
    Lexer lexer(fa, finalMap);

    // Read from file (recommended) or stdin
    std::string input = (argc > 1) ? readFile(argv[1]) : readStdin();

    lexer.tokenise(input);   // prints every token with line/column
    return 0;
}
```

The program can be run as:
```bash
./lexer input.dqv
```

---

## Conclusions / Results

The lexer was tested on the complete **DaQuVa** example program (`input.dqv`) — a realistic 40+ line DSL script containing:
- database connection strings,
- function definitions with parameters,
- complex `where` conditions,
- `Here(85) allowDanger` safety construct,
- `addColumns` with function calls,
- comments, strings with special characters, multi-character operators, etc.

**Result:** All tokens were recognised correctly. No lexical errors occurred. Comments were ignored, strings (even those containing `?user=admin&password=secret`) were captured as single `TOKEN_STRING` tokens, keywords took precedence over identifiers, and multi-character operators (`->`, `==`, `>=`) were preferred over their single-character parts.

**Sample output excerpt** (first few tokens):

```bash
Token: MY Lexeme: "my" Line: 3 Col: 1
Token: DB Lexeme: "db" Line: 3 Col: 4
Token: STRING Lexeme: "\"postgresql://localhost:5432/customers?user=admin&password=secret\"" Line: 3 Col: 7
Token: SEMICOLON Lexeme: ";" Line: 3 Col: 92
Token: FUNCTION Lexeme: "function" Line: 6 Col: 1
Token: IDENTIFIER Lexeme: "validateEmail" Line: 6 Col: 10
...
Token: EOF Line: 42 Col: 1
```

This laboratory proves once again the **equivalence** between regular languages and finite automata: the same `FiniteAutomaton` class that previously accepted strings generated by a regular grammar now powers a production-grade lexer for a complete domain-specific language.

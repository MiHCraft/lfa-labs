# Topic: Regular expressions

### Course: Formal Languages & Finite Automata
### Author: Mihail Pancenco

---

## Theory

A **regular expression (regex)** is a compact formal language used to describe **regular languages**. It is built from:

* **Symbols** from an alphabet (e.g., `a`, `b`, `0`, `1`).
* **Concatenation** (juxtaposition): `ab` means "a followed by b".
* **Union / OR**: `a|b` means "a or b".
* **Kleene star**: `a*` means "repeat a zero or more times".
* **Plus**: `a+` means "repeat a one or more times".
* **Optional**: `a?` means "a zero or one time".
* **Power**: `a^3` means "a exactly three times".

Every regular expression can be converted to an **NFA**, then to an equivalent **DFA** using subset construction. Regular expressions, finite automata, and **regular grammars (Type-3)** are equivalent descriptions of the same class of languages.

---

## Objectives

1. Explain what regular expressions are and what they are used for.
2. Implement a dynamic generator of valid words from a given set of regexes (no hardcoding), with:
   * `*` and `+` bounded to **at most 5** repetitions.
   * a visible processing sequence of the regex (bonus).
3. Provide a clear report of the implementation and any difficulties encountered.

---

## Implementation description

The project is written in **C++** and follows a pipeline that dynamically interprets each regex and generates all valid words within the repetition limit.

### Core steps

1. **Tokenization** of the regex string.
2. **AST construction** using a recursive-descent parser.
3. **Thompson NFA construction** from the AST.
4. **NFA → DFA** conversion with subset construction.
5. **DFA → Regular Grammar** conversion.
6. **Word generation** from the regular grammar.

### Bounded repetition

* `*` is interpreted as **0..5** repeats.
* `+` is interpreted as **1..5** repeats.
* `^n` remains **exactly n** repeats.

This keeps generation finite while preserving the intended language.

### Bonus: Processing sequence

For each regex, the program prints to the console:

1. The original regex.
2. The token list.
3. The AST.
4. The derived grammar.
5. All generated valid words.

At the same time, **Raylib** is used to visualize the DFA as a **slide-show**:

* First regex DFA is shown.
* Press **SPACE** to move to the next DFA.
* Press **Backspace** to go back.

---

## Main execution flow

1. Read regexes from `input.txt` (one regex per line).
2. For each regex:
   * Print regex, tokens, AST.
   * Build NFA, convert to DFA.
   * Convert DFA to regular grammar.
   * Generate all valid words.
3. Launch Raylib slideshow to visualize each DFA.

---

## Example (bounded repetition)

Input:

```
a*b+
```

Output:

```
b
bb
bbb
bbbb
bbbbb
ab
abb
abbb
abbbb
abbbbb
aab
aabb
aabbb
aabbbb
aabbbbb
aaab
aaabb
aaabbb
aaabbbb
aaabbbbb
aaaab
aaaabb
aaaabbb
aaaabbbb
aaaabbbbb
aaaaab
aaaaabb
aaaaabbb
aaaaabbbb
aaaaabbbbb
```

---

## Build and run

The code is located in:

* `main.cpp`

Typical build command (Raylib required):

```bash
g++ main.cpp -std=c++23 -o main.out -I/opt/homebrew/include -L/opt/homebrew/lib -lraylib
```

Then run:

```bash
./main.out
```




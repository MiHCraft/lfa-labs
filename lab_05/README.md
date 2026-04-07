# Topic: Chomsky Normal Form

### Course: Formal Languages & Finite Automata
### Author: Mihail Pancenco

---

## Theory

A **context-free grammar (CFG)** is defined as:

G = (Vn, Vt, P, S),

where:
- Vn is the set of non-terminal symbols,
- Vt is the set of terminal symbols,
- P is the set of production rules,
- S is the start symbol.

A grammar is in **Chomsky Normal Form (CNF)** if every production is one of the following:

- A -> BC (two non-terminals)
- A -> a  (one terminal)
- S -> eps (only if the language includes epsilon)

CNF is useful because it simplifies many algorithms on CFGs (parsing, CYK, proofs) by limiting the shape of productions.

---

## Objectives

1. Learn about Chomsky Normal Form.
2. Implement a step-by-step normalization of a grammar:
   - eliminate epsilon productions,
   - eliminate unit (renaming) productions,
   - eliminate inaccessible symbols,
   - eliminate non-productive symbols,
   - convert to CNF.
3. Make the implementation reusable for any grammar (bonus).

---

## Implementation description

The project is implemented in **C++** and centered around a simple `Grammar` + `Production` model. The program reads a grammar, applies each transformation in order, and prints the result after every step.

### Core data model

```cpp
struct Production {
    std::string lhs;
    std::vector<std::string> rhs; // empty = epsilon
};

struct Grammar {
    std::string start;
    std::vector<Production> productions;
};
```

### Normalization pipeline

1. **Epsilon elimination**
   - Compute nullable non-terminals.
   - Generate all variants of productions by removing nullable symbols.

2. **Unit production elimination**
   - Compute unit-closure for each non-terminal.
   - Replace unit productions with the actual right-hand sides.

3. **Inaccessible symbol removal**
   - BFS from the start symbol to keep only reachable non-terminals.

4. **Non-productive symbol removal**
   - Mark non-terminals that can derive terminals.
   - Remove all other symbols and rules.

5. **CNF conversion**
   - Replace terminals in long rules with new non-terminals (`T_a -> a`).
   - Binarize long productions into chains of length 2.
   - Keep `S -> eps` only if the language contains epsilon.

---

## Usage

The default input file is `input.txt` (in this folder). It uses the format:

```
A -> aB | bA | epsilon
```

Run with:

```bash
g++ main.cpp -std=c++23 -o main.out
./main.out
```

You can also pass a custom file:

```bash
./main.out my_grammar.txt
```

---

## Variant 20 (used in input.txt)

```
S -> aB | bA | A
A -> B | Sa | bBA | b
B -> b | bS | aD | epsilon
D -> AA
C -> Ba
```

---

## Results

The program prints the grammar after every step and finishes with the final **Chomsky Normal Form**. This makes it easy to verify each transformation and see exactly how the grammar is normalized.


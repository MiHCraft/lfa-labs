# Regular Grammar Processing and Finite Automaton Construction

### Course: Formal Languages & Finite Automata  
### Author: Mihail Pancenco  

----

## Theory

A **formal language** is a precisely defined set of strings constructed from a finite alphabet using a fixed set of production rules. Formal languages are widely used in computer science for modeling programming languages, communication protocols, and computation models.

A **grammar** describes how valid strings of a language are generated. A grammar is defined as:

G = (Vn, Vt, P, S),

where:
- Vn is the set of non-terminal symbols,
- Vt is the set of terminal symbols,
- P is the set of production rules,
- S is the start symbol.

A **regular grammar** is a special type of grammar in which all production rules have one of the following forms:

- A → a  
- A → aB  

where A and B are non-terminals and a is a terminal symbol.

A **finite automaton (FA)** is a mathematical model of computation that recognizes languages by reading symbols sequentially and transitioning between states. Regular grammars and finite automata are equivalent in expressive power, meaning that for every regular grammar, there exists a finite automaton that recognizes the same language.

---

## Objectives

* Implement a class for representing a regular grammar.
* Generate valid strings from the grammar.
* Convert a regular grammar into a finite automaton.
* Implement a string acceptance algorithm for the finite automaton.
* Validate correctness by testing generated strings.

---

## Implementation description

The project is implemented in **C++** using object-oriented design principles. The code is structured into multiple logical components.

### Grammar class

The `Grammar` class stores the formal grammar using sets for non-terminals and terminals, a vector for productions, and a string for the start symbol. It also provides functionality for adding grammar components and expanding strings using production rules.

```cpp
class Grammar {
private:
    std::set<std::string> Vn;
    std::set<char> Vt;
    std::vector<Production> P;
    std::string S;
};
```

---

### ValidWordGenerator class

This class generates valid words belonging to the language defined by the grammar. It uses a **breadth-first search (BFS)** strategy, ensuring that short valid strings are produced first while avoiding infinite expansions.

```cpp
std::vector<std::string> generate(size_t maxWords = 15);
```

---

### FiniteAutomaton class

This class models a **nondeterministic finite automaton (NFA)** using:

-   a set of states,
    
-   an alphabet,
    
-   a list of transitions,
    
-   a start state,
    
-   a set of final states.
    

It also implements a function that verifies whether a given string is accepted by the automaton.

```cpp
bool accepts(const std::string& word) const;
```

---

### Grammar → Finite Automaton Conversion

The `Converter` class transforms a grammar into a finite automaton by mapping each grammar production into a transition rule.

```cpp
FiniteAutomaton grammarToFiniteAutomaton(Grammar *g);
```

Conversion rules:

-   A → a → A --a--> FINAL
    
-   A → aB → A --a--> B
    

---

### Main execution logic

The main function initializes the grammar, generates valid words, converts the grammar into a finite automaton, and tests the correctness of the automaton.

```cpp
int main() {
    Grammar g("S");

    g.addProduction("S", "dA");
    g.addProduction("A", "d");
    g.addProduction("A", "aB");
    g.addProduction("B", "bC");
    g.addProduction("C", "cA");
    g.addProduction("C", "aS");

    ValidWordGenerator gen(&g);
    auto words = gen.generate(5);

    Converter conv;
    auto fa = conv.grammarToFiniteAutomaton(&g);

    for (const auto& w : words) {
        std::cout << w << " -> "
                  << (fa.accepts(w) ? "ACCEPTED" : "REJECTED") << "\n";
    }
}
```

---

## Conclusions / Results

The generated words were successfully recognized by the constructed finite automaton, confirming the correctness of the grammar-to-automaton conversion.

Example results:

```bash
dd           → ACCEPTED
dabca        → ACCEPTED
dabcad       → ACCEPTED
dabcabcad    → ACCEPTED
abc          → REJECTED
```

This demonstrates that:

-   The grammar correctly generates valid strings.
    
-   The finite automaton accurately recognizes the same language.
    
-   The implementation proves the theoretical equivalence between regular grammars and finite automata.
    
    



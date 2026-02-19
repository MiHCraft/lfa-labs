// #########################################################################################
// # Topic: Determinism in Finite Automata. Conversion from NDFA 2 DFA. Chomsky Hierarchy. #
// #########################################################################################


// <===== Includes ====>
#include <iostream> //for input and output. printf and scanf also there.
#include <vector> // this is an array with predefined good functions.
#include <set> // this is a set. Like an array, here are stored only unique elements. So that <a, b, a> and <a, b> is the same set. and also as <b, a>
#include <queue> // this is already implemented queue for some alggorithms.
#include <string> // just to not to use array of characters.
#include <cstdio> // ChatGPT said that it is important for printf, on other OS it might not work without it.
#include <memory> // chatGPT said that this is important for unique_ptr, which are some sort of smart pointers

// compile commamd, use the last version of c++
// g++ -std=c++23 main.cpp -o main.out

// We use an OOP style when all the classes store other objects (aka. composition) by pointers. And all the objects are stoted in RAM. Why this choice? Because it is easy for me to write.


// The grammar has nonterminal symbols, terminal symbols, rule of productions and a starting symbol.
// so, it has symbols and a productions.
// while the production is a set of 2 words. from the what word we go to what word.
//
// Firstly, we have a letter. A Letter is an abstract class, as there are only 2 types of Letters: terminal and non-terminal.
class AbstractLetter {
protected:
	char letter;
public:
	AbstractLetter(char letter){
		this->letter = letter;
	}

	virtual ~AbstractLetter() {}

	void setLetter(char letter) {
		this->letter = letter;
	}

	char getLetter() const {
		return this->letter;
	}

	void print() const {
		printf("%c", letter);
	}

	// This method is fully virtual, that makes this class Abstract.
	virtual bool isTerminal() const = 0;
};

class Terminal : public AbstractLetter {
public:
	Terminal(char letter) : AbstractLetter(letter) {}

	bool isTerminal() const override {
		return true;
	}
};

class NonTerminal : public AbstractLetter {
public:
	NonTerminal(char letter) : AbstractLetter(letter) {}

	bool isTerminal() const override {
		return false;
	}
};


class Word {
private:
	std::vector<AbstractLetter*> word;
public:
	void appendLetter(AbstractLetter* letter){
		word.push_back(letter);
	}
	
	bool containsTerminal() const {
		for (const auto& letter : word)
            		if (letter->isTerminal()) return true;
        	return false;
	}

	void print() const {
		for (const auto& letter : word) {
			letter->print();
		}
	}

	// says if this words ends with a given word
	bool endsWith(Word* givenWord) const{
	// return back later

		if (word.size() < givenWord->word.size())
            return false;

        size_t offset = word.size() - givenWord->word.size();

        for (size_t i = 0; i < givenWord->word.size(); ++i) {
            if (word[offset + i] != givenWord->word[i])
                return false;
        }

        return true;
	}

};


class Prodution {
private:
	Word* from;
	Word* to;
public:
	Production(Word* from, Word* to){
		this->from = from;
		this->to = to;
	}

	void print() const{
		this->from->print();
		this->printf(" -> ");
		this->to->print();
		printf(";\n");
	}

	bool canApplyRight(const Word* word){
		if (word->endsWith(this->from)){
			return true;
		}
		return false;
	}

	Word* applyRight(constWord* word){

	}
}

class Grammar {
private:
	std::vector<Terminal*> terminals;
	std::vector<NonTerminal*> nonTerminals;
	std::vector<Production*> productions;
	NonTerminal* startingSymbol;
public:
	// the Constructor in previous version has been accepting the starting symbol. So, for now just use this.
	Grammar(NonTerminal* startingSymbol){
		this->startingSymbol = startingSymbol;
	}

	// Expand Right is an operation that returns the set of all possible expansions from current word to another word.
	std::vector<Word*> expandRight(Word* currentWord) const{
		std::vector<Word*> resultingWords;
		for (const auto& p : productions){
			
			// will return later soon


		}

	}
}


/* ===== Grammar ===== */
class Grammar {
private:
    std::set<std::string> Vn;
    std::set<char> Vt;
    std::vector<Production> P;
    std::string S;

public:
    Grammar(const std::string& startSymbol) : S(startSymbol) {}

    bool isNonTerminal(const std::string& s) const {
        return Vn.find(s) != Vn.end();
    }

    std::vector<std::string> expandRight(const std::string& cur) const {
        std::vector<std::string> res;

        for (const auto& p : P) {
            if (cur.size() >= p.from.size() &&
                cur.compare(cur.size() - p.from.size(),
                            p.from.size(),
                            p.from) == 0) {

                res.push_back(
                    cur.substr(0, cur.size() - p.from.size()) + p.to
                );
            }
        }

        return res;
    }


    void addNonTerminal(const std::string& nt) {
        Vn.insert(nt);
    }

    void addTerminal(char t) {
        Vt.insert(t);
    }

    void addProduction(const std::string& from, const std::string& to) {
        P.push_back({from, to});
    }

    const std::vector<Production>& getProductions() const {
        return P;
    }

    const std::string& getStart() const {
        return S;
    }

    void print() const {
        std::cout << "VN = { ";
        for (const auto& s : Vn) std::cout << s << " ";
        std::cout << "}\n";

        std::cout << "VT = { ";
        for (char c : Vt) std::cout << c << " ";
        std::cout << "}\n";

        std::cout << "P:\n";
        for (const auto& p : P) {
            std::cout << "  " << p.from << " -> " << p.to << "\n";
        }

        std::cout << "S = " << S << "\n";
    }
};

/* ===== Word Generator ===== */
class ValidWordGenerator {
private:
    const Grammar* g;

public:
    ValidWordGenerator(const Grammar* grammar) : g(grammar) {}

    std::vector<std::string> generate(size_t maxWords = 15) {
        std::queue<std::string> q;
        std::set<std::string> seen;
        std::vector<std::string> result;

        q.push(g->getStart());

        size_t generated = 0;

        while (!q.empty() && generated < maxWords) {
            std::string cur = q.front();
            q.pop();

            auto nextStates = g->expandRight(cur);

            if (nextStates.empty()) {
                result.push_back(cur);
                ++generated;
                continue;
            }

            for (const auto& next : nextStates) {
                if (seen.insert(next).second) {
                    q.push(next);
                }
            }
        }

        return result;
    }

};




/* ===== Finite Automaton ===== */
class FiniteAutomaton {
public:
    using State = std::string;

    struct Transition {
        State from;
        char  symbol;
        State to;
    };

private:
    std::set<State> states;          // Q
    std::set<char> alphabet;         // Σ
    std::vector<Transition> delta;   // δ
    State start;                     // q0
    std::set<State> finals;          // F

public:
    FiniteAutomaton(const State& startState)
        : start(startState)
    {
        states.insert(start);
    }

    void addState(const State& s) {
        states.insert(s);
    }

    void addFinalState(const State& s) {
        states.insert(s);
        finals.insert(s);
    }

    void addSymbol(char c) {
        alphabet.insert(c);
    }

    void addTransition(const State& from, char symbol, const State& to) {
        states.insert(from);
        states.insert(to);
        alphabet.insert(symbol);
        delta.push_back({from, symbol, to});
    }

    const State& getStart() const {
        return start;
    }

    const std::set<State>& getStates() const {
        return states;
    }

    const std::set<State>& getFinalStates() const {
        return finals;
    }

    const std::vector<Transition>& getTransitions() const {
        return delta;
    }

    bool accepts(const std::string& word) const {
        std::set<State> current;
        current.insert(start);

        for (char c : word) {
            std::set<State> next;

            for (const auto& s : current) {
                for (const auto& tr : delta) {
                    if (tr.from == s && tr.symbol == c) {
                        next.insert(tr.to);
                    }
                }
            }

            if (next.empty()) return false;

            current = std::move(next);
        }

        for (const auto& s : current) {
            if (finals.count(s)) return true;
        }

        return false;
    }


    void print() const {
        std::cout << "States: { ";
        for (const auto& s : states) std::cout << s << " ";
        std::cout << "}\n";

        std::cout << "Alphabet: { ";
        for (char c : alphabet) std::cout << c << " ";
        std::cout << "}\n";

        std::cout << "Start: " << start << "\n";

        std::cout << "Final states: { ";
        for (const auto& s : finals) std::cout << s << " ";
        std::cout << "}\n";

        std::cout << "Transitions:\n";
        for (const auto& t : delta) {
            std::cout << "  " << t.from << " --" << t.symbol << "--> " << t.to << "\n";
        }
    }
};


class Converter {

    public:
        FiniteAutomaton grammarToFiniteAutomaton(Grammar *g){
            const std::string FINAL = "__FINAL__";

            FiniteAutomaton fa(g->getStart());

            fa.addFinalState(FINAL);

            for (const auto& p : g->getProductions()) {

                const std::string& from = p.from;
                const std::string& rhs  = p.to;

                if (rhs.empty()) continue;

                char terminal = rhs[0];

                // Case 1: A -> a
                if (rhs.size() == 1) {
                    fa.addTransition(from, terminal, FINAL);
                }
                // Case 2: A -> aB
                else {
                    std::string to = rhs.substr(1);
                    fa.addTransition(from, terminal, to);
                }
            }

            return fa;
        }
};


int main() {
    Grammar g("S");

    // variant 20
    g.addNonTerminal("S");
    g.addNonTerminal("A");
    g.addNonTerminal("B");
    g.addNonTerminal("C");

    g.addTerminal('a');
    g.addTerminal('b');
    g.addTerminal('c');
    g.addTerminal('d');

    g.addProduction("S", "dA");
    g.addProduction("A", "d");
    g.addProduction("A", "aB");
    g.addProduction("B", "bC");
    g.addProduction("C", "cA");
    g.addProduction("C", "aS");

    g.print();

    ValidWordGenerator gen(&g);

    auto words = gen.generate(5);

    for (const auto& w : words) {
        std::cout << w << "\n";
    }

    printf("\n\n----\n");

    Converter conv;
    auto fa = conv.grammarToFiniteAutomaton(&g);

    std::cout << "\n=== Finite Automaton ===\n";
    fa.print();


    printf("\n\n----\n");
    std::vector<std::string> tests = {
        "dd",
        "dabca",
        "dabcad",
        "dabcabcad",
        "abc"
    };



    for (const auto& w : tests) {
        std::cout << w << " -> " 
                << (fa.accepts(w) ? "ACCEPTED" : "REJECTED") 
                << "\n";
    }

    printf("\n\n");
    // should be everywhere ACCEPTED
    for (const auto& w : words) {
        std::cout << w << " -> " 
                << (fa.accepts(w) ? "ACCEPTED" : "REJECTED") 
                << "\n";
    }



    return 0;
}

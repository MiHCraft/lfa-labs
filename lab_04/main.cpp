#include <iostream>
#include <vector>
#include <set>
#include <queue>
#include <string>
#include <cstdio>
#include <memory>
#include <fstream>
#include <cctype>
#include <stdexcept>
#include <map>
#include <cmath>
#include <algorithm>

using namespace std;

#include <raylib.h>

// ============================================================
// ==================== YOUR ORIGINAL CODE (unchanged) ====================
// ============================================================

class AbstractLetter {
protected:
	std::string letter;
public:
	AbstractLetter(std::string letter) : letter(letter) {}
	virtual ~AbstractLetter() {}
	void setLetter(std::string l) { letter = l; }
	std::string getLetter() const { return letter; }
	void print() const { std::cout << letter; }
	virtual bool isTerminal() const = 0;
	bool isNonTerminal() const { return !isTerminal(); }
	bool equals(const AbstractLetter* other) const {
		return letter == other->letter && isTerminal() == other->isTerminal();
	}
};

class Terminal : public AbstractLetter {
public:
	Terminal(std::string letter) : AbstractLetter(letter) {}
	bool isTerminal() const override { return true; }
};

class NonTerminal : public AbstractLetter {
public:
	NonTerminal(std::string letter) : AbstractLetter(letter) {}
	bool isTerminal() const override { return false; }
};

class Word {
private:
	std::vector<const AbstractLetter*> word;
public:
	void appendLetter(const AbstractLetter* letter) { word.push_back(letter); }
	bool containsTerminal() const {
		for (const auto& letter : word) if (letter->isTerminal()) return true;
		return false;
	}
	bool containsNonTerminal() const {
		for (const auto& letter : word) if (letter->isNonTerminal()) return true;
		return false;
	}
	void print() const { for (const auto& letter : word) letter->print(); }
	size_t getLength() const { return word.size(); }
	std::vector<const AbstractLetter*> getAllLetters() const { return word; }
	bool endsWith(const Word* givenWord) const {
		if (getLength() < givenWord->getLength()) return false;
		size_t offset = getLength() - givenWord->getLength();
		for (size_t i = offset; i < getLength(); i++) {
			if (word[i]->getLetter() != givenWord->getAllLetters()[i - offset]->getLetter()) return false;
		}
		return true;
	}
	void deleteFromRight(int numberToDelete) {
		if (numberToDelete > static_cast<int>(word.size())) { printf("Do not delete more letters than you have!!!"); std::abort(); }
		for (int i = 0; i < numberToDelete; i++) word.pop_back();
	}
	void appendRightWord(const Word* toAdd) {
		for (const auto& letter : toAdd->getAllLetters()) appendLetter(letter);
	}
	bool equals(const Word* otherWord) const {
		if (word.size() != otherWord->word.size()) return false;
		for (size_t i = 0; i < word.size(); i++) {
			if (!word[i]->equals(otherWord->word[i])) return false;
		}
		return true;
	}
};

class Production {
private:
	std::unique_ptr<Word> from;
	std::unique_ptr<Word> to;
public:
	Production(std::unique_ptr<Word> from, std::unique_ptr<Word> to) : from(std::move(from)), to(std::move(to)) {}
	void print() const { from->print(); std::cout << " -> "; to->print(); std::cout << ";\n"; }
	bool canApplyRight(const Word* word) const { return word->endsWith(from.get()); }
	void applyRight(Word* word) const {
		if (canApplyRight(word)) {
			int sizeToDelete = from->getLength();
			word->deleteFromRight(sizeToDelete);
			word->appendRightWord(to.get());
		}
	}
	Word* getFrom() const { return from.get(); }
	Word* getTo() const { return to.get(); }
	std::unique_ptr<Word> applyRightNew(const Word* oldWord) const {
		auto newWord = std::make_unique<Word>(*oldWord);
		applyRight(newWord.get());
		return newWord;
	}
};

class Grammar {
private:
	std::vector<std::unique_ptr<Terminal>> terminals;
	std::vector<std::unique_ptr<NonTerminal>> nonTerminals;
	std::vector<std::unique_ptr<Production>> productions;
	const NonTerminal* startingSymbol;
public:
	Grammar(std::unique_ptr<NonTerminal> startingSymbol) {
		this->startingSymbol = startingSymbol.get();
		nonTerminals.push_back(std::move(startingSymbol));
	}
	std::vector<std::unique_ptr<Word>> getPossibleExpandRight(const Word* currentWord) const {
		std::vector<std::unique_ptr<Word>> resultingWords;
		for (const auto& p : productions) {
			if (p->canApplyRight(currentWord)) {
				resultingWords.push_back(p->applyRightNew(currentWord));
			}
		}
		return resultingWords;
	}
	void addNonTerminal(std::unique_ptr<NonTerminal> nonTermLetter) { nonTerminals.push_back(std::move(nonTermLetter)); }
	void addTerminal(std::unique_ptr<Terminal> TermLetter) { terminals.push_back(std::move(TermLetter)); }
	void addProduction(std::unique_ptr<Production> production) { productions.push_back(std::move(production)); }
	std::vector<const Production*> getProductions() const {
		std::vector<const Production*> result; result.reserve(productions.size());
		for (const auto& pr : productions) result.push_back(pr.get());
		return result;
	}
	const NonTerminal* getStartingSymbol() const { return startingSymbol; }
	std::vector<const NonTerminal*> getNonTerminals() const {
		std::vector<const NonTerminal*> result; result.reserve(nonTerminals.size());
		for (const auto& nt : nonTerminals) result.push_back(nt.get());
		return result;
	}
	std::vector<const Terminal*> getTerminals() const {
		std::vector<const Terminal*> result; result.reserve(terminals.size());
		for (const auto& t : terminals) result.push_back(t.get());
		return result;
	}
	std::string getStartingSymbolName() const { return startingSymbol->getLetter(); }
	const NonTerminal* getNonTerminalByLetter(const std::string& letter) const {
		for (const auto& nt : nonTerminals) if (nt->getLetter() == letter) return nt.get();
		return nullptr;
	}
	const Terminal* getTerminalByLetter(const std::string& letter) const {
		for (const auto& t : terminals) if (t->getLetter() == letter) return t.get();
		return nullptr;
	}
};

class ValidWordGenerator {
private:
	const Grammar* grammar;
	bool alreadySeen(const Word* word, std::vector<std::unique_ptr<Word>>& seen) const {
		for (auto& w : seen) if (word->equals(w.get())) return true;
		return false;
	}
public:
	ValidWordGenerator(const Grammar* grammar) : grammar(grammar) {}
	std::vector<std::unique_ptr<Word>> generate(size_t maxWords = 15) {
		std::queue<std::unique_ptr<Word>> queue;
		std::vector<std::unique_ptr<Word>> seen;
		std::vector<std::unique_ptr<Word>> result;
		size_t generated = 0;
		bool unlimited = (maxWords == 0);
		auto startWord = std::make_unique<Word>();
		startWord->appendLetter(grammar->getStartingSymbol());
		queue.push(std::move(startWord));
		while (!queue.empty() && (unlimited || generated < maxWords)) {
			auto current = std::move(queue.front()); queue.pop();
			if (alreadySeen(current.get(), seen)) continue;
			if (!current->containsNonTerminal()) {
				result.push_back(std::make_unique<Word>(*current.get()));
				generated++;
			}
			auto expansions = grammar->getPossibleExpandRight(current.get());
			for (auto& newWord : expansions) queue.push(std::move(newWord));
			seen.push_back(std::move(current));
		}
		return result;
	}
};

class State {
private:
	std::string nameOfState;
public:
	State(std::string name) : nameOfState(name) {}
	std::string getName() const { return nameOfState; }
	bool isName(std::string name) const { return nameOfState == name; }
};

class Symbol {
private:
	std::string nameOfSymbol;
public:
	Symbol(std::string name) : nameOfSymbol(name) {}
	std::string getNameOfSymbol() const { return nameOfSymbol; }
	bool isName(std::string name) const { return nameOfSymbol == name; }
};

class Transition {
private:
	const State* stateFrom;
	const Symbol* viaSymbol;
	const State* stateTo;
public:
	Transition(const State* from, const Symbol* via, const State* to) : stateFrom(from), viaSymbol(via), stateTo(to) {}
	bool isFromStateAndViaSymbol(const State* from, const Symbol* symbol) const {
		return stateFrom == from && viaSymbol->getNameOfSymbol() == symbol->getNameOfSymbol();
	}
	const State* getFromState() const { return stateFrom; }
	const Symbol* getSymbol() const { return viaSymbol; }
	const State* getToState() const { return stateTo; }
};

class AbstractAutomaton {
protected:
	std::vector<std::unique_ptr<State>> states;
	std::vector<std::unique_ptr<Symbol>> alphabet;
	std::vector<std::unique_ptr<Transition>> transitions;
	const State* startingState = nullptr;
	std::vector<const State*> finalStates;
	bool containsState(const State* state) const {
		for (const auto& st : states) if (st.get() == state) return true;
		return false;
	}
	bool isFinalState(const State* state) const {
		for (const State* st : finalStates) if (st == state) return true;
		return false;
	}
	bool containsSymbol(const Symbol* symbol) const {
		for (const auto& sy : alphabet) if (sy.get() == symbol) return true;
		return false;
	}
	bool containsTransition(const Transition* transition) const {
		for (const auto& tr : transitions) if (tr.get() == transition) return true;
		return false;
	}
public:
	AbstractAutomaton() = default;
	explicit AbstractAutomaton(std::unique_ptr<State> start) {
		startingState = start.get();
		states.push_back(std::move(start));
	}
	virtual ~AbstractAutomaton() = default;
	void addState(std::unique_ptr<State> state) { if (!containsState(state.get())) states.push_back(std::move(state)); }
	void setStateAsFinal(const State* finalState) {
		if (containsState(finalState) && !isFinalState(finalState)) finalStates.push_back(finalState);
	}
	void addFinalState(std::unique_ptr<State> state) {
		const State* ptr = state.get();
		addState(std::move(state));
		setStateAsFinal(ptr);
	}
	void addSymbol(std::unique_ptr<Symbol> symbol) {
		// deduplicate by name to avoid duplicate alphabet entries
		for (const auto& sy : alphabet) {
			if (sy->getNameOfSymbol() == symbol->getNameOfSymbol()) return;
		}
		alphabet.push_back(std::move(symbol));
	}
	virtual void addTransition(std::unique_ptr<Transition> transition) {
		if (!containsTransition(transition.get())) transitions.push_back(std::move(transition));
	}
	const State* getStateByName(std::string name) const {
		for (const auto& state : states) if (state->isName(name)) return state.get();
		return nullptr;
	}
	const Symbol* getSymbolByName(std::string name) {
		for (auto& symbol : alphabet) if (symbol->isName(name)) return symbol.get();
		return nullptr;
	}
	const State* getStartingState() const { return startingState; }
	std::vector<const State*> getStates() const {
		std::vector<const State*> result; for (const auto& s : states) result.push_back(s.get()); return result;
	}
	std::vector<const Symbol*> getAlphabet() const {
		std::vector<const Symbol*> result; for (const auto& s : alphabet) result.push_back(s.get()); return result;
	}
	std::vector<const Transition*> getTransitions() const {
		std::vector<const Transition*> result; for (const auto& t : transitions) result.push_back(t.get()); return result;
	}
	std::vector<const State*> getFinalStates() const { return finalStates; }
	virtual bool accepts(const std::vector<std::unique_ptr<Symbol>> word) const = 0;
};

class NonDeterministicAutomaton : public AbstractAutomaton {
public:
	explicit NonDeterministicAutomaton(std::unique_ptr<State> start) : AbstractAutomaton(std::move(start)) {}

	std::set<const State*> epsilon_closure(const std::set<const State*>& states) const {
		std::set<const State*> closure = states;
		std::queue<const State*> q;
		for (const State* s : states) q.push(s);
		while (!q.empty()) {
			const State* u = q.front(); q.pop();
			for (const auto& tr : transitions) {
				if (tr->getFromState() == u && tr->getSymbol()->getNameOfSymbol() == "ε") {
					const State* v = tr->getToState();
					if (closure.find(v) == closure.end()) {
						closure.insert(v);
						q.push(v);
					}
				}
			}
		}
		return closure;
	}

	bool accepts(const std::vector<std::unique_ptr<Symbol>> word) const override {
		std::set<const State*> current = {startingState};
		current = epsilon_closure(current);
		for (const auto& sym_ptr : word) {
			std::set<const State*> next;
			const Symbol* sym = sym_ptr.get();
			for (const State* s : current) {
				for (const auto& tr : transitions) {
					if (tr->getFromState() == s && tr->getSymbol()->getNameOfSymbol() == sym->getNameOfSymbol()) {
						next.insert(tr->getToState());
					}
				}
			}
			current = epsilon_closure(next);
		}
		for (const State* st : current) {
			if (isFinalState(st)) return true;
		}
		return false;
	}
};

class DeterministicAutomaton : public AbstractAutomaton {
public:
	explicit DeterministicAutomaton(std::unique_ptr<State> start) : AbstractAutomaton(std::move(start)) {}
	void addTransition(std::unique_ptr<Transition> transition) override {
		const State* from = transition->getFromState();
		const Symbol* sym = transition->getSymbol();
		for (const auto& tr : transitions) {
			if (tr->isFromStateAndViaSymbol(from, sym)) {
				printf("ERROR: DFA cannot have multiple transitions for same state and symbol\n");
				std::abort();
			}
		}
		transitions.push_back(std::move(transition));
	}
	bool accepts(const std::vector<std::unique_ptr<Symbol>> word) const override {
		const State* current = startingState;
		for (const auto& symbol : word) {
			const State* next = nullptr;
			for (const auto& tr : transitions) {
				if (tr->isFromStateAndViaSymbol(current, symbol.get())) {
					next = tr->getToState();
					break;
				}
			}
			if (next == nullptr) return false;
			current = next;
		}
		return isFinalState(current);
	}
};

class ConverterNFAToDFA {
private:
	std::string makeStateName(const std::set<const State*>& states) const {
		std::string name = "{";
		bool first = true;
		for (const State* s : states) {
			if (!first) name += ",";
			name += s->getName();
			first = false;
		}
		name += "}";
		return name;
	}
	std::set<const State*> move(const NonDeterministicAutomaton* nfa, const std::set<const State*>& states, const Symbol* symbol) const {
		std::set<const State*> result;
		for (const State* st : states) {
			for (const Transition* tr : nfa->getTransitions()) {
				if (tr->isFromStateAndViaSymbol(st, symbol)) result.insert(tr->getToState());
			}
		}
		return result;
	}
public:
	std::unique_ptr<DeterministicAutomaton> convert(const NonDeterministicAutomaton* nfa) const {
		std::set<const State*> startSet = {nfa->getStartingState()};
		startSet = nfa->epsilon_closure(startSet);
		std::string startName = makeStateName(startSet);
		auto startState = std::make_unique<State>(startName);
		auto dfa = std::make_unique<DeterministicAutomaton>(std::move(startState));
		for (const Symbol* s : nfa->getAlphabet()) {
			if (s->getNameOfSymbol() != "ε") dfa->addSymbol(std::make_unique<Symbol>(s->getNameOfSymbol()));
		}
		std::queue<std::set<const State*>> queue;
		std::set<std::string> visited;
		queue.push(startSet);
		visited.insert(startName);
		while (!queue.empty()) {
			std::set<const State*> currentSet = queue.front(); queue.pop();
			std::string currentName = makeStateName(currentSet);
			const State* currentState = dfa->getStateByName(currentName);
			for (const State* s : currentSet) {
				for (const State* f : nfa->getFinalStates()) {
					if (s == f) { dfa->setStateAsFinal(currentState); break; }
				}
			}
			for (const Symbol* sym : nfa->getAlphabet()) {
				if (sym->getNameOfSymbol() == "ε") continue;
				std::set<const State*> nextSet = move(nfa, currentSet, sym);
				nextSet = nfa->epsilon_closure(nextSet);
				if (nextSet.empty()) continue;
				std::string nextName = makeStateName(nextSet);
				const State* nextState = dfa->getStateByName(nextName);
				if (nextState == nullptr) {
					auto newState = std::make_unique<State>(nextName);
					nextState = newState.get();
					dfa->addState(std::move(newState));
					if (!visited.count(nextName)) {
						queue.push(nextSet);
						visited.insert(nextName);
					}
				}
				const Symbol* dfaSym = dfa->getSymbolByName(sym->getNameOfSymbol());
				dfa->addTransition(std::make_unique<Transition>(currentState, dfaSym, nextState));
			}
		}
		return dfa;
	}
};

class ConverterAutomatonToGrammar {
public:
	std::unique_ptr<Grammar> convert(const AbstractAutomaton* automaton) const {
		std::string startName = automaton->getStartingState()->getName();
		auto startNT = std::make_unique<NonTerminal>(startName);
		auto grammar = std::make_unique<Grammar>(std::move(startNT));
		for (const auto& st : automaton->getStates()) {
			std::string stName = st->getName();
			if (stName != startName) grammar->addNonTerminal(std::make_unique<NonTerminal>(stName));
		}
		for (const auto& sym : automaton->getAlphabet()) {
			if (sym->getNameOfSymbol() != "ε") grammar->addTerminal(std::make_unique<Terminal>(sym->getNameOfSymbol()));
		}
		for (const auto& tr : automaton->getTransitions()) {
			if (tr->getSymbol()->getNameOfSymbol() == "ε") continue;
			auto fromW = std::make_unique<Word>();
			fromW->appendLetter(grammar->getNonTerminalByLetter(tr->getFromState()->getName()));
			auto toW = std::make_unique<Word>();
			toW->appendLetter(grammar->getTerminalByLetter(tr->getSymbol()->getNameOfSymbol()));
			toW->appendLetter(grammar->getNonTerminalByLetter(tr->getToState()->getName()));
			grammar->addProduction(std::make_unique<Production>(std::move(fromW), std::move(toW)));
		}
		for (const State* f : automaton->getFinalStates()) {
			auto fromW = std::make_unique<Word>();
			fromW->appendLetter(grammar->getNonTerminalByLetter(f->getName()));
			auto toW = std::make_unique<Word>();
			grammar->addProduction(std::make_unique<Production>(std::move(fromW), std::move(toW)));
		}
		return grammar;
	}
};

void printAutomaton(const AbstractAutomaton* a) {
	printf("\nStates:\n");
	for (const State* s : a->getStates()) printf("%s\n", s->getName().c_str());
	printf("\nAlphabet:\n");
	for (const Symbol* s : a->getAlphabet()) if (s->getNameOfSymbol() != "ε") printf("%s\n", s->getNameOfSymbol().c_str());
	printf("\nTransitions:\n");
	for (const Transition* t : a->getTransitions()) {
		printf("δ(%s,%s) -> %s\n", t->getFromState()->getName().c_str(), t->getSymbol()->getNameOfSymbol().c_str(), t->getToState()->getName().c_str());
	}
	printf("\nStart: %s\n", a->getStartingState()->getName().c_str());
	printf("\nFinal states:\n");
	for (const State* s : a->getFinalStates()) printf("%s\n", s->getName().c_str());
}

void printGrammar(const Grammar* g) {
	printf("\nNonTerminals:\n");
	for (auto nt : g->getNonTerminals()) printf("%s\n", nt->getLetter().c_str());
	printf("\nTerminals:\n");
	for (auto t : g->getTerminals()) printf("%s\n", t->getLetter().c_str());
	printf("\nProductions:\n");
	for (auto p : g->getProductions()) p->print();
	printf("\nStart symbol: %s\n", g->getStartingSymbol()->getLetter().c_str());
}

void printGeneratedWords(const Grammar* g, int amount = 10) {
	ValidWordGenerator generator(g);
	auto words = generator.generate(amount);
	int i = 1;
	for (const auto& w : words) {
		printf("%d: ", i++);
		if (w->getLength() == 0) {
			printf("ε");
		} else {
			w->print();
		}
		printf("\n");
	}
}

void testWordsOnAutomaton(const AbstractAutomaton* automaton, const std::vector<std::unique_ptr<Word>>& words) {
	printf("\nTesting words on automaton:\n");
	for (const auto& w : words) {
		std::vector<std::unique_ptr<Symbol>> input;
		for (const auto& letter : w->getAllLetters()) {
			input.push_back(std::make_unique<Symbol>(letter->getLetter()));
		}
		bool accepted = automaton->accepts(std::move(input));
		w->print();
		printf(" -> %s\n", accepted ? "ACCEPT" : "REJECT");
	}
}

// ============================================================
// ====================== AST + PARSER ========================
// ============================================================

constexpr int kRepeatLimit = 5; // limit for '*' and '+'

enum class TokenType {
	SYMBOL,
	LPAREN,
	RPAREN,
	OR,
	STAR,
	PLUS,
	OPTIONAL,
	POWER
};

struct Token {
	TokenType type;
	char value = 0;   // for SYMBOL
	int number = 0;   // for POWER
};

std::vector<Token> tokenizeRegex(const std::string& s) {
	std::vector<Token> tokens;
	for (size_t i = 0; i < s.size(); ) {
		char c = s[i];
		if (isspace(static_cast<unsigned char>(c))) { i++; continue; }
		switch (c) {
			case '(' : tokens.push_back({TokenType::LPAREN}); i++; break;
			case ')' : tokens.push_back({TokenType::RPAREN}); i++; break;
			case '|' : tokens.push_back({TokenType::OR}); i++; break;
			case '*' : tokens.push_back({TokenType::STAR}); i++; break;
			case '+' : tokens.push_back({TokenType::PLUS}); i++; break;
			case '?' : tokens.push_back({TokenType::OPTIONAL}); i++; break;
			case '^' : {
				i++;
				if (i >= s.size() || !isdigit(static_cast<unsigned char>(s[i]))) {
					throw runtime_error("Expected number after ^");
				}
				int num = 0;
				while (i < s.size() && isdigit(static_cast<unsigned char>(s[i]))) {
					num = num * 10 + (s[i] - '0');
					i++;
				}
				Token t; t.type = TokenType::POWER; t.number = num;
				tokens.push_back(t);
				break;
			}
			default: {
				Token t; t.type = TokenType::SYMBOL; t.value = c;
				tokens.push_back(t);
				i++;
				break;
			}
		}
	}
	return tokens;
}

std::string tokenToString(const Token& t) {
	switch (t.type) {
		case TokenType::SYMBOL: return std::string("SYMBOL(") + t.value + ")";
		case TokenType::LPAREN: return "LPAREN";
		case TokenType::RPAREN: return "RPAREN";
		case TokenType::OR: return "OR";
		case TokenType::STAR: return "STAR";
		case TokenType::PLUS: return "PLUS";
		case TokenType::OPTIONAL: return "OPTIONAL";
		case TokenType::POWER: return "POWER(" + std::to_string(t.number) + ")";
	}
	return "UNKNOWN";
}

void printTokens(const std::vector<Token>& tokens) {
	std::cout << "\nTokens:\n";
	for (const auto& t : tokens) {
		std::cout << tokenToString(t) << " ";
	}
	std::cout << "\n";
}

enum class NodeType { SYMBOL, CONCAT, OR, REPEAT, OPTIONAL };

struct Node {
	NodeType type;
	char value = 0;
	int minRepeat = 1;
	int maxRepeat = 1;
	std::shared_ptr<Node> left, right, child;
	Node(NodeType t) : type(t) {}
};

class Parser {
public:
	Parser(const string& input) : s(input), pos(0) {}
	shared_ptr<Node> parse() {
		auto res = parse_expr();
		if (pos != s.size()) throw runtime_error("Unexpected input at position " + to_string(pos));
		return res;
	}
private:
	string s;
	size_t pos;
	char peek() { return pos < s.size() ? s[pos] : '\0'; }
	char get() { return pos < s.size() ? s[pos++] : '\0'; }
	bool match(char c) { if (peek() == c) { pos++; return true; } return false; }
	int parse_number() {
		int num = 0;
		if (!isdigit(peek())) throw runtime_error("Expected number after ^");
		while (isdigit(peek())) num = num * 10 + (get() - '0');
		return num;
	}
	shared_ptr<Node> parse_expr() {
		auto node = parse_term();
		while (match('|')) {
			auto right = parse_term();
			auto parent = make_shared<Node>(NodeType::OR);
			parent->left = node; parent->right = right; node = parent;
		}
		return node;
	}
	shared_ptr<Node> parse_term() {
		vector<shared_ptr<Node>> nodes;
		while (true) {
			char c = peek();
			if (c == '\0' || c == ')' || c == '|') break;
			nodes.push_back(parse_factor());
		}
		if (nodes.empty()) throw runtime_error("Expected term");
		auto node = nodes[0];
		for (size_t i = 1; i < nodes.size(); i++) {
			auto parent = make_shared<Node>(NodeType::CONCAT);
			parent->left = node; parent->right = nodes[i]; node = parent;
		}
		return node;
	}
	shared_ptr<Node> parse_factor() {
		auto node = parse_base();
		while (true) {
			if (match('*')) {
				// Kleene star limited to 0..kRepeatLimit repetitions
				auto p = make_shared<Node>(NodeType::REPEAT);
				p->child = node; p->minRepeat = 0; p->maxRepeat = kRepeatLimit; node = p;
			} else if (match('+')) {
				// Plus limited to 1..kRepeatLimit repetitions
				auto p = make_shared<Node>(NodeType::REPEAT);
				p->child = node; p->minRepeat = 1; p->maxRepeat = kRepeatLimit; node = p;
			} else if (match('?')) {
				auto p = make_shared<Node>(NodeType::REPEAT);
				p->child = node; p->minRepeat = 0; p->maxRepeat = 1; node = p;
			} else if (match('^')) {
				int n = parse_number();
				auto p = make_shared<Node>(NodeType::REPEAT);
				p->child = node; p->minRepeat = n; p->maxRepeat = n; node = p;
			} else break;
		}
		return node;
	}
	shared_ptr<Node> parse_base() {
		if (match('(')) {
			auto node = parse_expr();
			if (!match(')')) throw runtime_error("Expected ')'");
			return node;
		}
		char c = get();
		if (c == '\0') throw runtime_error("Unexpected end");
		auto node = make_shared<Node>(NodeType::SYMBOL);
		node->value = c;
		return node;
	}
};

void print_ast(shared_ptr<Node> node, int depth = 0) {
	string indent(depth * 2, ' ');
	switch (node->type) {
		case NodeType::SYMBOL: cout << indent << "SYMBOL(" << node->value << ")\n"; break;
		case NodeType::CONCAT: cout << indent << "CONCAT\n"; print_ast(node->left, depth+1); print_ast(node->right, depth+1); break;
		case NodeType::OR: cout << indent << "OR\n"; print_ast(node->left, depth+1); print_ast(node->right, depth+1); break;
		case NodeType::REPEAT:
			if (node->minRepeat == node->maxRepeat) {
				cout << indent << "REPEAT(^" << node->minRepeat << ")\n";
			} else {
				cout << indent << "REPEAT(" << node->minRepeat << ".." << node->maxRepeat << ")\n";
			}
			print_ast(node->child, depth+1);
			break;
		case NodeType::OPTIONAL: cout << indent << "OPTIONAL\n"; print_ast(node->child, depth+1); break;
	}
}

// ============================================================
// ================ AST → NFA Thompson (FIXED) ================
// ============================================================

std::unique_ptr<NonDeterministicAutomaton> build_nfa_from_ast(shared_ptr<Node> root) {
	auto automaton = std::make_unique<NonDeterministicAutomaton>(std::make_unique<State>("q0"));
	automaton->addSymbol(std::make_unique<Symbol>("ε"));
	int stateCounter = 1;

	auto newState = [&]() -> const State* {
		auto st = std::make_unique<State>("q" + to_string(stateCounter++));
		const State* ptr = st.get();
		automaton->addState(std::move(st));
		return ptr;
	};

	auto buildThompson = [&](auto&& self, shared_ptr<Node> n) -> pair<const State*, const State*> {
		if (n->type == NodeType::SYMBOL) {
			const State* s = newState();
			const State* e = newState();
			string symStr(1, n->value);
			automaton->addSymbol(std::make_unique<Symbol>(symStr));
			const Symbol* sym = automaton->getSymbolByName(symStr);
			automaton->addTransition(std::make_unique<Transition>(s, sym, e));
			return {s, e};
		}
		if (n->type == NodeType::CONCAT) {
			auto [s1, e1] = self(self, n->left);
			auto [s2, e2] = self(self, n->right);
			automaton->addTransition(std::make_unique<Transition>(e1, automaton->getSymbolByName("ε"), s2));
			return {s1, e2};
		}
		if (n->type == NodeType::OR) {
			const State* s = newState();
			const State* e = newState();
			auto [s1, e1] = self(self, n->left);
			auto [s2, e2] = self(self, n->right);
			automaton->addTransition(std::make_unique<Transition>(s, automaton->getSymbolByName("ε"), s1));
			automaton->addTransition(std::make_unique<Transition>(s, automaton->getSymbolByName("ε"), s2));
			automaton->addTransition(std::make_unique<Transition>(e1, automaton->getSymbolByName("ε"), e));
			automaton->addTransition(std::make_unique<Transition>(e2, automaton->getSymbolByName("ε"), e));
			return {s, e};
		}
		if (n->type == NodeType::REPEAT) {
			int minTimes = n->minRepeat;
			int maxTimes = n->maxRepeat;
			const Symbol* eps = automaton->getSymbolByName("ε");
			const State* start = newState();
			const State* end = newState();

			if (maxTimes == 0) {
				automaton->addTransition(std::make_unique<Transition>(start, eps, end));
				return {start, end};
			}

			const State* prevEnd = nullptr;
			for (int i = 1; i <= maxTimes; ++i) {
				auto [cs, ce] = self(self, n->child);
				if (i == 1) {
					automaton->addTransition(std::make_unique<Transition>(start, eps, cs));
				} else {
					automaton->addTransition(std::make_unique<Transition>(prevEnd, eps, cs));
				}
				if (i >= minTimes) {
					automaton->addTransition(std::make_unique<Transition>(ce, eps, end));
				}
				prevEnd = ce;
			}
			if (minTimes == 0) {
				automaton->addTransition(std::make_unique<Transition>(start, eps, end));
			}
			return {start, end};
		}
		if (n->type == NodeType::OPTIONAL) {
			const Symbol* eps = automaton->getSymbolByName("ε");
			const State* s = newState();
			const State* e = newState();
			auto [s1, e1] = self(self, n->child);
			automaton->addTransition(std::make_unique<Transition>(s, eps, s1));
			automaton->addTransition(std::make_unique<Transition>(s, eps, e));
			automaton->addTransition(std::make_unique<Transition>(e1, eps, e));
			return {s, e};
		}
		return {nullptr, nullptr};
	};

	auto [regex_start, regex_end] = buildThompson(buildThompson, root);

	// === FIXED: connect global start q0 to the start of the regex via ε-transition ===
	const Symbol* eps = automaton->getSymbolByName("ε");
	if (regex_start) {
		automaton->addTransition(std::make_unique<Transition>(automaton->getStartingState(), eps, regex_start));
	}
	automaton->setStateAsFinal(regex_end);

	return automaton;
}

// ============================================================
// =================== DFA VISUALIZATION ======================
// ============================================================

struct DfaSlide {
	std::string regex;
	std::unique_ptr<DeterministicAutomaton> dfa;
};

static Vector2 v2_add(Vector2 a, Vector2 b) { return {a.x + b.x, a.y + b.y}; }
static Vector2 v2_sub(Vector2 a, Vector2 b) { return {a.x - b.x, a.y - b.y}; }
static Vector2 v2_scale(Vector2 v, float s) { return {v.x * s, v.y * s}; }
static float v2_len(Vector2 v) { return std::sqrt(v.x * v.x + v.y * v.y); }
static Vector2 v2_norm(Vector2 v) { float l = v2_len(v); return (l > 0.0001f) ? v2_scale(v, 1.0f / l) : Vector2{0.0f, 0.0f}; }
static Vector2 v2_perp(Vector2 v) { return {-v.y, v.x}; }

static std::vector<Vector2> layoutCircle(int count, float radius, Vector2 center) {
	std::vector<Vector2> pos;
	if (count <= 0) return pos;
	pos.resize(count);
	float angleStep = 2.0f * PI / static_cast<float>(count);
	float startAngle = -PI / 2.0f;
	for (int i = 0; i < count; ++i) {
		float ang = startAngle + angleStep * i;
		pos[i] = {center.x + radius * std::cos(ang), center.y + radius * std::sin(ang)};
	}
	return pos;
}

static void drawArrow(Vector2 start, Vector2 end, float thickness, Color color) {
	DrawLineEx(start, end, thickness, color);
	Vector2 dir = v2_norm(v2_sub(end, start));
	Vector2 perp = v2_perp(dir);
	float arrowSize = 10.0f;
	Vector2 tip = end;
	Vector2 left = v2_add(v2_add(tip, v2_scale(dir, -arrowSize)), v2_scale(perp, arrowSize * 0.5f));
	Vector2 right = v2_add(v2_add(tip, v2_scale(dir, -arrowSize)), v2_scale(perp, -arrowSize * 0.5f));
	DrawTriangle(tip, left, right, color);
}

static std::string shortenName(const std::string& name, size_t maxLen = 10) {
	if (name.size() <= maxLen) return name;
	return name.substr(0, maxLen - 2) + "..";
}

static void drawDfaSlide(const DeterministicAutomaton* dfa, const std::string& regex, int index, int total) {
	const int screenW = GetScreenWidth();
	const int screenH = GetScreenHeight();
	const float stateRadius = 32.0f;
	const float margin = 90.0f;
	const float radius = std::min(screenW, screenH) * 0.5f - margin;
	Vector2 center = {screenW * 0.5f, screenH * 0.55f};

	auto states = dfa->getStates();
	int n = static_cast<int>(states.size());
	if (n == 0) return;

	auto positions = layoutCircle(n, radius, center);
	std::map<const State*, int> indexOf;
	for (int i = 0; i < n; ++i) indexOf[states[i]] = i;

	std::set<const State*> finalSet;
	for (const auto* s : dfa->getFinalStates()) finalSet.insert(s);

	std::map<std::pair<int, int>, std::set<std::string>> edgeLabels;
	for (const Transition* tr : dfa->getTransitions()) {
		int from = indexOf[tr->getFromState()];
		int to = indexOf[tr->getToState()];
		edgeLabels[{from, to}].insert(tr->getSymbol()->getNameOfSymbol());
	}

	// Header
	std::string header = "DFA " + std::to_string(index + 1) + " / " + std::to_string(total);
	DrawText(header.c_str(), 30, 20, 24, DARKGRAY);
	std::string regexLine = "Regex: " + regex;
	DrawText(regexLine.c_str(), 30, 50, 20, DARKGRAY);
	DrawText("Press SPACE for next (Backspace for previous)", 30, 75, 16, GRAY);

	// Edges
	for (const auto& kv : edgeLabels) {
		int from = kv.first.first;
		int to = kv.first.second;
		std::string label;
		for (const auto& s : kv.second) {
			if (!label.empty()) label += ",";
			label += s;
		}
		Vector2 pFrom = positions[from];
		Vector2 pTo = positions[to];
		if (from == to) {
			float loopR = stateRadius * 0.9f;
			Vector2 loopCenter = {pFrom.x, pFrom.y - stateRadius - loopR * 0.5f};
			DrawCircleLines(loopCenter.x, loopCenter.y, loopR, GRAY);
			DrawText(label.c_str(), loopCenter.x - MeasureText(label.c_str(), 16) / 2, loopCenter.y - loopR - 18, 16, GRAY);
			continue;
		}
		Vector2 dir = v2_norm(v2_sub(pTo, pFrom));
		Vector2 start = v2_add(pFrom, v2_scale(dir, stateRadius));
		Vector2 end = v2_add(pTo, v2_scale(dir, -stateRadius));
		drawArrow(start, end, 2.0f, GRAY);
		Vector2 mid = v2_add(start, v2_scale(v2_sub(end, start), 0.5f));
		Vector2 offset = v2_scale(v2_perp(dir), 14.0f);
		Vector2 labelPos = v2_add(mid, offset);
		DrawText(label.c_str(), labelPos.x - MeasureText(label.c_str(), 16) / 2, labelPos.y - 8, 16, GRAY);
	}

	// States
	for (int i = 0; i < n; ++i) {
		Vector2 p = positions[i];
		bool isFinal = finalSet.count(states[i]) > 0;
		DrawCircleV(p, stateRadius, RAYWHITE);
		DrawCircleLines(p.x, p.y, stateRadius, BLACK);
		if (isFinal) DrawCircleLines(p.x, p.y, stateRadius - 5, BLACK);

		std::string label = "S" + std::to_string(i);
		int labelW = MeasureText(label.c_str(), 18);
		DrawText(label.c_str(), p.x - labelW / 2, p.y - 10, 18, BLACK);

		std::string orig = shortenName(states[i]->getName());
		int origW = MeasureText(orig.c_str(), 14);
		DrawText(orig.c_str(), p.x - origW / 2, p.y + 12, 14, DARKGRAY);
	}

	// Start arrow
	const State* startState = dfa->getStartingState();
	int startIdx = indexOf[startState];
	Vector2 pStart = positions[startIdx];
	Vector2 from = {pStart.x - stateRadius - 40.0f, pStart.y};
	Vector2 to = {pStart.x - stateRadius, pStart.y};
	drawArrow(from, to, 2.0f, BLACK);
}

static void runDfaSlideshow(const std::vector<DfaSlide>& slides) {
	if (slides.empty()) return;
	InitWindow(1200, 800, "DFA Slideshow");
	SetTargetFPS(60);
	int index = 0;
	while (!WindowShouldClose()) {
		if (IsKeyPressed(KEY_SPACE) && index < static_cast<int>(slides.size()) - 1) index++;
		if (IsKeyPressed(KEY_BACKSPACE) && index > 0) index--;
		BeginDrawing();
		ClearBackground(RAYWHITE);
		drawDfaSlide(slides[index].dfa.get(), slides[index].regex, index, static_cast<int>(slides.size()));
		EndDrawing();
	}
	CloseWindow();
}

// ============================================================
// ====================== MAIN ======================
// ============================================================

int main() {
	ifstream fin("input.txt");
	if (!fin.is_open()) {
		cerr << "Cannot open input.txt\n";
		return 1;
	}

	string regex;
	int line_num = 1;
	std::vector<DfaSlide> slides;

	while (getline(fin, regex)) {
		if (regex.empty()) continue;

		cout << "============================================================\n";
		cout << "Regex #" << line_num << ": " << regex << "\n";
		cout << "============================================================\n";

		try {
			auto tokens = tokenizeRegex(regex);
			printTokens(tokens);

			Parser parser(regex);
			auto ast = parser.parse();

			cout << "\nAST:\n";
			print_ast(ast);

			auto nfa = build_nfa_from_ast(ast);

			ConverterNFAToDFA nfa2dfa;
			auto dfa = nfa2dfa.convert(nfa.get());

			ConverterAutomatonToGrammar fa2grammar;
			auto grammar = fa2grammar.convert(dfa.get());

			cout << "\nRegular Grammar:\n";
			printGrammar(grammar.get());

			cout << "\nGenerated valid words (all with repeat limit " << kRepeatLimit << "):\n";
			printGeneratedWords(grammar.get(), 0);

			DfaSlide slide;
			slide.regex = regex;
			slide.dfa = std::move(dfa);
			slides.push_back(std::move(slide));

			cout << "\nPipeline complete for this regex.\n";

		} catch (const exception& e) {
			cout << "Error in regex line " << line_num << ": " << e.what() << endl;
		}

		cout << "------------------------------------------------------------\n";
		line_num++;
	}

	cout << "\nAll regexes from input.txt processed successfully.\n";
	runDfaSlideshow(slides);
	return 0;
}

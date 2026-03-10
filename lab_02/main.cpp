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
	bool isNonTerminal() const{
		return !isTerminal();
	}

	bool equals(const AbstractLetter* otherLetter) const{
		if(this->letter != otherLetter->letter){
			return false;
		}
		if (this->isTerminal() != otherLetter->isTerminal()){
			return false;
		}
		return true;
	}
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
	std::vector<const AbstractLetter*> word;
public:
	void appendLetter(const AbstractLetter* letter){
		word.push_back(letter);
	}
	
	bool containsTerminal() const {
		for (const auto& letter : word)
            		if (letter->isTerminal()) return true;
        	return false;
	}

	bool containsNonTerminal() const {
		for (const auto& letter : word)
			if (letter->isNonTerminal()) return true;
		return false;
	}

	void print() const {
		for (const auto& letter : word) {
			letter->print();
		}
	}
	
	size_t getLength() const{
		return this->word.size();
	}

	std::vector<const AbstractLetter*> getAllLetters() const{
		return word;
	}

	// says if this words ends with a given word
	bool endsWith(const Word* givenWord) const{
	// return back later
		// if the given word is longer than this word, then this word can't end with the word that is longer
		if (this->getLength() < givenWord->getLength()){
			return false;
		}
		// where starts the ...
        	size_t offset = this->getLength() - givenWord->getLength();

        	for (size_t i = offset; i < this->getLength(); i++) {
       		     if (word[i]->getLetter() != givenWord->word[i-offset]->getLetter())
        	        return false;
       		}
		// no one different letter found
        	return true;
	}

	void deleteFromRight(int numberToDelete){
		if (numberToDelete > word.size()){
			printf("Do not delete more letters than you have!!!");
			std::abort();
		}

		for (int i = 0; i < numberToDelete; i++){
			//delete word.back(); // don't need to free memory from ram by pointer, because letters are stored in a Grammar.
			word.pop_back(); // remove the pointer itself from the vector will be enough.
		}
	}

	void appendRightWord(const Word* toAdd){
		for(const auto& letter : toAdd->getAllLetters()){
			this->appendLetter(letter);
		}
	}

	bool equals(Word* otherWord) const{
		size_t amountOfThisLetters = this->word.size();
		size_t amountOfLetters = otherWord->word.size();
		if (amountOfThisLetters != amountOfLetters) return false;
		for (size_t i = 0; i < amountOfLetters; i++){
			if(!this->word[i]->equals(otherWord->word[i])){
				return false;
			}
		}
		return true;
	}

};

class Production {
private:
	std::unique_ptr<Word> from;
	std::unique_ptr<Word> to;
public:
	Production(std::unique_ptr<Word> from, std::unique_ptr<Word> to){
		this->from = std::move(from);
		this->to = std::move(to);
	}

	void print() const{
		this->from->print();
		printf(" -> ");
		this->to->print();
		printf(";\n");
	}

	bool canApplyRight(const Word* word) const{
		if (word->endsWith(this->from.get())){
			return true;
		}
		return false;
	}

	// no const here because I modify the word by deleteFromRight and appendRightWord
	void applyRight(Word* word) const{
		
		// is we can apply right then the fiven word is already ended by the 'from'
		// and all that we need to do is to change the suffix 'from' to 'to'
		if(this->canApplyRight(word)){
			int sizeToDelete = this->from->getLength();
			word->deleteFromRight(sizeToDelete);
			word->appendRightWord(this->to.get());
		}
	}

	Word* getFrom() const {
		return this->from.get();
	}

	Word* getTo() const {
		return this->to.get();
	}

	std::unique_ptr<Word> applyRightNew(const Word* oldWord) const{
		std::unique_ptr<Word> newWord = std::make_unique<Word>(*oldWord);
		this->applyRight(newWord.get());
		return newWord;

	}
};

class Grammar {
private:
	std::vector<std::unique_ptr<Terminal>> terminals;
	std::vector<std::unique_ptr<NonTerminal>> nonTerminals;
	std::vector<std::unique_ptr<Production>> productions;
	const NonTerminal* startingSymbol; // pointer to some nonTerminal from nonTerminals vector set
public:
	// the Constructor in previous version has been accepting the starting symbol. So, for now just use this.
	Grammar(std::unique_ptr<NonTerminal> startingSymbol){
		this->startingSymbol = startingSymbol.get(); // raw borrow pointer, consider return here later if reallocating happens through vector but this pointer will be still old
		nonTerminals.push_back(std::move(startingSymbol));
	}

	// Expand Right is an operation that returns the set of all possible expansions from current word to another word.
	std::vector<std::unique_ptr<Word>> getPossibleExpandRight(const Word* currentWord) const{
		std::vector<std::unique_ptr<Word>> resultingWords;
		for (const auto& p : productions){
			if (p->canApplyRight(currentWord)){
				resultingWords.push_back(p->applyRightNew(currentWord));

			}
			// will return later soon
		}
		return resultingWords;
	}

	void addNonTerminal(std::unique_ptr<NonTerminal> nonTermLetter){
		this->nonTerminals.push_back(std::move(nonTermLetter));
	}

	void addTerminal(std::unique_ptr<Terminal> TermLetter){
		this->terminals.push_back(std::move(TermLetter));
	}
	void addProduction(std::unique_ptr<Production> production){
		this->productions.push_back(std::move(production));
	}
	std::vector<const Production*> getProductions() const{
		std::vector<const Production*> result;
		result.reserve(this->terminals.size());

		for (const std::unique_ptr<Production>& pr : this->productions){
			result.push_back(pr.get());
		}

		return result;
	}

	const NonTerminal* getStartingSymbol() const {
		return startingSymbol;
	}

	std::vector<const NonTerminal*> getNonTerminals() const {
		std::vector<const NonTerminal*> result;
		result.reserve(this->nonTerminals.size());

		for (const std::unique_ptr<NonTerminal>& nt : this->nonTerminals){
			result.push_back(nt.get());
		}

		return result;
	}

	std::vector<const Terminal*> getTerminals() const {
		std::vector<const Terminal*> result;
		result.reserve(this->terminals.size());

		for (const std::unique_ptr<Terminal>& t : this->terminals){
			result.push_back(t.get());
		}

		return result;
	}

	std::string getStartingSymbolName() const {
		std::string resultingString = "";
		resultingString += startingSymbol->getLetter();
		return resultingString;
	}
};

class ValidWordGenerator{
private:
	const Grammar* grammar; // points to existing grammar somewhere on the unique_ptr alghorithm

	bool alreadySeen(const Word* word, std::vector<std::unique_ptr<Word>>& seen) const{
		for(auto& w : seen){
			if (word->equals(w.get())){
				return true;
			}
		}
		return false;
	}

public:
	ValidWordGenerator(const Grammar* grammar){
		this->grammar = grammar;
	}
	std::vector<std::unique_ptr<Word>> generate(size_t maxWords = 15){
		std::queue<std::unique_ptr<Word>> queue;
		std::vector<std::unique_ptr<Word>> seen;
		std::vector<std::unique_ptr<Word>> result;

		size_t generated = 0;
		
		std::unique_ptr<Word> startWord = std::make_unique<Word>();
		startWord->appendLetter(this->grammar->getStartingSymbol());
		queue.push(std::move(startWord));

		while(!queue.empty() && generated < maxWords){
			std::unique_ptr<Word> current = std::move(queue.front()); // get the info from queue
			queue.pop(); // delete just getted value from queue
			if (this->alreadySeen(current.get(), seen))
				continue;
			if (current->containsNonTerminal() == false){
				result.push_back(std::make_unique<Word>(*current.get()));
				generated++;
			}
			std::vector<std::unique_ptr<Word>> expansions = this->grammar->getPossibleExpandRight(current.get());
			for (std::unique_ptr<Word>& newWord : expansions)
				queue.push(std::move(newWord));
			seen.push_back(std::move(current));


		}

		return result;
	}

};



// Automaton. Can be finite and non-finite.
// Will be just one Class of Automaton and a bool function to answer the question if this is a finite or not.

// Automaton has States, Alphabet, Transition function (State x alphabet -> state)
class State {
private:
	std::string nameOfState;
public:
	State(std::string name){
		this->nameOfState = name;
	}

	bool isName(std::string name){
		return this->nameOfState == name;
	}
};

class Symbol {
private:
	std::string nameOfSymbol;
public:
	Symbol(std::string name){
		this->nameOfSymbol = name;
	}
	std::string getNameOfSymbol() const{
		return this->nameOfSymbol;
	}
	bool isName(std::string name) const {
		return this->nameOfSymbol == name;
	}
};
class Transition {
private:
	const State* stateFrom;
	const Symbol* viaSymbol;
	const State* stateTo;
public:
	Transition(const State* from, const Symbol* via, const State* to){
		this->stateFrom = from;
		this->viaSymbol = via;
		this->stateTo = to;
	}
	bool isFromStateAndViaSymbol(const State* from,const Symbol* symbol) const {
		if (this->stateFrom == from && this->viaSymbol->getNameOfSymbol() == symbol->getNameOfSymbol())
			return true;
		return false;
	}
	const State* getToState() const{
		return stateTo;
	}
};



class Automaton {
private:
	std::vector<std::unique_ptr<State>> states;
	std::vector<std::unique_ptr<Symbol>> alphabet;
	std::vector<std::unique_ptr<Transition>> transitions;
	const State* startingState;
	std::vector<const State*> finalStates;

	bool containsState(const State* state) const {
		for (const std::unique_ptr<State>& st: this->states){
			if (st.get() == state){
				return true;
			}
		}
		return false;
	}

	bool isFinalState(const State* state) const {
		for (const State* const& st : this->finalStates){
			if (st == state){
				return true;
			}
		}
		return false;
	}

	bool containsSymbol(const Symbol* symbol) const {
		for (const std::unique_ptr<Symbol>& sy : this->alphabet){
			if (sy.get() == symbol) {
				return true;
			}
		}
		return false;
	}

	bool containsTransition(const Transition* transition) const {
		for (const std::unique_ptr<Transition>& tr : this->transitions){
			if (tr.get() == transition) {
				return true;
			}
		}
		return false;
	}

public:
	Automaton(std::unique_ptr<State> start){
		this->startingState = start.get();
		this->states.push_back(std::move(start));
	}
	
	void addState(std::unique_ptr<State> state){
		if(!this->containsState(state.get()))
			this->states.push_back(std::move(state));
	}

	void setStateAsFinal(const State* finalState) {
		if(this->containsState(finalState) && !this->isFinalState(finalState)){
			finalStates.push_back(finalState);
		}
	}

	void addFinalState(std::unique_ptr<State> state) {
		const State* state_ptr = state.get();
		this->addState(std::move(state));
		this->setStateAsFinal(state_ptr);
	}

	void addSymbol(std::unique_ptr<Symbol> symbol) {
		if (!this->containsSymbol(symbol.get()))
			this->alphabet.push_back(std::move(symbol));
	}

	void addTransition(std::unique_ptr<Transition> transition){
		if (!this->containsTransition(transition.get()))
			this->transitions.push_back(std::move(transition));
	}

	bool accepts(const std::vector<std::unique_ptr<Symbol>> word) const{
		std::vector<const State*> currentStates;
		currentStates.push_back(this->startingState);

		for (const std::unique_ptr<Symbol>& symbol : word){
			std::vector<const State*> nextStates;

			for (const State*& s : currentStates){
				for (const std::unique_ptr<Transition>& tr : this->transitions){
					if (tr->isFromStateAndViaSymbol(s, symbol.get())){
						nextStates.push_back(tr->getToState());
					}
				}
			}

			if (nextStates.empty()) return false;
			currentStates = std::move(nextStates);
		}

		for (const State*& st: currentStates){
			if (this->isFinalState(st)){
				return true;
			}
		}

		return false;
	}

	const State* getStateByName(std::string name){
		for (std::unique_ptr<State>& state : this->states){
			if (state->isName(name)){
				return state.get();
			}
		}
		return nullptr;
	}

	const Symbol* getSymbolByName(std::string name){
		for (std::unique_ptr<Symbol>& symbol : this->alphabet){
			if (symbol->isName(name)){
				return symbol.get();
			}
		}
		return nullptr;
	}
};


class ConverterGrammarToAutomaton{
public:
	std::unique_ptr<Automaton> convert(const Grammar* grammar) const{
		// Create final state
		std::unique_ptr<State> FINAL = std::make_unique<State>("__FINAL__");
		const State* finalState = FINAL.get();
		// create Starting state
		std::unique_ptr<State> startingState = std::make_unique<State>(grammar->getStartingSymbolName());

		// create Automaton itself (and parsing the starting state)
		std::unique_ptr<Automaton> automaton = std::make_unique<Automaton>(std::move(startingState));
		// parse the final state;
		automaton->addFinalState(std::move(FINAL));

		std::vector<const NonTerminal*> nonTerminals = grammar->getNonTerminals();


		// NonTerminals are the States;
		for (const NonTerminal* nt : nonTerminals){
			std::string st = "";
			st += nt->getLetter();
			std::unique_ptr<State> stat = std::make_unique<State>(st);
			automaton->addState(std::move(stat));
			//add each nt to states as a state. create new state each time and call function addState();
		}

		// Terminals are alphabet, nullptr = epsilpn.
		std::vector<const Terminal*> terminals = grammar->getTerminals();
		for(const Terminal* t : terminals){
			std::string st = "";
			st += t->getLetter();
			std::unique_ptr<Symbol> sy = std::make_unique<Symbol>(st);
			automaton->addSymbol(std::move(sy));
		}


		std::vector<const Production*> productions = grammar->getProductions();
		// Now transform productions to transitions;
		for (const Production* p : productions) {
			// get first letter from FROM
			const Word* wordFrom = p->getFrom();
			const AbstractLetter* al = wordFrom->getAllLetters()[0];
			const char fromChar = al->getLetter();

			std::string fromString = "";
			fromString += fromChar;
			const State* fromState = automaton->getStateByName(fromString);

			const Word* wordTo = p->getTo();
			if (wordTo->getLength() == 1 && wordTo->getAllLetters()[0]->isTerminal()){
				// format A->a
				char viaChar = wordTo->getAllLetters()[0]->getLetter();
				std::string viaString = "";
				viaString += viaChar;
				const Symbol* symbolVia = automaton->getSymbolByName(viaString);
				const State* toState = finalState;
				std::unique_ptr<Transition> transition= std::make_unique<Transition>(fromState, symbolVia, toState);
				automaton->addTransition(std::move(transition));
			}

			if (wordTo->getLength() == 2){
				// format A->aB
				char viaChar = wordTo->getAllLetters()[0]->getLetter();
				std::string viaString = "";
				viaString += viaChar;
				const Symbol* symbolVia = automaton->getSymbolByName(viaString);


				char toChar = wordTo->getAllLetters()[1]->getLetter();
				std::string toString = "";
				toString += toChar;
				const State* toState = automaton->getStateByName(toString);
				
				std::unique_ptr<Transition> transition= std::make_unique<Transition>(fromState, symbolVia, toState);
				automaton->addTransition(std::move(transition));
			}
			
		}

		return automaton;
	}
};

// TODO:
// 1) FA -> Regular Grammar
// 2) determine if FA is DFA or NFA
// 3) NDFA -> DFA
// 4) Graphically represent FA

// my variant:
/*
Variant 20
Q = {q0,q1,q2,q3},
∑ = {a,b,c},
F = {q3},
δ(q0,a) = q0,
δ(q0,a) = q1,
δ(q2,a) = q2,
δ(q1,b) = q2,
δ(q2,c) = q3,
δ(q3,c) = q3.
*/

int main() {
	// ----------------------------
	// Create grammar: S -> aS | b
	// ----------------------------

	std::unique_ptr<NonTerminal> S = std::make_unique<NonTerminal>('S');
	Grammar grammar(std::move(S));

	grammar.addTerminal(std::make_unique<Terminal>('a'));
	grammar.addTerminal(std::make_unique<Terminal>('b'));

	// S -> aS
	{
		std::unique_ptr<Word> from = std::make_unique<Word>();
		from->appendLetter(grammar.getStartingSymbol());

		std::unique_ptr<Word> to = std::make_unique<Word>();
		to->appendLetter(grammar.getTerminals()[0]);  // 'a'
		to->appendLetter(grammar.getStartingSymbol());

		grammar.addProduction(
			std::make_unique<Production>(std::move(from), std::move(to))
		);
	}

	// S -> b
	{
		std::unique_ptr<Word> from = std::make_unique<Word>();
		from->appendLetter(grammar.getStartingSymbol());

		std::unique_ptr<Word> to = std::make_unique<Word>();
		to->appendLetter(grammar.getTerminals()[1]);  // 'b'

		grammar.addProduction(
			std::make_unique<Production>(std::move(from), std::move(to))
		);
	}

	// ----------------------------
	// Generate few words
	// ----------------------------

	printf("Generated words:\n");

	ValidWordGenerator gen(&grammar);
	auto words = gen.generate(6);

	for (const auto& w : words) {
		w->print();
		printf("\n");
	}

	// ----------------------------
	// Convert grammar → automaton
	// ----------------------------

	ConverterGrammarToAutomaton converter;
	std::unique_ptr<Automaton> automaton = converter.convert(&grammar);

	// ----------------------------
	// Test automaton on: ab
	// ----------------------------

	std::vector<std::unique_ptr<Symbol>> testWord;
	testWord.push_back(std::make_unique<Symbol>("a"));
	testWord.push_back(std::make_unique<Symbol>("b"));

	bool accepted = automaton->accepts(std::move(testWord));

	printf("\nTest word: ab -> %s\n", accepted ? "ACCEPT" : "REJECT");

	return 0;
}

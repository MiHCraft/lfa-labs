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
#include <cmath>
#include <map>
#include <raylib.h>

// compile commamd, use the last version of c++
// g++ main.cpp -std=c++23 -o main.out -I/opt/homebrew/include -L/opt/homebrew/lib -lraylib

// We use an OOP style when all the classes store other objects (aka. composition) by pointers. And all the objects are stoted in RAM. Why this choice? Because it is easy for me to write.


// The grammar has nonterminal symbols, terminal symbols, rule of productions and a starting symbol.
// so, it has symbols and a productions.
// while the production is a set of 2 words. from the what word we go to what word.
//
// Firstly, we have a letter. A Letter is an abstract class, as there are only 2 types of Letters: terminal and non-terminal.
class AbstractLetter {
protected:
	std::string letter;
public:
	AbstractLetter(std::string letter){
		this->letter = letter;
	}

	virtual ~AbstractLetter() {}

	void setLetter(std::string letter) {
		this->letter = letter;
	}

	std::string getLetter() const {
		return this->letter;
	}

	void print() const {
		std::cout << letter;
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
	Terminal(std::string letter) : AbstractLetter(letter) {}

	bool isTerminal() const override {
		return true;
	}
};

class NonTerminal : public AbstractLetter {
public:
	NonTerminal(std::string letter) : AbstractLetter(letter) {}

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
		std::cout << " -> ";
		this->to->print();
		std::cout << ";\n";
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


	//lookups
	const NonTerminal* getNonTerminalByLetter(const std::string& letter) const {
		for (const auto& nt : nonTerminals) {
			if (nt->getLetter() == letter) {
				return nt.get();
			}
		}
		return nullptr;
	}

	const Terminal* getTerminalByLetter(const std::string& letter) const {
		for (const auto& t : terminals) {
			if (t->getLetter() == letter) {
				return t.get();
			}
		}
		return nullptr;
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

	std::string getName() const {
        return nameOfState;
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
	
	const State* getFromState() const {
		return stateFrom;
	}

	const Symbol* getSymbol() const {
		return viaSymbol;
	}

	const State* getToState() const{
		return stateTo;
	}
};



class AbstractAutomaton {
protected:
	std::vector<std::unique_ptr<State>> states;
	std::vector<std::unique_ptr<Symbol>> alphabet;
	std::vector<std::unique_ptr<Transition>> transitions;
	const State* startingState = nullptr;
	std::vector<const State*> finalStates;

	// helper methods
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
	AbstractAutomaton() = default;
	explicit AbstractAutomaton(std::unique_ptr<State> start){
		this->startingState = start.get();
		this->states.push_back(std::move(start));
	}

	virtual ~AbstractAutomaton() = default;

	void addState(std::unique_ptr<State> state);
	void setStateAsFinal(const State* finalState);
	void addFinalState(std::unique_ptr<State> state);
	void addSymbol(std::unique_ptr<Symbol> symbol);
	virtual void addTransition(std::unique_ptr<Transition> transition);
	const State* getStateByName(std::string name) const;
	const Symbol* getSymbolByName(std::string name);
	
	virtual bool accepts(const std::vector<std::unique_ptr<Symbol>> word) const = 0;

	const State* getStartingState() const {
		return startingState;
	}

	std::vector<const State*> getStates() const {
		std::vector<const State*> result;
		for (const auto& s : states)
			result.push_back(s.get());
		return result;
	}

	std::vector<const Symbol*> getAlphabet() const {
		std::vector<const Symbol*> result;
		for (const auto& s : alphabet)
			result.push_back(s.get());
		return result;
	}

	std::vector<const Transition*> getTransitions() const {
		std::vector<const Transition*> result;
		for (const auto& t : transitions)
			result.push_back(t.get());
		return result;
	}

	std::vector<const State*> getFinalStates() const {
		return finalStates;
	}



};

void AbstractAutomaton::addState(std::unique_ptr<State> state) {
	if(!this->containsState(state.get()))
		this->states.push_back(std::move(state));
}

void AbstractAutomaton::setStateAsFinal(const State* finalState) {
	if(this->containsState(finalState) && !this->isFinalState(finalState)){
		finalStates.push_back(finalState);
	}
}

void AbstractAutomaton::addFinalState(std::unique_ptr<State> state) {
	const State* state_ptr = state.get();
	this->addState(std::move(state));
	this->setStateAsFinal(state_ptr);
}

void AbstractAutomaton::addSymbol(std::unique_ptr<Symbol> symbol) {
	if (!this->containsSymbol(symbol.get()))
		this->alphabet.push_back(std::move(symbol));
}

void AbstractAutomaton::addTransition(std::unique_ptr<Transition> transition){
	if (!this->containsTransition(transition.get()))
		this->transitions.push_back(std::move(transition));
}

const State* AbstractAutomaton::getStateByName(std::string name) const{
	for (const std::unique_ptr<State>& state : this->states){
		if (state->isName(name)){
			return state.get();
		}
	}
	return nullptr;
}

const Symbol* AbstractAutomaton::getSymbolByName(std::string name){
	for (std::unique_ptr<Symbol>& symbol : this->alphabet){
		if (symbol->isName(name)){
			return symbol.get();
		}
	}
	return nullptr;
}

class NonDeterministicAutomaton : public AbstractAutomaton{
public:
	explicit NonDeterministicAutomaton(std::unique_ptr<State> start)
        : AbstractAutomaton(std::move(start)) 
    {
    }

	bool isDeterministic() const {
		for (const auto& st : states) {
			for (const auto& sym : alphabet) {
				std::set<const State*> targets;
				for (const auto& tr : transitions) {
					if (tr->isFromStateAndViaSymbol(st.get(), sym.get())) {
						targets.insert(tr->getToState());
					}
				}
				if (targets.size() > 1) {
					return false;
				}
			}
		}
		return true;
	}

	bool accepts(const std::vector<std::unique_ptr<Symbol>> word) const override{
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
};









class ConverterGrammarToAutomaton{
public:
	std::unique_ptr<NonDeterministicAutomaton> convert(const Grammar* grammar) const{
		// Create final state
		std::unique_ptr<State> FINAL = std::make_unique<State>("__FINAL__");
		const State* finalState = FINAL.get();
		// create Starting state
		std::unique_ptr<State> startingState = std::make_unique<State>(grammar->getStartingSymbolName());

		// create Automaton itself (and parsing the starting state)
		std::unique_ptr<NonDeterministicAutomaton> automaton = std::make_unique<NonDeterministicAutomaton>(std::move(startingState));
		// parse the final state;
		
		automaton->addFinalState(std::move(FINAL));

		std::vector<const NonTerminal*> nonTerminals = grammar->getNonTerminals();


		// NonTerminals are the States;
		for (const NonTerminal* nt : nonTerminals){
			std::string st = nt->getLetter();

			// starting state already exist
			if (st == grammar->getStartingSymbolName())
				continue;

			std::unique_ptr<State> stat = std::make_unique<State>(st);
			automaton->addState(std::move(stat));
			//add each nt to states as a state. create new state each time and call function addState();
		}

		// Terminals are alphabet, nullptr = epsilpn.
		std::vector<const Terminal*> terminals = grammar->getTerminals();
		for(const Terminal* t : terminals){
			std::string st = t->getLetter();
			std::unique_ptr<Symbol> sy = std::make_unique<Symbol>(st);
			automaton->addSymbol(std::move(sy));
		}


		std::vector<const Production*> productions = grammar->getProductions();
		// Now transform productions to transitions;
		for (const Production* p : productions) {
			// get first letter from FROM
			const Word* wordFrom = p->getFrom();
			const AbstractLetter* al = wordFrom->getAllLetters()[0];
			const std::string fromString = al->getLetter();
			
			const State* fromState = automaton->getStateByName(fromString);

			const Word* wordTo = p->getTo();
			if (wordTo->getLength() == 1 && wordTo->getAllLetters()[0]->isTerminal()){
				// format A->a
				std::string viaString = wordTo->getAllLetters()[0]->getLetter();
				const Symbol* symbolVia = automaton->getSymbolByName(viaString);
				const State* toState = finalState;
				std::unique_ptr<Transition> transition= std::make_unique<Transition>(fromState, symbolVia, toState);
				automaton->addTransition(std::move(transition));
			}

			if (wordTo->getLength() == 2){
				// format A->aB
				std::string viaString = wordTo->getAllLetters()[0]->getLetter();
				const Symbol* symbolVia = automaton->getSymbolByName(viaString);


				std::string toString = wordTo->getAllLetters()[1]->getLetter();
				const State* toState = automaton->getStateByName(toString);
				
				std::unique_ptr<Transition> transition= std::make_unique<Transition>(fromState, symbolVia, toState);
				automaton->addTransition(std::move(transition));
			}

			if (wordTo->getLength() == 0){
				// A -> ε
				automaton->setStateAsFinal(fromState);
				continue;
			}
			
		}

		return automaton;
	}
};

class DeterministicAutomaton : public AbstractAutomaton {
public:

    explicit DeterministicAutomaton(std::unique_ptr<State> start)
        : AbstractAutomaton(std::move(start))
    {}

    void addTransition(std::unique_ptr<Transition> transition) override {

        const State* from = transition->getFromState();
        const Symbol* sym = transition->getSymbol();

        for (const auto& tr : this->transitions) {
            if (tr->isFromStateAndViaSymbol(from, sym)) {
                printf("ERROR: DFA cannot have multiple transitions for same state and symbol\n");
                std::abort();
            }
        }

        this->transitions.push_back(std::move(transition));
    }

    bool accepts(const std::vector<std::unique_ptr<Symbol>> word) const override {

        const State* current = this->startingState;

        for (const auto& symbol : word) {

            const State* next = nullptr;

            for (const auto& tr : this->transitions) {
                if (tr->isFromStateAndViaSymbol(current, symbol.get())) {
                    next = tr->getToState();
                    break;
                }
            }

            if (next == nullptr)
                return false;

            current = next;
        }

        return this->isFinalState(current);
    }
};

class ConverterAutomatonToGrammar {
public:
    std::unique_ptr<Grammar> convert(const AbstractAutomaton* automaton) const {
        // Create starting non-terminal from starting state
        std::string startName = automaton->getStartingState()->getName();
        std::unique_ptr<NonTerminal> startNT = std::make_unique<NonTerminal>(startName);
        std::unique_ptr<Grammar> grammar = std::make_unique<Grammar>(std::move(startNT));

        // Add non-terminals for all other states
        for (const auto& st : automaton->getStates()) {
            std::string stName = st->getName();
            if (stName != startName) {
                grammar->addNonTerminal(std::make_unique<NonTerminal>(stName));
            }
        }

        // Add terminals for alphabet
        for (const auto& sym : automaton->getAlphabet()) {
            grammar->addTerminal(std::make_unique<Terminal>(sym->getNameOfSymbol()));
        }

        // Add productions for transitions (Q -> a P)
        for (const auto& tr : automaton->getTransitions()) {
            std::unique_ptr<Word> fromW = std::make_unique<Word>();
            std::string fromName = tr->getFromState()->getName();
            const NonTerminal* fromNT = grammar->getNonTerminalByLetter(fromName);
            fromW->appendLetter(fromNT);

            std::unique_ptr<Word> toW = std::make_unique<Word>();
            std::string symName = tr->getSymbol()->getNameOfSymbol();
            const Terminal* symT = grammar->getTerminalByLetter(symName);
            toW->appendLetter(symT);
            std::string toName = tr->getToState()->getName();
            const NonTerminal* toNT = grammar->getNonTerminalByLetter(toName);
            toW->appendLetter(toNT);

            grammar->addProduction(std::make_unique<Production>(std::move(fromW), std::move(toW)));
        }

        // Add epsilon productions for final states (F -> ε)
        for (const State* f : automaton->getFinalStates()) {
            std::unique_ptr<Word> fromW = std::make_unique<Word>();
            std::string fName = f->getName();
            const NonTerminal* fNT = grammar->getNonTerminalByLetter(fName);
            fromW->appendLetter(fNT);

            std::unique_ptr<Word> toW = std::make_unique<Word>();  // empty for epsilon

            grammar->addProduction(std::make_unique<Production>(std::move(fromW), std::move(toW)));
        }

        return grammar;
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

    std::set<const State*> move(
        const NonDeterministicAutomaton* nfa,
        const std::set<const State*>& states,
        const Symbol* symbol
    ) const {

        std::set<const State*> result;

        for (const State* st : states) {
            for (const Transition* tr : nfa->getTransitions()) {
                if (tr->isFromStateAndViaSymbol(st, symbol)) {
                    result.insert(tr->getToState());
                }
            }
        }

        return result;
    }

public:

    std::unique_ptr<DeterministicAutomaton>
    convert(const NonDeterministicAutomaton* nfa) const {

        // starting subset
        std::set<const State*> startSet;
        startSet.insert(nfa->getStartingState());

        std::string startName = makeStateName(startSet);

        std::unique_ptr<State> startState =
            std::make_unique<State>(startName);

        std::unique_ptr<DeterministicAutomaton> dfa =
            std::make_unique<DeterministicAutomaton>(std::move(startState));

        // copy alphabet
        for (const Symbol* s : nfa->getAlphabet()) {
            dfa->addSymbol(std::make_unique<Symbol>(s->getNameOfSymbol()));
        }

        std::queue<std::set<const State*>> queue;
        std::set<std::string> visited;

        queue.push(startSet);
        visited.insert(startName);

        while (!queue.empty()) {

            std::set<const State*> currentSet = queue.front();
            queue.pop();

            std::string currentName = makeStateName(currentSet);
            const State* currentState = dfa->getStateByName(currentName);

            // check if final
            for (const State* s : currentSet) {
                for (const State* f : nfa->getFinalStates()) {
                    if (s == f) {
                        dfa->setStateAsFinal(currentState);
                    }
                }
            }

            // transitions
            for (const Symbol* sym : nfa->getAlphabet()) {

                std::set<const State*> nextSet = move(nfa, currentSet, sym);

                if (nextSet.empty())
                    continue;

                std::string nextName = makeStateName(nextSet);

                const State* nextState = dfa->getStateByName(nextName);

                if (nextState == nullptr) {

                    std::unique_ptr<State> newState =
                        std::make_unique<State>(nextName);

                    nextState = newState.get();

                    dfa->addState(std::move(newState));

                    if (!visited.count(nextName)) {
                        queue.push(nextSet);
                        visited.insert(nextName);
                    }
                }

                const Symbol* dfaSym =
                    dfa->getSymbolByName(sym->getNameOfSymbol());

                std::unique_ptr<Transition> tr =
                    std::make_unique<Transition>(currentState, dfaSym, nextState);

                dfa->addTransition(std::move(tr));
            }
        }

        return dfa;
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

void printAutomaton(const AbstractAutomaton* a){
    printf("\nStates:\n");
    for (const State* s : a->getStates()){
        printf("%s\n", s->getName().c_str());
    }

    printf("\nAlphabet:\n");
    for (const Symbol* s : a->getAlphabet()){
        printf("%s\n", s->getNameOfSymbol().c_str());
    }

    printf("\nTransitions:\n");
    for (const Transition* t : a->getTransitions()){
        printf("δ(%s,%s) -> %s\n",
            t->getFromState()->getName().c_str(),
            t->getSymbol()->getNameOfSymbol().c_str(),
            t->getToState()->getName().c_str());
    }

    printf("\nStart: %s\n", a->getStartingState()->getName().c_str());

    printf("\nFinal states:\n");
    for (const State* s : a->getFinalStates()){
        printf("%s\n", s->getName().c_str());
    }
}


void printGrammar(const Grammar* g){

    printf("\nNonTerminals:\n");
    for (auto nt : g->getNonTerminals()){
        printf("%s\n", nt->getLetter().c_str());
    }

    printf("\nTerminals:\n");
    for (auto t : g->getTerminals()){
        printf("%s\n", t->getLetter().c_str());
    }

    printf("\nProductions:\n");
    for (auto p : g->getProductions()){
        p->print();
    }

    printf("\nStart symbol: %s\n", g->getStartingSymbol()->getLetter().c_str());
}

void printGeneratedWords(const Grammar* g, int amount = 10){

    printf("\nGenerated words from grammar:\n");

    ValidWordGenerator generator(g);

    std::vector<std::unique_ptr<Word>> words = generator.generate(amount);

    int i = 1;
    for (const auto& w : words){
        printf("%d: ", i++);
        w->print();
        printf("\n");
    }

}

std::vector<std::unique_ptr<Word>> generateWords(const Grammar* g, int amount = 10){

    printf("\nGenerated words from grammar:\n");

    ValidWordGenerator generator(g);
    std::vector<std::unique_ptr<Word>> words = generator.generate(amount);

    int i = 1;
    for (const auto& w : words){
        printf("%d: ", i++);
        w->print();
        printf("\n");
    }

    return words;
}


void testWordsOnAutomaton(
    const AbstractAutomaton* automaton,
    const std::vector<std::unique_ptr<Word>>& words
){

    printf("\nTesting words on automaton:\n");

    for (const auto& w : words){

        std::vector<std::unique_ptr<Symbol>> input;

        for (const auto& letter : w->getAllLetters()){
            input.push_back(std::make_unique<Symbol>(letter->getLetter()));
        }

        bool accepted = automaton->accepts(std::move(input));

        w->print();
        printf(" -> %s\n", accepted ? "ACCEPT" : "REJECT");
    }
}

class Drawing {

private:

    int width;
    int height;

    struct Vec {
        float x;
        float y;
    };

    std::map<const State*, Vec> layoutStates(const AbstractAutomaton* automaton, float xOffset)
    {
        std::map<const State*, Vec> pos;

        auto states = automaton->getStates();

        float radius = 200.0f;
        float cx = xOffset;
        float cy = height / 2.0f - 50;   // a bit higher to leave space for tables

        int n = states.size();
        int i = 0;

        for (const State* st : states)
        {
            float angle = 2 * 3.14159265f * i / n;

            float x = cx + cos(angle) * radius;
            float y = cy + sin(angle) * radius;

            pos[st] = {x, y};
            i++;
        }

        return pos;
    }

    bool isFinal(const AbstractAutomaton* a, const State* s)
    {
        for (const State* f : a->getFinalStates())
            if (f == s)
                return true;
        return false;
    }

    void drawState(const State* state, Vec pos, bool isStart, bool isFinal)
    {
        float r = 30.0f;

        DrawCircleLines(pos.x, pos.y, r, BLACK);
        if (isFinal)
            DrawCircleLines(pos.x, pos.y, r - 6, BLACK);

        DrawText(state->getName().c_str(), pos.x - 12, pos.y - 10, 20, BLACK);

        if (isStart)
        {
            DrawTriangle({pos.x - 50, pos.y}, {pos.x - 35, pos.y - 10}, {pos.x - 35, pos.y + 10}, BLACK);
        }
    }

    void drawTransition(Vec from, Vec to, const std::string& label, bool selfLoop)
    {
        if (selfLoop)
        {
            DrawCircleLines(from.x, from.y - 55, 20, DARKGRAY);
            DrawText(label.c_str(), from.x - 8, from.y - 85, 20, BLACK);
            DrawTriangle({from.x + 10, from.y - 75}, {from.x + 20, from.y - 65}, {from.x, from.y - 65}, DARKGRAY);
            return;
        }

        Vector2 p1 = {from.x, from.y};
        Vector2 p2 = {to.x,   to.y};

        DrawLineEx(p1, p2, 4.0f, DARKGRAY);

        float dx = p2.x - p1.x;
        float dy = p2.y - p1.y;
        float len = sqrt(dx * dx + dy * dy);
        if (len == 0) return;

        dx /= len;
        dy /= len;

        float inset = 35.0f;
        Vector2 head = { p2.x - dx * inset, p2.y - dy * inset };

        DrawCircle(head.x, head.y, 8, RED);
        DrawCircleLines(head.x, head.y, 8, BLACK);

        float midx = (p1.x + p2.x) / 2.0f - 8;
        float midy = (p1.y + p2.y) / 2.0f - 15;
        DrawText(label.c_str(), (int)midx, (int)midy, 20, BLACK);
    }

    // ===================================================================
    // NEW: Draws a nice transition table (works for both NFA and DFA)
    // ===================================================================
    void drawTransitionTable(const AbstractAutomaton* aut, float tableX, float tableY, const char* title)
    {
        float cellW = 105.0f;
        float cellH = 38.0f;

        // Collect transitions (NFA can have multiple, DFA has one)
        std::map<std::string, std::map<std::string, std::string>> tableData;
        for (const auto& tr : aut->getTransitions())
        {
            std::string from = tr->getFromState()->getName();
            std::string sym  = tr->getSymbol()->getNameOfSymbol();
            std::string to   = tr->getToState()->getName();

            if (tableData[from][sym].empty())
                tableData[from][sym] = to;
            else
                tableData[from][sym] += "," + to;
        }

        auto states   = aut->getStates();
        auto symbols  = aut->getAlphabet();
        int numCols   = symbols.size() + 1;
        float totalW  = numCols * cellW;
        float totalH  = (states.size() + 1) * cellH;

        // Title
        DrawText(title, tableX + 20, tableY - 40, 24, BLACK);

        // === DRAW GRID ===
        // vertical lines
        for (int c = 0; c <= numCols; c++)
        {
            float lx = tableX + c * cellW;
            DrawLine(lx, tableY, lx, tableY + totalH, BLACK);
        }
        // horizontal lines
        for (int r = 0; r <= states.size() + 1; r++)
        {
            float ly = tableY + r * cellH;
            DrawLine(tableX, ly, tableX + totalW, ly, BLACK);
        }

        // === HEADER ===
        DrawText("State", tableX + 12, tableY + 8, 20, BLACK);
        float colX = tableX + cellW;
        for (const auto& sy : symbols)
        {
            DrawText(sy->getNameOfSymbol().c_str(), colX + 25, tableY + 8, 20, BLACK);
            colX += cellW;
        }

        // === ROWS ===
        int row = 1;
        for (const State* st : states)
        {
            std::string sName = st->getName();
            float rowY = tableY + row * cellH + 8;

            // State name with start/final markers
            std::string label = sName;
            if (st == aut->getStartingState()) label = "→" + label;
            bool final = false;
            for (const State* f : aut->getFinalStates())
                if (f == st) { final = true; break; }
            if (final) label += "*";

            DrawText(label.c_str(), tableX + 12, rowY, 20, BLACK);

            // Transition cells
            colX = tableX + cellW + 10;
            for (const auto& sy : symbols)
            {
                std::string symName = sy->getNameOfSymbol();
                std::string targets = tableData[sName][symName];
                if (targets.empty()) targets = "{}";

                DrawText(targets.c_str(), colX, rowY, 20, BLACK);
                colX += cellW;
            }
            row++;
        }
    }

public:

    Drawing(int w = 1400, int h = 1000)
    {
        width = w;
        height = h;
    }

    void drawAutomata(
        const AbstractAutomaton* nfa,
        const AbstractAutomaton* dfa
    )
    {
        InitWindow(width, height, "NFA vs DFA - Graphs + Transition Tables");
        SetTargetFPS(60);

        auto posNFA = layoutStates(nfa, width * 0.25f);
        auto posDFA = layoutStates(dfa, width * 0.75f);

        while (!WindowShouldClose())
        {
            BeginDrawing();
            ClearBackground(RAYWHITE);

            // ====================== TITLES ======================
            DrawText("NFA (original)", width * 0.20f - 80, 15, 30, BLACK);
            DrawText("DFA (converted)", width * 0.75f - 95, 15, 30, BLACK);

            // ====================== NFA GRAPH (left) ======================
            for (const auto& tr : nfa->getTransitions())
            {
                Vec from = posNFA[tr->getFromState()];
                Vec to   = posNFA[tr->getToState()];
                bool loop = tr->getFromState() == tr->getToState();
                drawTransition(from, to, tr->getSymbol()->getNameOfSymbol(), loop);
            }
            for (const State* st : nfa->getStates())
            {
                drawState(st, posNFA[st],
                          st == nfa->getStartingState(),
                          isFinal(nfa, st));
            }

            // ====================== DFA GRAPH (right) ======================
            for (const auto& tr : dfa->getTransitions())
            {
                Vec from = posDFA[tr->getFromState()];
                Vec to   = posDFA[tr->getToState()];
                bool loop = tr->getFromState() == tr->getToState();
                drawTransition(from, to, tr->getSymbol()->getNameOfSymbol(), loop);
            }
            for (const State* st : dfa->getStates())
            {
                drawState(st, posDFA[st],
                          st == dfa->getStartingState(),
                          isFinal(dfa, st));
            }

            // ====================== TABLES (bottom) ======================
            drawTransitionTable(nfa, 40,  780, "NFA Transition Table");
            drawTransitionTable(dfa, width * 0.5f + 30, 780, "DFA Transition Table");

            EndDrawing();
        }

        CloseWindow();
    }
};
int main(){

    printf("====================================\n");
    printf("Creating NFA\n");
    printf("====================================\n");

    std::unique_ptr<State> q0 = std::make_unique<State>("q0");

    std::unique_ptr<NonDeterministicAutomaton> nfa =
        std::make_unique<NonDeterministicAutomaton>(std::move(q0));

    nfa->addState(std::make_unique<State>("q1"));
    nfa->addState(std::make_unique<State>("q2"));
    nfa->addState(std::make_unique<State>("q3"));

    nfa->addSymbol(std::make_unique<Symbol>("a"));
    nfa->addSymbol(std::make_unique<Symbol>("b"));
    nfa->addSymbol(std::make_unique<Symbol>("c"));

    const State* q3 = nfa->getStateByName("q3");
    nfa->setStateAsFinal(q3);

    const State* q0p = nfa->getStateByName("q0");
    const State* q1 = nfa->getStateByName("q1");
    const State* q2 = nfa->getStateByName("q2");

    const Symbol* a = nfa->getSymbolByName("a");
    const Symbol* b = nfa->getSymbolByName("b");
    const Symbol* c = nfa->getSymbolByName("c");

    nfa->addTransition(std::make_unique<Transition>(q0p,a,q0p));
    nfa->addTransition(std::make_unique<Transition>(q0p,a,q1));
    nfa->addTransition(std::make_unique<Transition>(q2,a,q2));
    nfa->addTransition(std::make_unique<Transition>(q1,b,q2));
    nfa->addTransition(std::make_unique<Transition>(q2,c,q3));
    nfa->addTransition(std::make_unique<Transition>(q3,c,q3));

    printAutomaton(nfa.get());



    printf("\n====================================\n");
    printf("Checking determinism\n");
    printf("====================================\n");

    printf("Deterministic? -> %s\n",
           nfa->isDeterministic() ? "YES" : "NO");



    printf("\n====================================\n");
    printf("FA -> Grammar\n");
    printf("====================================\n");

    ConverterAutomatonToGrammar fa2grammar;
    std::unique_ptr<Grammar> grammar = fa2grammar.convert(nfa.get());

    printGrammar(grammar.get());



    printf("\n====================================\n");
    printf("Generate words from Grammar\n");
    printf("====================================\n");

    auto words = generateWords(grammar.get(),10);



    printf("\n====================================\n");
    printf("Grammar -> Automaton\n");
    printf("====================================\n");

    ConverterGrammarToAutomaton grammar2fa;

    std::unique_ptr<NonDeterministicAutomaton> autoFromGrammar =
        grammar2fa.convert(grammar.get());

    printAutomaton(autoFromGrammar.get());



    printf("\n====================================\n");
    printf("Testing generated words on Grammar->FA\n");
    printf("====================================\n");

    testWordsOnAutomaton(autoFromGrammar.get(), words);



    printf("\n====================================\n");
    printf("NFA -> DFA\n");
    printf("====================================\n");

    ConverterNFAToDFA converter;

    std::unique_ptr<DeterministicAutomaton> dfa =
        converter.convert(nfa.get());

    printAutomaton(dfa.get());



    printf("\n====================================\n");
    printf("DFA -> Grammar\n");
    printf("====================================\n");

    std::unique_ptr<Grammar> grammar2 =
        fa2grammar.convert(dfa.get());

    printGrammar(grammar2.get());



    printf("\n====================================\n");
    printf("Generate words from DFA Grammar\n");
    printf("====================================\n");

    generateWords(grammar2.get(),10);



    printf("\n====================================\n");
    printf("Program finished successfully\n");
    printf("====================================\n");


	Drawing drawing;

	drawing.drawAutomata(
		nfa.get(),
		dfa.get()
	);



    return 0;
}
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

	const NonTerminal* getStartingSymbol() const{
		return startingSymbol;
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
			if (current->containsNonTerminal() == true){
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




///* ===== Finite Automaton ===== */
//class FiniteAutomaton {
//public:
    //using State = std::string;
//
    //struct Transition {
        //State from;
        //char  symbol;
        //State to;
    //};
//
//private:
    //std::set<State> states;          // Q
    //std::set<char> alphabet;         // Σ
    //std::vector<Transition> delta;   // δ
    //State start;                     // q0
    //std::set<State> finals;          // F
//
//public:
    //FiniteAutomaton(const State& startState)
        //: start(startState)
    //{
        //states.insert(start);
    //}
//
    //void addState(const State& s) {
        //states.insert(s);
    //}
//
    //void addFinalState(const State& s) {
        //states.insert(s);
        //finals.insert(s);
    //}
//
    //void addSymbol(char c) {
        //alphabet.insert(c);
    //}
//
    //void addTransition(const State& from, char symbol, const State& to) {
        //states.insert(from);
        //states.insert(to);
        //alphabet.insert(symbol);
        //delta.push_back({from, symbol, to});
    //}
//
    //const State& getStart() const {
        //return start;
    //}
//
    //const std::set<State>& getStates() const {
        //return states;
    //}
//
    //const std::set<State>& getFinalStates() const {
        //return finals;
    //}
//
    //const std::vector<Transition>& getTransitions() const {
        //return delta;
    //}
//
    //bool accepts(const std::string& word) const {
        //std::set<State> current;
        //current.insert(start);
//
        //for (char c : word) {
            //std::set<State> next;
//
            //for (const auto& s : current) {
                //for (const auto& tr : delta) {
                    //if (tr.from == s && tr.symbol == c) {
                        //next.insert(tr.to);
                    //}
                //}
            //}
//
            //if (next.empty()) return false;
//
            //current = std::move(next);
        //}
//
        //for (const auto& s : current) {
            //if (finals.count(s)) return true;
        //}
//
        //return false;
    //}
//
//
    //void print() const {
        //std::cout << "States: { ";
        //for (const auto& s : states) std::cout << s << " ";
        //std::cout << "}\n";
//
        //std::cout << "Alphabet: { ";
        //for (char c : alphabet) std::cout << c << " ";
        //std::cout << "}\n";
//
        //std::cout << "Start: " << start << "\n";
//
        //std::cout << "Final states: { ";
        //for (const auto& s : finals) std::cout << s << " ";
        //std::cout << "}\n";
//
        //std::cout << "Transitions:\n";
        //for (const auto& t : delta) {
            //std::cout << "  " << t.from << " --" << t.symbol << "--> " << t.to << "\n";
        //}
    //}
//};
//
//
//class Converter {
//
    //public:
        //FiniteAutomaton grammarToFiniteAutomaton(Grammar *g){
            //const std::string FINAL = "__FINAL__";
//
            //FiniteAutomaton fa(g->getStart());
//
            //fa.addFinalState(FINAL);
//
            //for (const auto& p : g->getProductions()) {
//
                //const std::string& from = p.from;
                //const std::string& rhs  = p.to;
//
                //if (rhs.empty()) continue;
//
                //char terminal = rhs[0];
//
                //// Case 1: A -> a
                //if (rhs.size() == 1) {
                    //fa.addTransition(from, terminal, FINAL);
                //}
                //// Case 2: A -> aB
                //else {
                    //std::string to = rhs.substr(1);
                    //fa.addTransition(from, terminal, to);
                //}
            //}
//
            //return fa;
        //}
//};
//

int main() {
    //Grammar g("S");
//
    //// variant 20
    //g.addNonTerminal("S");
    //g.addNonTerminal("A");
    //g.addNonTerminal("B");
    //g.addNonTerminal("C");
//
    //g.addTerminal('a');
    //g.addTerminal('b');
    //g.addTerminal('c');
    //g.addTerminal('d');
//
    //g.addProduction("S", "dA");
    //g.addProduction("A", "d");
    //g.addProduction("A", "aB");
    //g.addProduction("B", "bC");
    //g.addProduction("C", "cA");
    //g.addProduction("C", "aS");
//
    //g.print();
//
    //ValidWordGenerator gen(&g);
//
    //auto words = gen.generate(5);
//
    //for (const auto& w : words) {
        //std::cout << w << "\n";
    //}
//
    //printf("\n\n----\n");
//
    //Converter conv;
    //auto fa = conv.grammarToFiniteAutomaton(&g);
//
    //std::cout << "\n=== Finite Automaton ===\n";
    //fa.print();
//
//
    //printf("\n\n----\n");
    //std::vector<std::string> tests = {
        //"dd",
        //"dabca",
        //"dabcad",
        //"dabcabcad",
        //"abc"
    //};
//
//
//
    //for (const auto& w : tests) {
        //std::cout << w << " -> " 
                //<< (fa.accepts(w) ? "ACCEPTED" : "REJECTED") 
                //<< "\n";
    //}
//
    //printf("\n\n");
    //// should be everywhere ACCEPTED
    //for (const auto& w : words) {
        //std::cout << w << " -> " 
                //<< (fa.accepts(w) ? "ACCEPTED" : "REJECTED") 
                //<< "\n";
    //}
//
//
//
    return 0;
}

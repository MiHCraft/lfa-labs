#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <string>
#include <cctype>

// ------------------------------------------------------------
// Token types for DaQuVa (unique values)
// ------------------------------------------------------------
enum TokenType {
    // Keywords (priority 10)
    TOKEN_MY = 100,
    TOKEN_DB = 101,
    TOKEN_FUNCTION = 102,
    TOKEN_RETURN = 103,
    TOKEN_WHERE = 104,
    TOKEN_AND = 105,
    TOKEN_OR = 106,
    TOKEN_COLUMNS = 107,
    TOKEN_USING = 108,
    TOKEN_TOOL = 109,
    TOKEN_CONSOLE = 110,
    TOKEN_FILE = 111,
    TOKEN_FROM = 112,
    TOKEN_ADDCOLUMNS = 113,
    TOKEN_HERE = 114,
    TOKEN_ALLOWDANGER = 115,
    TOKEN_SCAN = 116,
    TOKEN_FILTER = 117,
    TOKEN_MERGE = 118,
    TOKEN_OUTPUT = 119,
    TOKEN_NEWTABLE = 120,
    TOKEN_FINDLOCALDUPLICATES = 121,

    // Identifiers & literals (priority 5)
    TOKEN_IDENTIFIER = 200,
    TOKEN_NUMBER = 201,
    TOKEN_STRING = 202,

    // Operators & punctuation (priority 1)
    TOKEN_DOT = 300,          // .
    TOKEN_COMMA = 301,        // ,
    TOKEN_SEMICOLON = 302,    // ;
    TOKEN_COLON = 303,        // :
    TOKEN_ARROW = 304,        // ->
    TOKEN_LBRACKET = 305,     // [
    TOKEN_RBRACKET = 306,     // ]
    TOKEN_LPAREN = 307,       // (
    TOKEN_RPAREN = 308,       // )
    TOKEN_LBRACE = 309,       // {   (reserved, not in grammar but for completeness)
    TOKEN_RBRACE = 310,       // }
    TOKEN_ASSIGN = 311,       // =
    TOKEN_EQ = 312,           // ==
    TOKEN_NE = 313,           // !=
    TOKEN_LT = 314,           // <
    TOKEN_LE = 315,           // <=
    TOKEN_GT = 316,           // >
    TOKEN_GE = 317,           // >=

    TOKEN_UNKNOWN = 0,
    TOKEN_EOF = -1
};

// Priority for maximal munch (higher number wins)
int tokenPriority(TokenType t) {
    if (t >= 100 && t < 200) return 10;   // keywords
    if (t >= 200 && t < 300) return 5;    // identifiers, numbers, strings
    return 1;                              // operators & punctuation
}

std::string tokenTypeName(TokenType t) {
    switch (t) {
        case TOKEN_MY: return "MY";
        case TOKEN_DB: return "DB";
        case TOKEN_FUNCTION: return "FUNCTION";
        case TOKEN_RETURN: return "RETURN";
        case TOKEN_WHERE: return "WHERE";
        case TOKEN_AND: return "AND";
        case TOKEN_OR: return "OR";
        case TOKEN_COLUMNS: return "COLUMNS";
        case TOKEN_USING: return "USING";
        case TOKEN_TOOL: return "TOOL";
        case TOKEN_CONSOLE: return "CONSOLE";
        case TOKEN_FILE: return "FILE";
        case TOKEN_FROM: return "FROM";
        case TOKEN_ADDCOLUMNS: return "ADDCOLUMNS";
        case TOKEN_HERE: return "HERE";
        case TOKEN_ALLOWDANGER: return "ALLOWDANGER";
        case TOKEN_SCAN: return "SCAN";
        case TOKEN_FILTER: return "FILTER";
        case TOKEN_MERGE: return "MERGE";
        case TOKEN_OUTPUT: return "OUTPUT";
        case TOKEN_NEWTABLE: return "NEWTABLE";
        case TOKEN_FINDLOCALDUPLICATES: return "FINDLOCALDUPLICATES";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_STRING: return "STRING";
        case TOKEN_DOT: return "DOT";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_COLON: return "COLON";
        case TOKEN_ARROW: return "ARROW";
        case TOKEN_LBRACKET: return "LBRACKET";
        case TOKEN_RBRACKET: return "RBRACKET";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
        case TOKEN_ASSIGN: return "ASSIGN";
        case TOKEN_EQ: return "EQ";
        case TOKEN_NE: return "NE";
        case TOKEN_LT: return "LT";
        case TOKEN_LE: return "LE";
        case TOKEN_GT: return "GT";
        case TOKEN_GE: return "GE";
        default: return "UNKNOWN";
    }
}

// ------------------------------------------------------------
// Finite Automaton class (as before)
// ------------------------------------------------------------
class FiniteAutomaton {
public:
    using State = std::string;

    struct Transition {
        State from;
        char  symbol;
        State to;
    };

private:
    std::set<State> states;
    std::set<char> alphabet;
    std::vector<Transition> delta;
    State start;
    std::set<State> finals;

public:
    FiniteAutomaton(const State& startState) : start(startState) {
        states.insert(start);
    }

    void addState(const State& s) { states.insert(s); }
    void addFinalState(const State& s) { states.insert(s); finals.insert(s); }
    void addSymbol(char c) { alphabet.insert(c); }

    void addTransition(const State& from, char symbol, const State& to) {
        states.insert(from);
        states.insert(to);
        alphabet.insert(symbol);
        delta.push_back({from, symbol, to});
    }

    const State& getStart() const { return start; }
    const std::set<State>& getStates() const { return states; }
    const std::set<State>& getFinalStates() const { return finals; }
    const std::vector<Transition>& getTransitions() const { return delta; }

    // For debugging
    void print() const {
        std::cout << "States: { ";
        for (const auto& s : states) std::cout << s << " ";
        std::cout << "}\nAlphabet: { ";
        for (char c : alphabet) std::cout << c << " ";
        std::cout << "}\nStart: " << start << "\nFinal states: { ";
        for (const auto& s : finals) std::cout << s << " ";
        std::cout << "}\nTransitions:\n";
        for (const auto& t : delta)
            std::cout << "  " << t.from << " --" << t.symbol << "--> " << t.to << "\n";
    }
};

// ------------------------------------------------------------
// Lexer using the finite automaton
// ------------------------------------------------------------
class Lexer {
public:
    Lexer(const FiniteAutomaton& automaton, const std::map<std::string, TokenType>& finalToToken)
        : fa(automaton), finalToToken(finalToToken) {}

    // Tokenise the entire input string, printing tokens with line/column
    void tokenise(const std::string& input) {
        size_t i = 0;
        size_t line = 1, col = 1;
        while (i < input.length()) {
            // Skip whitespace (except newline, which we handle separately)
            while (i < input.length() && (input[i] == ' ' || input[i] == '\t')) {
                ++i; ++col;
            }
            if (i >= input.length()) break;

            // Handle newlines
            if (input[i] == '\n') {
                ++line; col = 1; ++i;
                continue;
            }

            // Handle comments: skip until end of line
            if (input[i] == '#') {
                while (i < input.length() && input[i] != '\n') {
                    ++i; ++col;
                }
                continue;
            }

            // Start of a potential token
            std::set<std::string> currentStates = { fa.getStart() };
            size_t lastAcceptPos = i;
            TokenType lastAcceptType = TOKEN_UNKNOWN;
            size_t lastAcceptLen = 0;
            size_t lastAcceptCol = col;

            size_t j = i;
            size_t currentCol = col;
            while (j < input.length() && !currentStates.empty()) {
                char c = input[j];

                // Compute next set of states
                std::set<std::string> nextStates;
                for (const auto& state : currentStates) {
                    for (const auto& trans : fa.getTransitions()) {
                        if (trans.from == state && trans.symbol == c) {
                            nextStates.insert(trans.to);
                        }
                    }
                }

                if (nextStates.empty()) break;

                currentStates = std::move(nextStates);
                ++j;
                ++currentCol;

                // Check if any current state is final; keep the highest priority token
                TokenType best = TOKEN_UNKNOWN;
                int bestPriority = -1;
                for (const auto& state : currentStates) {
                    auto it = finalToToken.find(state);
                    if (it != finalToToken.end()) {
                        int prio = tokenPriority(it->second);
                        if (prio > bestPriority) {
                            bestPriority = prio;
                            best = it->second;
                        }
                    }
                }
                if (best != TOKEN_UNKNOWN) {
                    lastAcceptPos = i;
                    lastAcceptType = best;
                    lastAcceptLen = j - i;
                    lastAcceptCol = col;
                }
            }

            if (lastAcceptType != TOKEN_UNKNOWN) {
                std::string lexeme = input.substr(lastAcceptPos, lastAcceptLen);
                std::cout << "Token: " << tokenTypeName(lastAcceptType)
                          << "  Lexeme: \"" << lexeme << "\""
                          << "  Line: " << line << "  Col: " << lastAcceptCol << "\n";
                // Advance i and col
                i = lastAcceptPos + lastAcceptLen;
                col += lastAcceptLen;
            } else {
                // No valid token – report error and skip one character
                std::cout << "Lexical error: unexpected character '" << input[i] << "'"
                          << " at line " << line << ", col " << col << "\n";
                ++i; ++col;
            }
        }
        std::cout << "Token: EOF  Line: " << line << "  Col: " << col << "\n";
    }

private:
    const FiniteAutomaton& fa;
    const std::map<std::string, TokenType>& finalToToken;
};

// ------------------------------------------------------------
// Build the automaton for DaQuVa
// ------------------------------------------------------------
std::pair<FiniteAutomaton, std::map<std::string, TokenType>> buildDaQuVaAutomaton() {
    FiniteAutomaton fa("start");
    std::map<std::string, TokenType> finalToToken;

    // Helper: add a single transition for a character
    auto addChar = [&](const std::string& from, char c, const std::string& to) {
        fa.addTransition(from, c, to);
    };

    // Helper: add a range of characters
    auto addCharRange = [&](const std::string& from, char first, char last, const std::string& to) {
        for (char c = first; c <= last; ++c)
            fa.addTransition(from, c, to);
    };

    // Helper: add a character class (e.g., "a-zA-Z0-9_")
    auto addCharClass = [&](const std::string& from, const std::string& charClass, const std::string& to) {
        size_t i = 0;
        while (i < charClass.length()) {
            if (i + 2 < charClass.length() && charClass[i+1] == '-') {
                char start = charClass[i];
                char end = charClass[i+2];
                for (char c = start; c <= end; ++c)
                    fa.addTransition(from, c, to);
                i += 3;
            } else {
                fa.addTransition(from, charClass[i], to);
                ++i;
            }
        }
    };

    // Helper: add a keyword (creates a chain of states)
    auto addKeyword = [&](const std::string& keyword, TokenType type) {
        std::string currentState = "start";
        std::string prefix = "kw_" + keyword + "_";
        for (size_t i = 0; i < keyword.length(); ++i) {
            char c = keyword[i];
            std::string nextState = prefix + keyword.substr(0, i+1);
            fa.addTransition(currentState, c, nextState);
            currentState = nextState;
        }
        fa.addFinalState(currentState);
        finalToToken[currentState] = type;
    };

    // Helper: add a multi-character operator (like "->", ">=")
    auto addOperator = [&](const std::string& op, TokenType type) {
        std::string currentState = "start";
        std::string prefix = "op_" + op + "_";
        for (size_t i = 0; i < op.length(); ++i) {
            char c = op[i];
            std::string nextState = prefix + op.substr(0, i+1);
            fa.addTransition(currentState, c, nextState);
            currentState = nextState;
        }
        fa.addFinalState(currentState);
        finalToToken[currentState] = type;
    };

    // ----- Keywords -----
    addKeyword("my", TOKEN_MY);
    addKeyword("db", TOKEN_DB);
    addKeyword("function", TOKEN_FUNCTION);
    addKeyword("return", TOKEN_RETURN);
    addKeyword("where", TOKEN_WHERE);
    addKeyword("AND", TOKEN_AND);
    addKeyword("OR", TOKEN_OR);
    addKeyword("columns", TOKEN_COLUMNS);
    addKeyword("using", TOKEN_USING);
    addKeyword("tool", TOKEN_TOOL);
    addKeyword("console", TOKEN_CONSOLE);
    addKeyword("file", TOKEN_FILE);
    addKeyword("from", TOKEN_FROM);
    addKeyword("addColumns", TOKEN_ADDCOLUMNS);
    addKeyword("Here", TOKEN_HERE);
    addKeyword("allowDanger", TOKEN_ALLOWDANGER);
    addKeyword("scan", TOKEN_SCAN);
    addKeyword("filter", TOKEN_FILTER);
    addKeyword("merge", TOKEN_MERGE);
    addKeyword("output", TOKEN_OUTPUT);
    addKeyword("newTable", TOKEN_NEWTABLE);
    addKeyword("findLocalDuplicates", TOKEN_FINDLOCALDUPLICATES);

    // ----- Identifiers -----
    // First character: letter or underscore
    addCharClass("start", "a-zA-Z_", "id");
    // Subsequent characters: letter, digit, underscore
    addCharClass("id", "a-zA-Z0-9_", "id");
    fa.addFinalState("id");
    finalToToken["id"] = TOKEN_IDENTIFIER;

    // ----- Numbers (non-negative integers) -----
    addCharClass("start", "0-9", "num");
    addCharClass("num", "0-9", "num");
    fa.addFinalState("num");
    finalToToken["num"] = TOKEN_NUMBER;

    // ----- String literals -----
    // Opening quote
    fa.addTransition("start", '"', "string");
    // In string state, loop on any character except quote
    for (char c = 32; c <= 126; ++c) {  // printable ASCII
        if (c != '"') {
            fa.addTransition("string", c, "string");
        }
    }
    // Closing quote leads to final state
    fa.addTransition("string", '"', "string_end");
    fa.addFinalState("string_end");
    finalToToken["string_end"] = TOKEN_STRING;

    // ----- Single-character operators & punctuation -----
    auto addSingle = [&](char ch, const std::string& stateName, TokenType tt) {
        fa.addTransition("start", ch, stateName);
        fa.addFinalState(stateName);
        finalToToken[stateName] = tt;
    };
    addSingle('.', "dot", TOKEN_DOT);
    addSingle(',', "comma", TOKEN_COMMA);
    addSingle(';', "semicolon", TOKEN_SEMICOLON);
    addSingle(':', "colon", TOKEN_COLON);
    addSingle('[', "lbracket", TOKEN_LBRACKET);
    addSingle(']', "rbracket", TOKEN_RBRACKET);
    addSingle('(', "lparen", TOKEN_LPAREN);
    addSingle(')', "rparen", TOKEN_RPAREN);
    addSingle('{', "lbrace", TOKEN_LBRACE);
    addSingle('}', "rbrace", TOKEN_RBRACE);
    addSingle('=', "assign", TOKEN_ASSIGN);   // single '=' for assignment

    // ----- Multi-character operators -----
    addOperator("->", TOKEN_ARROW);
    addOperator("==", TOKEN_EQ);
    addOperator("!=", TOKEN_NE);
    addOperator("<=", TOKEN_LE);
    addOperator(">=", TOKEN_GE);
    addOperator("<", TOKEN_LT);    // single, but addOperator works for one char too
    addOperator(">", TOKEN_GT);

    return {fa, finalToToken};
}

// ------------------------------------------------------------
// Main: read input from file or stdin, tokenise
// ------------------------------------------------------------
int main(int argc, char* argv[]) {
    // Build the automaton
    auto [fa, finalMap] = buildDaQuVaAutomaton();

    // Optional: print automaton for debugging
    // fa.print();

    Lexer lexer(fa, finalMap);

    std::string input;
    if (argc > 1) {
        // Read from file
        std::ifstream file(argv[1]);
        if (!file) {
            std::cerr << "Error: cannot open file " << argv[1] << "\n";
            return 1;
        }
        input.assign((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
    } else {
        // Read from stdin
        std::cout << "Enter DaQuVa code:\n";
        input.assign((std::istreambuf_iterator<char>(std::cin)),
                      std::istreambuf_iterator<char>());
    }

    lexer.tokenise(input);
    return 0;
}
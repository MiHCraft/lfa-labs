#include <cctype>
#include <fstream>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;

enum class TokenType {
    My, Db, Function, Return, Where, And, Or, Columns, Using, Tool,
    Console, File, From, AddColumns, Here, AllowDanger, Scan, Filter,
    Merge, Output, NewTable, FindLocalDuplicates,
    Identifier, Number, String,
    Dot, Comma, Semicolon, Colon, Arrow, LBracket, RBracket, LParen,
    RParen, Assign, Eq, Ne, Lt, Le, Gt, Ge,
    End
};

struct Token {
    TokenType type;
    string lexeme;
    size_t line;
    size_t column;
};

struct TokenRule {
    TokenType type;
    regex pattern;
};

static string tokenName(TokenType type) {
    switch (type) {
        case TokenType::My: return "MY";
        case TokenType::Db: return "DB";
        case TokenType::Function: return "FUNCTION";
        case TokenType::Return: return "RETURN";
        case TokenType::Where: return "WHERE";
        case TokenType::And: return "AND";
        case TokenType::Or: return "OR";
        case TokenType::Columns: return "COLUMNS";
        case TokenType::Using: return "USING";
        case TokenType::Tool: return "TOOL";
        case TokenType::Console: return "CONSOLE";
        case TokenType::File: return "FILE";
        case TokenType::From: return "FROM";
        case TokenType::AddColumns: return "ADDCOLUMNS";
        case TokenType::Here: return "HERE";
        case TokenType::AllowDanger: return "ALLOWDANGER";
        case TokenType::Scan: return "SCAN";
        case TokenType::Filter: return "FILTER";
        case TokenType::Merge: return "MERGE";
        case TokenType::Output: return "OUTPUT";
        case TokenType::NewTable: return "NEWTABLE";
        case TokenType::FindLocalDuplicates: return "FINDLOCALDUPLICATES";
        case TokenType::Identifier: return "IDENTIFIER";
        case TokenType::Number: return "NUMBER";
        case TokenType::String: return "STRING";
        case TokenType::Dot: return "DOT";
        case TokenType::Comma: return "COMMA";
        case TokenType::Semicolon: return "SEMICOLON";
        case TokenType::Colon: return "COLON";
        case TokenType::Arrow: return "ARROW";
        case TokenType::LBracket: return "LBRACKET";
        case TokenType::RBracket: return "RBRACKET";
        case TokenType::LParen: return "LPAREN";
        case TokenType::RParen: return "RPAREN";
        case TokenType::Assign: return "ASSIGN";
        case TokenType::Eq: return "EQ";
        case TokenType::Ne: return "NE";
        case TokenType::Lt: return "LT";
        case TokenType::Le: return "LE";
        case TokenType::Gt: return "GT";
        case TokenType::Ge: return "GE";
        case TokenType::End: return "EOF";
    }
    return "UNKNOWN";
}

class Lexer {
public:
    vector<Token> tokenize(const string& source) const {
        vector<Token> tokens;
        size_t pos = 0;
        size_t line = 1;
        size_t column = 1;

        while (pos < source.size()) {
            smatch match;
            string rest = source.substr(pos);

            if (regex_search(rest, match, regex(R"(^[ \t\r]+)"))) {
                advance(match.str(), pos, line, column);
                continue;
            }
            if (regex_search(rest, match, regex(R"(^\n)"))) {
                advance(match.str(), pos, line, column);
                continue;
            }
            if (regex_search(rest, match, regex(R"(^#[^\n]*)"))) {
                advance(match.str(), pos, line, column);
                continue;
            }

            bool found = false;
            for (const auto& rule : rules) {
                if (regex_search(rest, match, rule.pattern)) {
                    string lexeme = match.str();
                    tokens.push_back({rule.type, lexeme, line, column});
                    advance(lexeme, pos, line, column);
                    found = true;
                    break;
                }
            }

            if (!found) {
                throw runtime_error("Lexical error at line " + to_string(line) +
                                    ", column " + to_string(column) +
                                    ": unexpected character '" + source.substr(pos, 1) + "'");
            }
        }

        tokens.push_back({TokenType::End, "", line, column});
        return tokens;
    }

private:
    vector<TokenRule> rules = {
        {TokenType::Arrow, regex(R"(^->)")},
        {TokenType::Eq, regex(R"(^==)")},
        {TokenType::Ne, regex(R"(^!=)")},
        {TokenType::Le, regex(R"(^<=)")},
        {TokenType::Ge, regex(R"(^>=)")},

        {TokenType::My, regex(R"(^my\b)")},
        {TokenType::Db, regex(R"(^db\b)")},
        {TokenType::Function, regex(R"(^function\b)")},
        {TokenType::Return, regex(R"(^return\b)")},
        {TokenType::Where, regex(R"(^where\b)")},
        {TokenType::And, regex(R"(^AND\b)")},
        {TokenType::Or, regex(R"(^OR\b)")},
        {TokenType::Columns, regex(R"(^columns\b)")},
        {TokenType::Using, regex(R"(^using\b)")},
        {TokenType::Tool, regex(R"(^tool\b)")},
        {TokenType::Console, regex(R"(^console\b)")},
        {TokenType::File, regex(R"(^file\b)")},
        {TokenType::From, regex(R"(^from\b)")},
        {TokenType::AddColumns, regex(R"(^addColumns\b)")},
        {TokenType::Here, regex(R"(^Here\b)")},
        {TokenType::AllowDanger, regex(R"(^allowDanger\b)")},
        {TokenType::Scan, regex(R"(^scan\b)")},
        {TokenType::Filter, regex(R"(^filter\b)")},
        {TokenType::Merge, regex(R"(^merge\b)")},
        {TokenType::Output, regex(R"(^output\b)")},
        {TokenType::NewTable, regex(R"(^newTable\b)")},
        {TokenType::FindLocalDuplicates, regex(R"(^findLocalDuplicates\b)")},

        {TokenType::String, regex(R"(^"([^"\\]|\\.)*")")},
        {TokenType::Number, regex(R"(^[0-9]+)")},
        {TokenType::Identifier, regex(R"(^[A-Za-z_][A-Za-z0-9_]*)")},

        {TokenType::Dot, regex(R"(^\.)")},
        {TokenType::Comma, regex(R"(^,)")},
        {TokenType::Semicolon, regex(R"(^;)")},
        {TokenType::Colon, regex(R"(^:)")},
        {TokenType::LBracket, regex(R"(^\[)")},
        {TokenType::RBracket, regex(R"(^\])")},
        {TokenType::LParen, regex(R"(^\()")},
        {TokenType::RParen, regex(R"(^\))")},
        {TokenType::Assign, regex(R"(^=)")},
        {TokenType::Lt, regex(R"(^<)")},
        {TokenType::Gt, regex(R"(^>)")}
    };

    static void advance(const string& text, size_t& pos, size_t& line, size_t& column) {
        for (char ch : text) {
            pos++;
            if (ch == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }
        }
    }
};

struct AstNode {
    string type;
    string value;
    vector<AstNode> children;

    AstNode(string type, string value = {}) : type(std::move(type)), value(std::move(value)) {}

    void add(AstNode child) {
        children.push_back(std::move(child));
    }
};

static void printAst(const AstNode& node, const string& prefix = "", bool last = true) {
    cout << prefix << (last ? "`-- " : "|-- ") << node.type;
    if (!node.value.empty()) cout << ": " << node.value;
    cout << "\n";

    string nextPrefix = prefix + (last ? "    " : "|   ");
    for (size_t i = 0; i < node.children.size(); i++) {
        printAst(node.children[i], nextPrefix, i + 1 == node.children.size());
    }
}

class Parser {
public:
    explicit Parser(vector<Token> tokens) : tokens(std::move(tokens)) {}

    AstNode parseProgram() {
        AstNode program("Program");
        while (!check(TokenType::End)) {
            program.add(parseStatement());
        }
        consume(TokenType::End, "expected end of input");
        return program;
    }

private:
    vector<Token> tokens;
    size_t current = 0;

    AstNode parseStatement() {
        if (match(TokenType::My)) return parseDatabaseDeclaration();
        if (match(TokenType::Function)) return parseFunctionDeclaration();
        if (match(TokenType::Return)) return parseReturnStatement();
        if (match(TokenType::Output)) return parseOutputStatement();
        if (match(TokenType::Merge)) return parseMergeStatement();
        return parseAssignment();
    }

    AstNode parseDatabaseDeclaration() {
        AstNode node("DatabaseDeclaration");
        consume(TokenType::Db, "expected 'db' after 'my'");
        node.add(parsePrimary());
        consume(TokenType::Semicolon, "expected ';' after database declaration");
        return node;
    }

    AstNode parseFunctionDeclaration() {
        Token name = consume(TokenType::Identifier, "expected function name");
        AstNode node("FunctionDeclaration", name.lexeme);

        consume(TokenType::LParen, "expected '(' after function name");
        AstNode params("Parameters");
        if (!check(TokenType::RParen)) {
            do {
                params.add(AstNode("Parameter", consume(TokenType::Identifier, "expected parameter name").lexeme));
            } while (match(TokenType::Comma));
        }
        consume(TokenType::RParen, "expected ')' after parameters");
        consume(TokenType::Colon, "expected ':' before function body");

        node.add(std::move(params));
        AstNode body("Body");
        while (!check(TokenType::Return) && !check(TokenType::End)) {
            body.add(parseStatement());
        }
        body.add(parseStatement());
        node.add(std::move(body));
        return node;
    }

    AstNode parseAssignment() {
        Token name = consume(TokenType::Identifier, "expected statement or assignment target");
        AstNode node("Assignment", name.lexeme);
        consume(TokenType::Assign, "expected '=' after assignment target");
        node.add(parseExpression());
        consume(TokenType::Semicolon, "expected ';' after assignment");
        return node;
    }

    AstNode parseReturnStatement() {
        AstNode node("ReturnStatement");
        node.add(parseExpression());
        consume(TokenType::Semicolon, "expected ';' after return statement");
        return node;
    }

    AstNode parseOutputStatement() {
        AstNode node("OutputStatement");
        node.add(parseExpression());
        consume(TokenType::Arrow, "expected '->' in output statement");
        if (match(TokenType::Console)) {
            node.add(AstNode("Target", "console"));
        } else if (match(TokenType::File)) {
            AstNode target("Target", "file");
            target.add(parsePrimary());
            node.add(std::move(target));
        } else {
            error(peek(), "expected output target");
        }
        consume(TokenType::Semicolon, "expected ';' after output statement");
        return node;
    }

    AstNode parseMergeStatement() {
        AstNode node("MergeStatement");
        node.add(parseExpression());
        consume(TokenType::Arrow, "expected '->' in merge statement");
        consume(TokenType::Here, "expected 'Here' merge target");
        consume(TokenType::LParen, "expected '(' after Here");
        AstNode target("HereTarget");
        target.add(parsePrimary());
        consume(TokenType::RParen, "expected ')' after Here value");
        consume(TokenType::AllowDanger, "expected 'allowDanger' after Here target");
        target.add(AstNode("SafetyModifier", "allowDanger"));
        node.add(std::move(target));
        consume(TokenType::Semicolon, "expected ';' after merge statement");
        return node;
    }

    AstNode parseExpression() {
        if (match(TokenType::Scan)) return parseScanExpression();
        if (match(TokenType::Filter)) return parseFilterExpression();
        if (match(TokenType::FindLocalDuplicates)) return parseDuplicateExpression();
        if (match(TokenType::NewTable)) return parseNewTableExpression();
        return parseLogicExpression();
    }

    AstNode parseScanExpression() {
        AstNode node("ScanExpression");
        node.add(parseQualifiedExpression());
        consume(TokenType::Using, "expected 'using' in scan expression");
        consume(TokenType::Tool, "expected 'tool' after using");
        node.add(parseExpression());
        return node;
    }

    AstNode parseFilterExpression() {
        AstNode node("FilterExpression");
        node.add(parseExpression());
        consume(TokenType::Where, "expected 'where' in filter expression");
        node.add(parseLogicExpression());
        return node;
    }

    AstNode parseDuplicateExpression() {
        AstNode node("FindLocalDuplicatesExpression");
        node.add(parseExpression());
        consume(TokenType::Columns, "expected 'columns' in duplicate search");
        node.add(parseExpression());
        consume(TokenType::Using, "expected 'using' in duplicate search");
        consume(TokenType::Tool, "expected 'tool' after using");
        node.add(parseExpression());
        return node;
    }

    AstNode parseNewTableExpression() {
        AstNode node("NewTableExpression");
        node.add(parseExpression());
        consume(TokenType::From, "expected 'from' in newTable expression");
        node.add(parseExpression());
        consume(TokenType::AddColumns, "expected 'addColumns' in newTable expression");
        node.add(parseArrayLiteral());
        return node;
    }

    AstNode parseLogicExpression() {
        AstNode node = parseComparison();
        while (match(TokenType::And) || match(TokenType::Or)) {
            Token op = previous();
            AstNode parent("LogicalExpression", op.lexeme);
            parent.add(std::move(node));
            parent.add(parseComparison());
            node = std::move(parent);
        }
        return node;
    }

    AstNode parseComparison() {
        AstNode node = parseQualifiedExpression();
        if (match(TokenType::Eq) || match(TokenType::Ne) || match(TokenType::Lt) ||
            match(TokenType::Le) || match(TokenType::Gt) || match(TokenType::Ge)) {
            Token op = previous();
            AstNode parent("ComparisonExpression", op.lexeme);
            parent.add(std::move(node));
            parent.add(parseQualifiedExpression());
            return parent;
        }
        return node;
    }

    AstNode parseQualifiedExpression() {
        AstNode node = parsePrimary();
        while (match(TokenType::Dot)) {
            AstNode parent("MemberExpression");
            parent.add(std::move(node));
            parent.add(parsePrimary());
            node = std::move(parent);
        }
        return node;
    }

    AstNode parsePrimary() {
        if (match(TokenType::String)) return AstNode("StringLiteral", previous().lexeme);
        if (match(TokenType::Number)) return AstNode("NumberLiteral", previous().lexeme);
        if (match(TokenType::LBracket)) return finishArrayLiteral();

        if (match(TokenType::Identifier)) {
            Token name = previous();
            if (match(TokenType::LParen)) return finishCallExpression(name.lexeme);
            return AstNode("Identifier", name.lexeme);
        }

        error(peek(), "expected expression");
        return AstNode("Error");
    }

    AstNode parseArrayLiteral() {
        consume(TokenType::LBracket, "expected '['");
        return finishArrayLiteral();
    }

    AstNode finishArrayLiteral() {
        AstNode node("ArrayLiteral");
        if (!check(TokenType::RBracket)) {
            do {
                node.add(parseExpression());
            } while (match(TokenType::Comma));
        }
        consume(TokenType::RBracket, "expected ']' after array literal");
        return node;
    }

    AstNode finishCallExpression(const string& name) {
        AstNode node("CallExpression", name);
        if (!check(TokenType::RParen)) {
            do {
                node.add(parseExpression());
            } while (match(TokenType::Comma));
        }
        consume(TokenType::RParen, "expected ')' after arguments");
        return node;
    }

    bool match(TokenType type) {
        if (!check(type)) return false;
        advance();
        return true;
    }

    bool check(TokenType type) const {
        if (isAtEnd()) return type == TokenType::End;
        return peek().type == type;
    }

    const Token& advance() {
        if (!isAtEnd()) current++;
        return previous();
    }

    bool isAtEnd() const {
        return peek().type == TokenType::End;
    }

    const Token& peek() const {
        return tokens[current];
    }

    const Token& previous() const {
        return tokens[current - 1];
    }

    Token consume(TokenType type, const string& message) {
        if (check(type)) return advance();
        error(peek(), message);
        return peek();
    }

    [[noreturn]] void error(const Token& token, const string& message) const {
        throw runtime_error("Parse error at line " + to_string(token.line) +
                            ", column " + to_string(token.column) +
                            " near '" + token.lexeme + "': " + message);
    }
};

static string readFile(const string& path) {
    ifstream file(path);
    if (!file) {
        throw runtime_error("cannot open file: " + path);
    }
    return string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
}

static void printTokens(const vector<Token>& tokens) {
    cout << "Tokens\n";
    for (const auto& token : tokens) {
        cout << "  " << tokenName(token.type);
        if (!token.lexeme.empty()) cout << " lexeme=[" << token.lexeme << "]";
        cout << " at " << token.line << ":" << token.column << "\n";
    }
}

int main(int argc, char* argv[]) {
    try {
        string path = argc > 1 ? argv[1] : "input.dqv";
        string source = readFile(path);

        Lexer lexer;
        vector<Token> tokens = lexer.tokenize(source);
        Parser parser(tokens);
        AstNode ast = parser.parseProgram();

        printTokens(tokens);
        cout << "\nAbstract Syntax Tree\n";
        printAst(ast);
        return 0;
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

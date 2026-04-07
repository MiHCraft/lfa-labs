#include <algorithm>
#include <cctype>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Production {
    string lhs;
    vector<string> rhs; // empty = epsilon
};

struct Grammar {
    string start;
    set<string> nonterminals;
    set<string> terminals;
    vector<Production> productions;

    void recomputeSymbolSets() {
        nonterminals.clear();
        terminals.clear();
        for (const auto& p : productions) {
            nonterminals.insert(p.lhs);
            for (const auto& sym : p.rhs) {
                if (sym.empty()) continue;
                if (!sym.empty() && isupper(static_cast<unsigned char>(sym[0]))) {
                    nonterminals.insert(sym);
                } else {
                    terminals.insert(sym);
                }
            }
        }
    }
};

static string trim(const string& s) {
    size_t b = 0;
    while (b < s.size() && isspace(static_cast<unsigned char>(s[b]))) b++;
    size_t e = s.size();
    while (e > b && isspace(static_cast<unsigned char>(s[e - 1]))) e--;
    return s.substr(b, e - b);
}

static vector<string> split(const string& s, char delim) {
    vector<string> out;
    string token;
    stringstream ss(s);
    while (getline(ss, token, delim)) {
        out.push_back(token);
    }
    return out;
}

static bool isEpsilonToken(const string& t) {
    string x = t;
    for (auto& c : x) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return x == "eps" || x == "epsilon" || x == "&" || x == "e";
}

static vector<string> parseRhsSymbols(const string& rhsRaw) {
    string rhs = trim(rhsRaw);
    if (rhs.empty()) return {};
    if (isEpsilonToken(rhs)) return {};

    // If spaced, split by whitespace; otherwise parse compact form (aBBA -> a B B A)
    bool hasSpace = rhs.find(' ') != string::npos || rhs.find('\t') != string::npos;
    vector<string> symbols;
    if (hasSpace) {
        stringstream ss(rhs);
        string tok;
        while (ss >> tok) {
            symbols.push_back(tok);
        }
    } else {
        for (char ch : rhs) {
            if (isspace(static_cast<unsigned char>(ch))) continue;
            string tok(1, ch);
            symbols.push_back(tok);
        }
    }
    return symbols;
}

static void dedupeProductions(Grammar& g) {
    set<string> seen;
    vector<Production> unique;
    for (const auto& p : g.productions) {
        string key = p.lhs + "->";
        for (size_t i = 0; i < p.rhs.size(); i++) {
            if (i) key.push_back(' ');
            key += p.rhs[i];
        }
        if (seen.insert(key).second) {
            unique.push_back(p);
        }
    }
    g.productions.swap(unique);
}

static Grammar parseGrammarFromFile(const string& path, bool& ok) {
    ifstream in(path);
    if (!in) {
        ok = false;
        return {};
    }

    Grammar g;
    string line;
    vector<string> prodLines;

    while (getline(in, line)) {
        string t = trim(line);
        if (t.empty()) continue;
        if (t[0] == '#') continue;

        // Allow optional "S: X" start line
        if (t.size() > 2 && (t[0] == 'S' && t[1] == ':')) {
            g.start = trim(t.substr(2));
            continue;
        }

        if (t.find("->") != string::npos) {
            prodLines.push_back(t);
        }
    }

    for (const auto& l : prodLines) {
        auto parts = split(l, '-');
        // Expect format: A -> ... (split on "->" manually)
        size_t arrow = l.find("->");
        if (arrow == string::npos) continue;
        string lhs = trim(l.substr(0, arrow));
        string rhsAll = trim(l.substr(arrow + 2));
        if (lhs.empty()) continue;
        if (g.start.empty()) g.start = lhs;

        auto alternatives = split(rhsAll, '|');
        for (auto& alt : alternatives) {
            vector<string> rhs = parseRhsSymbols(alt);
            g.productions.push_back({lhs, rhs});
        }
    }

    g.recomputeSymbolSets();
    dedupeProductions(g);
    ok = !g.start.empty() && !g.productions.empty();
    return g;
}

static Grammar defaultVariantGrammar() {
    Grammar g;
    g.start = "S";
    g.productions = {
        {"S", {"a", "B"}},
        {"S", {"b", "A"}},
        {"S", {"A"}},
        {"A", {"B"}},
        {"A", {"S", "a"}},
        {"A", {"b", "B", "A"}},
        {"A", {"b"}},
        {"B", {"b"}},
        {"B", {"b", "S"}},
        {"B", {"a", "D"}},
        {"B", {}},
        {"D", {"A", "A"}},
        {"C", {"B", "a"}},
    };
    g.recomputeSymbolSets();
    dedupeProductions(g);
    return g;
}

static string rhsToString(const vector<string>& rhs) {
    if (rhs.empty()) return "eps";
    string out;
    for (size_t i = 0; i < rhs.size(); i++) {
        if (i) out.push_back(' ');
        out += rhs[i];
    }
    return out;
}

static void printGrammar(const Grammar& g, const string& title) {
    cout << "\n" << title << "\n";
    cout << "Start: " << g.start << "\n";

    cout << "Vn = { ";
    for (auto it = g.nonterminals.begin(); it != g.nonterminals.end(); ++it) {
        if (it != g.nonterminals.begin()) cout << ", ";
        cout << *it;
    }
    cout << " }\n";

    cout << "Vt = { ";
    for (auto it = g.terminals.begin(); it != g.terminals.end(); ++it) {
        if (it != g.terminals.begin()) cout << ", ";
        cout << *it;
    }
    cout << " }\n";

    map<string, vector<vector<string>>> grouped;
    for (const auto& p : g.productions) grouped[p.lhs].push_back(p.rhs);

    cout << "P:\n";
    for (const auto& entry : grouped) {
        cout << "  " << entry.first << " -> ";
        for (size_t i = 0; i < entry.second.size(); i++) {
            if (i) cout << " | ";
            cout << rhsToString(entry.second[i]);
        }
        cout << "\n";
    }
}

static Grammar removeEpsilonProductions(const Grammar& g, bool& startNullable) {
    set<string> nullable;
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& p : g.productions) {
            if (p.rhs.empty()) {
                if (nullable.insert(p.lhs).second) changed = true;
                continue;
            }
            bool allNullable = true;
            for (const auto& sym : p.rhs) {
                if (!isupper(static_cast<unsigned char>(sym[0])) || nullable.count(sym) == 0) {
                    allNullable = false;
                    break;
                }
            }
            if (allNullable) {
                if (nullable.insert(p.lhs).second) changed = true;
            }
        }
    }

    startNullable = nullable.count(g.start) > 0;

    Grammar out;
    out.start = g.start;

    for (const auto& p : g.productions) {
        if (p.rhs.empty()) {
            // remove all eps productions for now; we'll re-add S->eps if needed
            continue;
        }

        // generate all variants by dropping nullable nonterminals
        vector<vector<string>> variants;
        vector<string> current;

        function<void(size_t)> dfs = [&](size_t idx) {
            if (idx == p.rhs.size()) {
                variants.push_back(current);
                return;
            }
            const string& sym = p.rhs[idx];
            bool isNullable = isupper(static_cast<unsigned char>(sym[0])) && nullable.count(sym) > 0;
            if (isNullable) {
                // drop
                dfs(idx + 1);
            }
            // keep
            current.push_back(sym);
            dfs(idx + 1);
            current.pop_back();
        };

        dfs(0);

        for (const auto& rhs : variants) {
            if (rhs.empty()) {
                if (p.lhs == g.start && startNullable) {
                    out.productions.push_back({p.lhs, {}});
                }
            } else {
                out.productions.push_back({p.lhs, rhs});
            }
        }
    }

    out.recomputeSymbolSets();
    dedupeProductions(out);
    return out;
}

static Grammar removeUnitProductions(const Grammar& g) {
    map<string, set<string>> unit;
    for (const auto& nt : g.nonterminals) {
        unit[nt].insert(nt);
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& p : g.productions) {
            if (p.rhs.size() == 1 && isupper(static_cast<unsigned char>(p.rhs[0][0]))) {
                const string& A = p.lhs;
                const string& B = p.rhs[0];
                if (unit[A].insert(B).second) changed = true;
                for (const auto& C : unit[B]) {
                    if (unit[A].insert(C).second) changed = true;
                }
            }
        }
    }

    Grammar out;
    out.start = g.start;

    // Build production list without unit productions
    for (const auto& A : g.nonterminals) {
        for (const auto& B : unit[A]) {
            for (const auto& p : g.productions) {
                if (p.lhs != B) continue;
                if (p.rhs.size() == 1 && isupper(static_cast<unsigned char>(p.rhs[0][0]))) {
                    continue; // skip unit
                }
                out.productions.push_back({A, p.rhs});
            }
        }
    }

    out.recomputeSymbolSets();
    dedupeProductions(out);
    return out;
}

static Grammar removeInaccessibleSymbols(const Grammar& g) {
    set<string> reachable;
    queue<string> q;
    reachable.insert(g.start);
    q.push(g.start);

    while (!q.empty()) {
        string A = q.front();
        q.pop();
        for (const auto& p : g.productions) {
            if (p.lhs != A) continue;
            for (const auto& sym : p.rhs) {
                if (!sym.empty() && isupper(static_cast<unsigned char>(sym[0]))) {
                    if (reachable.insert(sym).second) q.push(sym);
                }
            }
        }
    }

    Grammar out;
    out.start = g.start;
    for (const auto& p : g.productions) {
        if (reachable.count(p.lhs)) {
            out.productions.push_back(p);
        }
    }
    out.recomputeSymbolSets();
    dedupeProductions(out);
    return out;
}

static Grammar removeNonProductiveSymbols(const Grammar& g) {
    set<string> productive;
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& p : g.productions) {
            bool ok = true;
            for (const auto& sym : p.rhs) {
                if (sym.empty()) continue;
                if (isupper(static_cast<unsigned char>(sym[0]))) {
                    if (productive.count(sym) == 0) {
                        ok = false;
                        break;
                    }
                }
            }
            if (ok) {
                if (productive.insert(p.lhs).second) changed = true;
            }
        }
    }

    Grammar out;
    out.start = g.start;
    for (const auto& p : g.productions) {
        if (productive.count(p.lhs) == 0) continue;
        bool ok = true;
        for (const auto& sym : p.rhs) {
            if (sym.empty()) continue;
            if (isupper(static_cast<unsigned char>(sym[0])) && productive.count(sym) == 0) {
                ok = false;
                break;
            }
        }
        if (ok) out.productions.push_back(p);
    }

    out.recomputeSymbolSets();
    dedupeProductions(out);
    return out;
}

static string uniqueNonTerminal(const set<string>& used, const string& base) {
    if (!used.count(base)) return base;
    int idx = 1;
    while (used.count(base + to_string(idx))) idx++;
    return base + to_string(idx);
}

static Grammar toChomskyNormalForm(const Grammar& g, bool startNullable) {
    Grammar cnf = g;
    cnf.recomputeSymbolSets();

    // If epsilon is in language, create new start symbol with eps production.
    if (startNullable) {
        set<string> used = cnf.nonterminals;
        string newStart = uniqueNonTerminal(used, "S0");
        cnf.productions.push_back({newStart, {cnf.start}});
        cnf.productions.push_back({newStart, {}});
        // remove epsilon productions of old start (CNF allows epsilon only from start)
        vector<Production> filtered;
        for (const auto& p : cnf.productions) {
            if (p.lhs == cnf.start && p.rhs.empty()) continue;
            filtered.push_back(p);
        }
        cnf.productions.swap(filtered);
        cnf.start = newStart;
        cnf.recomputeSymbolSets();
    }

    // Replace terminals in long productions with new nonterminals.
    map<string, string> termToVar;
    set<string> used = cnf.nonterminals;
    vector<Production> replaced;
    for (const auto& p : cnf.productions) {
        if (p.rhs.size() >= 2) {
            vector<string> rhs = p.rhs;
            for (auto& sym : rhs) {
                if (!sym.empty() && !isupper(static_cast<unsigned char>(sym[0]))) {
                    if (!termToVar.count(sym)) {
                        string base = string("T_") + sym;
                        string var = uniqueNonTerminal(used, base);
                        used.insert(var);
                        termToVar[sym] = var;
                    }
                    sym = termToVar[sym];
                }
            }
            replaced.push_back({p.lhs, rhs});
        } else {
            replaced.push_back(p);
        }
    }

    for (const auto& entry : termToVar) {
        replaced.push_back({entry.second, {entry.first}});
    }

    cnf.productions.swap(replaced);
    cnf.recomputeSymbolSets();

    // Binarize productions longer than 2
    vector<Production> binarized;
    int freshId = 1;
    set<string> usedAfter = cnf.nonterminals;

    for (const auto& p : cnf.productions) {
        if (p.rhs.size() <= 2) {
            binarized.push_back(p);
            continue;
        }

        vector<string> rhs = p.rhs;
        string currentLhs = p.lhs;
        for (size_t i = 0; i + 2 < rhs.size(); i++) {
            string base = string("X") + to_string(freshId++);
            string newVar = uniqueNonTerminal(usedAfter, base);
            usedAfter.insert(newVar);
            binarized.push_back({currentLhs, {rhs[i], newVar}});
            currentLhs = newVar;
        }
        binarized.push_back({currentLhs, {rhs[rhs.size() - 2], rhs[rhs.size() - 1]}});
    }

    cnf.productions.swap(binarized);
    cnf.recomputeSymbolSets();

    // Remove any unit productions introduced by start handling
    cnf = removeUnitProductions(cnf);

    // Ensure epsilon only from the start symbol (if at all)
    if (startNullable) {
        vector<Production> filtered;
        for (const auto& p : cnf.productions) {
            if (p.rhs.empty() && p.lhs != cnf.start) continue;
            filtered.push_back(p);
        }
        cnf.productions.swap(filtered);
    } else {
        vector<Production> filtered;
        for (const auto& p : cnf.productions) {
            if (p.rhs.empty()) continue;
            filtered.push_back(p);
        }
        cnf.productions.swap(filtered);
    }

    cnf.recomputeSymbolSets();
    dedupeProductions(cnf);
    return cnf;
}

int main(int argc, char* argv[]) {
    string path = (argc > 1) ? argv[1] : "input.txt";
    bool ok = false;
    Grammar g = parseGrammarFromFile(path, ok);
    if (!ok) {
        cerr << "Could not read grammar from " << path << ". Using built-in Variant 20.\n";
        g = defaultVariantGrammar();
    }

    printGrammar(g, "Original Grammar");

    bool startNullable = false;
    Grammar noEps = removeEpsilonProductions(g, startNullable);
    printGrammar(noEps, "1) After Eliminating Epsilon Productions");

    Grammar noUnit = removeUnitProductions(noEps);
    printGrammar(noUnit, "2) After Eliminating Renaming (Unit) Productions");

    Grammar noInacc = removeInaccessibleSymbols(noUnit);
    printGrammar(noInacc, "3) After Eliminating Inaccessible Symbols");

    Grammar noNonProd = removeNonProductiveSymbols(noInacc);
    printGrammar(noNonProd, "4) After Eliminating Non-Productive Symbols");

    Grammar cnf = toChomskyNormalForm(noNonProd, startNullable);
    printGrammar(cnf, "5) Chomsky Normal Form");

    return 0;
}

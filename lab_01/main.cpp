#include <iostream>
#include <vector>
#include <set>
#include <queue>
#include <string>

/* ===== Production ===== */
struct Production {
    std::string from;   // left side (non-terminal)
    std::string to;     // right side (string of terminals + optional non-terminal)
};

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

    void generate(size_t maxWords = 15) {
        std::queue<std::string> q;
        std::set<std::string> seen;

        q.push(g->getStart());

        size_t generated = 0;

        while (!q.empty() && generated < maxWords) {
            std::string cur = q.front();
            q.pop();

            bool expanded = false;

            for (const auto& p : g->getProductions()) {
                if (cur.size() >= p.from.size() &&

                    // the end of a string is the same as p.from
                    cur.compare(cur.size() - p.from.size(), p.from.size(), p.from) == 0) {

                    // new string: replace p.from by p.to
                    std::string next = cur.substr(0, cur.size() - p.from.size()) + p.to;

                    // if hasn't seen, then mark as seen and put in queue (no computed loops)
                    if (seen.insert(next).second) {
                        q.push(next);
                    }

                    expanded = true;
                }
            }

            // can't expand = it's the end word
            if (!expanded) {
                std::cout << cur << "\n";
                ++generated;
            }
        }
    }
};

/* ===== Main ===== */
int main() {
    Grammar g("S");

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
    gen.generate();

    return 0;
}





/*




VN = { A B C S }
VT = { a b c d }
P:
  S -> dA
  A -> d
  A -> aB
  B -> bC
  C -> cA
  C -> aS

S
dA
daB
dabC
dabaS
dabadA
dabadd

S
dA
dd









*/
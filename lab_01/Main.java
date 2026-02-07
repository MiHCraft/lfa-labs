import java.util.ArrayList;
import java.util.Arrays;
import java.util.Random;

/*

// vmware
// parallels

Variant 20:
VN={S, A, B, C},
VT={a, b, c, d}, 
P={ 
    S → dA     
    A → d    
    A → aB   
    B → bC    
    C → cA
    C → aS
}

*/

class Grammar {
    // Some state variables as needed.
    // {V_n, V_t, P, S}
    ArrayList<String> v_n;
    ArrayList<String> v_t;

    // "A -> cG" ; "B -> fD"
    ArrayList<String> p_from; // "A"    "B"
    ArrayList<String> p_to;   // "cG"   "fD"

    public Grammar(ArrayList<String> v_n, ArrayList<String> v_t, 
                   ArrayList<String> p_from, ArrayList<String> p_to) {
        this.v_n = v_n;
        this.v_t = v_t;
        this.p_from = p_from;
        this.p_to = p_to;
    }

    
    


}

public class Main {
    public static void main(String[] args) {
        ArrayList<String> v_n = new ArrayList<>(Arrays.asList("S", "A", "B", "C"));
        ArrayList<String> v_t = new ArrayList<>(Arrays.asList("a", "b", "c", "d"));

        ArrayList<String> p_from = new ArrayList<>(Arrays.asList("S", "A", "A", "B", "C", "C"));
        ArrayList<String> p_to = new ArrayList<>(Arrays.asList("dA", "d", "aB", "bC", "cA", "aS"));

        Grammar g = new Grammar(v_n, v_t, p_from, p_to);
        System.out.println("Hello, World");


    }
}
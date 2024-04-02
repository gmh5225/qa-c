//10

int main() {
    int a = 1; 
    int b = a; 
    a = 2; 
    b = a; 

    int c = a + b + 3; // c == 7
    a = 1; 
    b = 2; 
    // 7 + 2 + 1 == 10
    int d = c + b + a;
    return d;
}
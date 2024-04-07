//15

int addTen(int *n) {
    *n = *n + 10;
    return 0; 
}

int main() {
    int value = 5;
    addTen(&value);

    return value;
}
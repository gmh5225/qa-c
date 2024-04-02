//0

int main() {
    int a = 5;
    int b = a + 2; // b should now be 7
    b = b - 3; // b should now be 4
    int c = b - 1; // c should be 3
    a = c + 6; // a should now be 9
    return a - 9; // Should return 0 if everything is correct, any deviation will result in a different error code
}
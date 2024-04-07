//10
int swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int main() {
    int v1 = 5;
    int v2 = 10;
    swap(&v1, &v2);
    return v1;
}
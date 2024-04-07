// 10

int main() {
    int v1 = 5;
    int v2 = 10;
    int *ptr1 = &v1; 
    int *ptr2 = &v2;
    *ptr1 = *ptr2;
    return *ptr1;
}
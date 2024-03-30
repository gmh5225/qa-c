//30

int main() {
    int v1 = 5;
    int v2 = 10;
    int *ptr1 = &v1;
    int *ptr2 = &v2;
    int temp = *ptr1;
    *ptr1 = *ptr2;
    *ptr2 = temp;
    temp = 20; 
    temp = 30; 
    *ptr1 = temp;
    return v1;
}
    // 10

    int main() {
        int v1 = 5;
        int v2 = 10;
        int *ptr1 = &v1; 

        *ptr1 = 10; 
        int result = *ptr1;
        return result;
    }

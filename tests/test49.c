//33

int add10(int **dptr) {
    **dptr = **dptr + 10; 
}


int doubleValue(int *ptr) {
    *ptr = *ptr + *ptr;
}

int main() {
    int value = 5; 
    int *ptr = &value;
    int **dptr = &ptr;

    add10(dptr); // value = 15
    doubleValue(ptr); // value = 30
    

    int additionResult = **dptr + 3; // additionResult = 33

    
    return additionResult; // 33
}
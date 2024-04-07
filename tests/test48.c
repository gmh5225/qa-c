//15

int mutateValue(int **dptr) {
    **dptr = **dptr + 5; 
}

int main() {
    int value = 10;
    int *ptr = &value;
    int **dptr = &ptr;

    mutateValue(dptr);
    
    return **dptr;
}
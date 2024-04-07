//5


int main() {
    int value = 5;
    int *ptr = &value; 
    int **dptr = &ptr; 

    
    return **dptr;
}
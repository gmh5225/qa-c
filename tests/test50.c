//3

int main() {
    int value = 5; 
    int *ptr = &value;
    value = 3; 
    int result = *ptr;
    return result;
}
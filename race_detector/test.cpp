#include <iostream>

void print(int a) {
    if (a < a * 2) {
        std::cout << a << std::endl;
    }
}

int main(int argc, char *argv[]) {
    int a = 5;
    int b = a + 10;
    std::cout << a << " " << b << std::endl;
    print(a);
    return 0;
}

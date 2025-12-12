//write a factorial function in C++
#include <iostream> 
using namespace std;
int factorial(int n) {
    if (n < 0) {
        cout << "Factorial is not defined for negative numbers." << endl;
        return -1; // Indicate error
    }
    if (n == 0 || n == 1) {
        return 1; // Base case
    }
    return n * factorial(n - 1); // Recursive case
}
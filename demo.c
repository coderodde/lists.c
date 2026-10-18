#include "linked_list.h"
#include <stdbool.h>
#include <stdio.h>

static size_t passed_assertions = 0;
static size_t failed_assertions = 0;

static void REPORT() 
{
    printf("Passed assertions: %zu\n", passed_assertions);
    printf("Failed assertions: %zu\n", failed_assertions);
    printf("Total assertions:  %zu\n", passed_assertions + failed_assertions);
    printf("Success rate:      %.2f%%\n", (double)passed_assertions / (passed_assertions + failed_assertions) * 100.0);

    if (failed_assertions > 0) 
    {
        puts("[STATUS] Some tests failed.");
        exit(EXIT_FAILURE);
    }
        
    puts("[STATUS] All tests passed.");
    exit(EXIT_SUCCESS);
}

#define ASSERT(condition, message) assert_impl((condition), __LINE__, (message));

static void assert_impl(bool condition, size_t line, const char* message) 
{
    if (condition) {
        passed_assertions++;
    } else {
        failed_assertions++;
        fprintf(stderr, "Assertion failed on line %zu: %s\n", line, message);
    }
}

int main()
{
    ASSERT(true, "Oh f yeah.")
    REPORT();
    return EXIT_SUCCESS;
}
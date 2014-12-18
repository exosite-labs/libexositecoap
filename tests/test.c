#include <stdio.h>
#include <string.h>
#include "minunit.h"
#include "../src/exosite.h"

int tests_run = 0;

static char * test_math() {
	mu_assert("[ERROR] 2+2 != 4", 2+2 == 4);
	return 0;
}

static char * all_tests() {
	// Make Sure the Tests Are Working
	mu_run_test(test_math);

	// Actually Run the Real Tests
	return 0;
}

int main(int argc, char **argv) {
	char *result = all_tests();
	if (result != 0) {
		printf("%s\n", result);
	}
	else {
		printf("ALL TESTS PASSED\n");
	}
	printf("Tests run: %d\n", tests_run);

	return result != 0;
}


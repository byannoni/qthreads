
#include <stdio.h>

#include "tinytest/tinytest.h"
#include "../pt_error.c"

enum pt_error current_error_code = PT_SUCCESS;

void error_codes_correspond(void)
{
	printf("Testing error code %d mapped correctly (%d)...\n", current_error_code, pt_error_map[current_error_code].err);
	ASSERT_EQUALS(current_error_code, pt_error_map[current_error_code].err);
}

int main(int argc, char** argv)
{
	for(; current_error_code < PT_ELAST; ++current_error_code)
		RUN(error_codes_correspond);
	return TEST_REPORT();
}


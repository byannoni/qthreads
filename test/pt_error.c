
#include <stdio.h>

#include "tinytest/tinytest.h"
#include "../pt_error.c"

enum pt_error current_error_code = PT_SUCCESS;

void error_codes_correspond()
{
	printf("Testing error code %d mapped correctly (%d)...\n", current_error_code, pt_error_map[current_error_code].err);
	ASSERT_EQUALS(current_error_code, pt_error_map[current_error_code].err);
}

void strerror_r_test()
{
	char buf1[8];
	char buf2[7];
	char buf3[200];
	int e = 0;

	puts("Testing pt_strerror_r()...");
	pt_strerror_r(PT_SUCCESS, buf1, 8);
	printf("Testing normal \"%s\" == \"%s\"...\n", "Success", buf1);
	ASSERT_STRING_EQUALS("Success", buf1);
	pt_strerror_r(PT_SUCCESS, buf2, 7);
	e = errno;
	printf("Testing overflow \"%s\" == \"%s\"...\n", "Succes", buf2);
	ASSERT_STRING_EQUALS("Succes", buf2);
	pt_strerror_r(PT_SUCCESS, buf3, 200);
	printf("Testing underflow \"%s\" == \"%s\"...\n", "Success", buf3);
	ASSERT_STRING_EQUALS("Success", buf3);
	puts("Testing pt_strerror_r() on invalid code...");
	pt_strerror_r(PT_ELAST, buf3, 200);
	e = errno;
	ASSERT_EQUALS(e, EINVAL);
}

int main(int argc, char** argv)
{
	for(; current_error_code < PT_ELAST; ++current_error_code)
		RUN(error_codes_correspond);

	RUN(strerror_r_test);
	return TEST_REPORT();
}


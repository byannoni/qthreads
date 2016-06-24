
#include <stdio.h>

#include "tinytest/tinytest.h"
#include "../qterror.c"

enum qterror current_error_code = QTSUCCESS;

void error_codes_correspond()
{
	printf("Testing error code %d mapped correctly (%d)...\n", current_error_code, qterror_map[current_error_code].err);
	ASSERT_EQUALS(current_error_code, qterror_map[current_error_code].err);
}

void strerror_r_test()
{
	char buf1[8];
	char buf2[7];
	char buf3[200];
	int e = 0;

	puts("Testing qtstrerror_r()...");
	qtstrerror_r(QTSUCCESS, buf1, 8);
	printf("Testing normal \"%s\" == \"%s\"...\n", "Success", buf1);
	ASSERT_STRING_EQUALS("Success", buf1);
	qtstrerror_r(QTSUCCESS, buf2, 7);
	e = errno;
	printf("Testing overflow \"%s\" == \"%s\"...\n", "Succes", buf2);
	ASSERT_STRING_EQUALS("Succes", buf2);
	qtstrerror_r(QTSUCCESS, buf3, 200);
	printf("Testing underflow \"%s\" == \"%s\"...\n", "Success", buf3);
	ASSERT_STRING_EQUALS("Success", buf3);
	puts("Testing qtstrerror_r() on invalid code...");
	qtstrerror_r(QTELAST, buf3, 200);
	e = errno;
	ASSERT_EQUALS(e, EINVAL);
}

int main(int argc, char** argv)
{
	for(; current_error_code < QTELAST; ++current_error_code)
		RUN(error_codes_correspond);

	RUN(strerror_r_test);
	return TEST_REPORT();
}


#ifndef scuts_included
#define scuts_included

#include <stdio.h>
#include <stdbool.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>

extern int runCount, failedCount;

typedef bool (*TestCase)(void);
void StartTests(void);
void EndTests(void);
bool AssertFailure(const char *message, const char* file, int line);

bool AssertEqString(const char* file, int line, const char* expected, const char* actual);
bool AssertEqWString(const char* file, int line, const wchar_t* expected, const wchar_t* actual);
bool AssertEqNumber(const char* file, int line, long long expected, long long actual);

bool AssertTrue(const char* file, int line, bool expr, const char* exprText);
bool AssertFalse(const char* file, int line, bool expr, const char* exprText);

#define MAX_TESTS_PER_GROUP (100)

typedef struct
{
	const char* initName;
	const char* cleanupName;
	char* testNames[MAX_TESTS_PER_GROUP + 1];
} TestGroup;

#ifdef WIN32
#define TESTFUNC __declspec(dllexport)
#else
#define TESTFUNC
#endif

#define BEGIN_TEST_GROUP(groupName) \
TESTFUNC TestGroup* groupName(void){ \
	static TestGroup testGroup; \
	testGroup.initName = NULL; \
	testGroup.cleanupName = NULL; \
	memset(testGroup.testNames, 0, sizeof(char*) * (MAX_TESTS_PER_GROUP + 1)); \
	int testCount = 0;

#define INIT(name) testGroup.initName = #name;

#define CLEANUP(name) testGroup.cleanupName = #name;

#define TEST(name) \
	if (testCount == MAX_TESTS_PER_GROUP) { fprintf(stderr, "Too many tests in group\n"); exit(1); } \
	testGroup.testNames[testCount++] = #name;

#define END_TEST_GROUP \
	return &testGroup; \
}

#define ASSERT_FAILURE(message) \
do { \
	return AssertFailure((message), __FILE__, __LINE__); \
} while (0)

#define CMP(a, b) (_Generic((a), \
int: cmpint((a), (b)), \
char *: cmpstr((a), (b))\
))


#define ASSERT_EQ(a, b) (_Generic((b), \
int: AssertEqNumber(__FILE__, __LINE__, (long long)(a), (long long)(b)), \
unsigned int: AssertEqNumber(__FILE__, __LINE__, (long long)(a), (long long)(b)), \
long: AssertEqNumber(__FILE__, __LINE__, (long long)(a), (long long)(b)), \
unsigned long: AssertEqNumber(__FILE__, __LINE__, (long long)(a), (long long)(b)), \
long long: AssertEqNumber(__FILE__, __LINE__, (long long)(a), (long long)(b)), \
unsigned long long: AssertEqNumber(__FILE__, __LINE__, (long long)(a), (long long)(b)), \
wchar_t *: AssertEqWString(__FILE__, __LINE__, (wchar_t *)(a), (wchar_t *)(b)), \
char *: AssertEqString(__FILE__, __LINE__, (char *)(a), (char *)(b)) \
))

#define ASSERT_TRUE(expr) AssertTrue(__FILE__, __LINE__, (expr), #expr)

#define ASSERT_FALSE(expr) AssertFalse(__FILE__, __LINE__, (expr), #expr)

void RunTestGroup(const char* name);

#define RUN_TEST_GROUP(name) do { RunTestGroup(#name); } while (0);

#ifdef __clang__
#pragma clang diagnostic ignored "-Wint-conversion"
#pragma clang diagnostic ignored "-Wint-to-pointer-cast"
#endif


#endif





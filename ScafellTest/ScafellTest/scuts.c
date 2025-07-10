#include <string.h>
#include <stdlib.h>
#include <wchar.h>

#ifdef WIN32
#include <Windows.h>
typedef HMODULE Module;
#endif

#ifdef __APPLE__
#include <dlfcn.h>
typedef void* Module;
#endif

#include "scuts.h"

typedef TestGroup* (*TestGroupFunc)(void);
typedef void (*TestWrapperFunc)(void);
typedef bool (*TestFunc)(void);

int runCount, failedCount;

void StartTests(void)
{
	runCount = 0;
	failedCount = 0;
}

void EndTests(void)
{
	printf("Tests run: %d\n", runCount);
	printf("Passed:    %d\n", runCount - failedCount);
	printf("Failed:    %d\n", failedCount);
	exit(failedCount ? EXIT_FAILURE : EXIT_SUCCESS);
}

static void* LoadFunc(Module m, const char* name)
{
	void* result = NULL;
#ifdef  WIN32
	result = GetProcAddress(m, name);
#endif

#ifdef __APPLE__
	result = dlsym(m, name);
#endif

	if (result == NULL)
	{
		fprintf(stderr, "Aborting - failed to load function %s\n", name);
	}

	return result;
}

static void RunTest(Module module, const TestGroup *tg, int number)
{
	const char* testName = tg->testNames[number];
	printf("  %s...", testName);
	TestFunc testFunc = LoadFunc(module, testName);

	if (tg->initName)
	{
		TestWrapperFunc init = LoadFunc(module, tg->initName);
		init();
	}

	if (testFunc())
	{
		printf("passed\n");
	}
	else
	{
		printf("failed\n");
		failedCount++;
	}

	runCount++;

	if (tg->cleanupName)
	{
		TestWrapperFunc cleanup = LoadFunc(module, tg->cleanupName);
		cleanup();
	}


}

void RunTestGroup(const char* name)
{
	TestGroupFunc loadTestGroup = NULL;
	Module module;
#ifdef WIN32
	module = GetModuleHandle(NULL);
#endif

#ifdef __APPLE__
	module = RTLD_DEFAULT;
#endif

	printf("Test Group: %s\n", name);
	printf("------------");
	for (int i = 0; name[i]; i++)
	{
		putc('-', stdout);
	}

	putc('\n', stdout);

	loadTestGroup = LoadFunc(module, name);
	TestGroup* testGroup = loadTestGroup();
	for (int i = 0; testGroup->testNames[i]; i++)
	{
		RunTest(module, testGroup, i);
	}

	putc('\n', stdout);
}

bool AssertFailure(const char *message, const char* file, int line)
{
	fprintf(stderr, "Assert failure %s at %s: %d\n", message, file, line);
	return false;
}

bool AssertEqString(const char* file, int line, const char* expected, const char* actual)
{
	if (strcmp(expected, actual) != 0)
	{
		return AssertFailure("string compare", file, line);
	}

	return true;
}

bool AssertEqWString(const char* file, int line, const wchar_t* expected, const wchar_t* actual)
{
	if (wcscmp(expected, actual) != 0)
	{
		return AssertFailure("string compare", file, line);
	}

	return true;
}

bool AssertEqNumber(const char* file, int line, long long expected, long long actual)
{
	if (expected != actual)
	{
		return AssertFailure("numeric compare", file, line);
	}

	return true;
}

bool AssertTrue(const char* file, int line, bool expr, const char* exprText)
{
	if (!expr)
	{
		char buf[100];
		sprintf(buf, "%s is false but expected true", exprText);
		return AssertFailure(buf, file, line);
	}

	return true;
}
bool AssertFalse(const char* file, int line, bool expr, const char* exprText)
{
	if (expr)
	{
		char buf[100];
		sprintf(buf, "%s is true but expected false", exprText);
		return AssertFailure(buf, file, line);
	}

	return true;
}



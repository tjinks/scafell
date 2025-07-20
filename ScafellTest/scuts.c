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

static void *mem_alloc(size_t s) {
    void *result = malloc(s);
    if (result) return result;
    
    fprintf(stderr, "Out of memory!\n");
    exit(1);
}

scuts_test_group *scuts_tests = NULL;

scuts_test_group *scuts_add_test_group(const char *name) {
    scuts_test_group *result = mem_alloc(sizeof(scuts_test_group));
    result->name = name;
    result->init = NULL;
    result->cleanup = NULL;
    result->first_test = NULL;
    result->next = scuts_tests;
    scuts_tests = result;
    return result;
}

scuts_test *scuts_add_test(scuts_test_group *tg, const char *name, scuts_test_func test_func) {
    scuts_test *result = mem_alloc(sizeof(scuts_test));
    result->name = name;
    result->tg = tg;
    result->test_func = test_func;
    result->next = tg->first_test;
    tg->first_test = result;
    return result;
}

static scuts_test_group *find_test_group(const char *name) {
    for (scuts_test_group *tg = scuts_tests; tg; tg = tg->next) {
        if (strcmp(tg->name, name) == 0) {
            return tg;
        }
    }
    
    return NULL;
}

static scuts_test *find_test(const scuts_test_group *tg, const char *name) {
    for (scuts_test *test = tg->first_test; test; test = test->next) {
        if (strcmp(test->name, name) == 0) {
            return test;
        }
    }
    
    return NULL;
}

static void run_test(const scuts_test *test, scuts_test_run *run) {
    printf("%s...\n", test->name);
    if (test->tg->init) test->tg->init();
    bool result = test->test_func();
    if (test->tg->cleanup) test->tg->cleanup();
    run->run_count++;
    if (result) run->pass_count++;
}

static void run_test_group(const scuts_test_group *tg, scuts_test_run *run) {
    const char *name = tg->name;
    printf("Test Group: %s\n", name);
    printf("------------");
    for (int i = 0; name[i]; i++)
    {
        putc('-', stdout);
    }

    putc('\n', stdout);
    
    for (const scuts_test *test = tg->first_test; test; test = test->next) {
        run_test(test, run);
    }
}

static void run_all(scuts_test_run *run) {
    for (const scuts_test_group *tg = scuts_tests; tg; tg = tg->next) {
        run_test_group(tg, run);
    }
}

static int end_test_run(scuts_test_run *run)
{
    int failed_count = run->run_count - run->pass_count;
    printf("Tests run: %d\n", run->run_count);
    printf("Passed:    %d\n", run->pass_count);
    printf("Failed:    %d\n", failed_count);
    return failed_count ? EXIT_FAILURE : EXIT_SUCCESS;
}

int scuts(int argc, const char *argv[]) {
    scuts_test_run run = {0, 0};
    run_all(&run);
    return end_test_run(&run);
}

#if 0
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
#endif

bool assert_failure(const char *message, const char* file, int line)
{
    fprintf(stderr, "Assert failure %s at %s: %d\n", message, file, line);
    return false;
}

bool assert_eq_string(const char* file, int line, const char* expected, const char* actual)
{
    if (strcmp(expected, actual) != 0)
    {
        return assert_failure("string compare", file, line);
    }

    return true;
}

bool assert_eq_wstring(const char* file, int line, const wchar_t* expected, const wchar_t* actual)
{
    if (wcscmp(expected, actual) != 0)
    {
        return assert_failure("string compare", file, line);
    }

    return true;
}

bool assert_eq_number(const char* file, int line, long long expected, long long actual)
{
    if (expected != actual)
    {
        return assert_failure("numeric compare", file, line);
    }

    return true;
}

bool assert_true(const char* file, int line, bool expr, const char* exprText)
{
    if (!expr)
    {
        char buf[100];
        sprintf(buf, "%s is false but expected true", exprText);
        return assert_failure(buf, file, line);
    }

    return true;
}
bool assert_false(const char* file, int line, bool expr, const char* exprText)
{
    if (expr)
    {
        char buf[100];
        sprintf(buf, "%s is true but expected false", exprText);
        return assert_failure(buf, file, line);
    }

    return true;
}


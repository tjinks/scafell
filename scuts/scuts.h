#ifndef scuts_included
#define scuts_included

#include <stdio.h>
#include <stdbool.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>

typedef void (*scuts_init_func)(void);
typedef bool (*scuts_test_func)(void);

struct scuts_test;

typedef struct scuts_test_group {
    const char *name;
    scuts_init_func init;
    scuts_init_func cleanup;
    struct scuts_test_group *next;
    struct scuts_test *first_test;
} scuts_test_group;

typedef struct scuts_test {
    const char *name;
    scuts_test_group *tg;
    struct scuts_test *next;
    scuts_test_func test_func;
} scuts_test;

typedef struct {
    int run_count;
    int pass_count;
} scuts_test_run;

int scuts(int argc, const char *argv[]);

scuts_test_group *scuts_add_test_group(const char *name);

scuts_test *scuts_add_test(scuts_test_group *tg, const char *name, scuts_test_func test_func);

bool assert_failure(const char *message, const char* file, int line);

bool assert_eq_string(const char* file, int line, const char* expected, const char* actual);
bool assert_eq_wstring(const char* file, int line, const wchar_t* expected, const wchar_t* actual);
bool assert_eq_number(const char* file, int line, long long expected, long long actual);

bool assert_true(const char* file, int line, bool expr, const char* exprText);
bool assert_false(const char* file, int line, bool expr, const char* exprText);



#define BEGIN_TEST_GROUP(group_name) \
void register_##group_name(void) { \
    scuts_test_group *tg = scuts_add_test_group(#group_name);

#define INIT(init_func) tg->init = (init_func);

#define CLEANUP(cleanup_func) tg->cleanup = (cleanup_func);

#define TEST(test) scuts_add_test(tg, #test, (test));

#define END_TEST_GROUP }

#define REGISTER(tg) extern void register_##tg(void); register_##tg()

#define ASSERT_FAILURE(message) \
do { \
    return assert_failure((message), __FILE__, __LINE__); \
} while (0)

#define ASSERT_EQ(a, b) (_Generic((b), \
int: assert_eq_number(__FILE__, __LINE__, (long long)(a), (long long)(b)), \
unsigned int: assert_eq_number(__FILE__, __LINE__, (long long)(a), (long long)(b)), \
unsigned char: assert_eq_number(__FILE__, __LINE__, (long long)(a), (long long)(b)), \
char: assert_eq_number(__FILE__, __LINE__, (long long)(a), (long long)(b)), \
long: assert_eq_number(__FILE__, __LINE__, (long long)(a), (long long)(b)), \
unsigned long: assert_eq_number(__FILE__, __LINE__, (long long)(a), (long long)(b)), \
long long: assert_eq_number(__FILE__, __LINE__, (long long)(a), (long long)(b)), \
unsigned long long: assert_eq_number(__FILE__, __LINE__, (long long)(a), (long long)(b)), \
wchar_t *: assert_eq_wstring(__FILE__, __LINE__, (wchar_t *)(a), (wchar_t *)(b)), \
char *: assert_eq_string(__FILE__, __LINE__, (char *)(a), (char *)(b)) \
))

#define ASSERT_TRUE(expr) assert_true(__FILE__, __LINE__, (expr), #expr)

#define ASSERT_FALSE(expr) assert_false(__FILE__, __LINE__, (expr), #expr)

#ifdef __clang__
#pragma clang diagnostic ignored "-Wint-conversion"
#pragma clang diagnostic ignored "-Wint-to-pointer-cast"
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4312)
#endif

#endif





#pragma once

#include <cstdio>
#include <cstring>
#include <cstddef>

extern bool runDlistTests();

#define RUNTEST(name) printf("running "#name"\t"); if (test_##name()) {printf("\tOK\n"); } else {printf("\tFAIL\n"); okay=false;}

#define BEGIN_TEST bool testret = true;

#define END_TEST return testret;

#define expect(x) \
if (!(x)) { printf("Failed expectation: '"#x "'\n"); testret = false;}

#define expectEqual(a, b) \
if (!((a) == (b))) { printf("Failed expectation: '"#a " == " #b "' but saw " #a " == %d\n", a); testret = false;}

#define expectEqualPtr(a, b) \
if (!((void*)(a) == (void*)(b))) { printf("Failed expectation: '"#a " == " #b "' but saw " #a " == %p\n", a); testret = false;}


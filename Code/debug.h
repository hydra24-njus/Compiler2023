#ifdef __DEBUG__
#define debug(format,...) printf(format,##__VA_ARGS__)
#else
#define debug(format,...)
#endif
#include <assert.h>
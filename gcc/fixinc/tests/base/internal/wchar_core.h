/*  DO NOT EDIT THIS FILE.

    It has been auto-edited by fixincludes from:

	"fixinc/tests/inc/internals/wchar_core.h"

    This had to be done to correct non-standard usages in the
    original, manufacturer supplied header file.  */



#if defined( IRIX_WCSFTIME_CHECK )
#if _NO_XOPEN5 && !defined(__c99)
extern size_t          wcsftime(wchar_t *, __SGI_LIBC_NAMESPACE_QUALIFIER size_t, const char *, const struct tm *);
#endif  /* IRIX_WCSFTIME_CHECK */

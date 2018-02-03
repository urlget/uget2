noinst_PROGRAMS = \
	test-json  test-jsonrpc  test-plugin+app  test-data  \
	test-uglib  test-uget
#	test-uglib-cxx  test-uget-cxx
TESTS_LIBS = @PTHREAD_LIBS@  @CURL_LIBS@  @GLIB_LIBS@

if WITH_LIBPWMD
TESTS_LIBS += @LIBPWMD_LIBS@
endif

# set the include path found by configure
AM_CPPFLAGS = -I$(top_srcdir)/uglib   -I$(top_srcdir)/uget
AM_CFLAGS   = @PTHREAD_CFLAGS@  @LFS_CFLAGS@  @CURL_CFLAGS@  @GLIB_CFLAGS@
AM_LDFLAGS  = @LFS_LDFLAGS@

# test_json_CPPFLAGS = -I$(top_srcdir)/uglib
test_json_LDADD    = $(top_builddir)/uglib/libuglib.a  $(TESTS_LIBS)
test_json_SOURCES  = test-json.c

# test_jsonrpc_CPPFLAGS = -I$(top_srcdir)/uglib -I$(top_srcdir)/uget
test_jsonrpc_LDADD    = $(top_builddir)/uget/libuget.a $(top_builddir)/uglib/libuglib.a  $(TESTS_LIBS)
test_jsonrpc_SOURCES  = test-jsonrpc.c

test_plugin_app_CPPFLAGS = -I$(top_srcdir)/uglib -I$(top_srcdir)/uget  @LIBGCRYPT_CFLAGS@  @LIBCRYPTO_CFLAGS@
test_plugin_app_LDADD    = $(top_builddir)/uget/libuget.a $(top_builddir)/uglib/libuglib.a  $(TESTS_LIBS)  @LIBGCRYPT_LIBS@  @LIBCRYPTO_LIBS@
test_plugin_app_SOURCES  = test-plugin+app.c

# test_data_CPPFLAGS   = -I$(top_srcdir)/uglib
test_data_LDADD    = $(top_builddir)/uglib/libuglib.a  $(TESTS_LIBS)
test_data_SOURCES  = test-data.c

# test_uglib_CPPFLAGS = -I$(top_srcdir)/uglib
test_uglib_LDADD    = $(top_builddir)/uglib/libuglib.a  $(TESTS_LIBS)
test_uglib_SOURCES  = test-uglib.c

# test_uget_CPPFLAGS  = -I$(top_srcdir)/uglib -I$(top_srcdir)/uget
test_uget_LDADD     = $(top_builddir)/uget/libuget.a $(top_builddir)/uglib/libuglib.a  $(TESTS_LIBS)
test_uget_SOURCES   = test-uget.c

## test C++ standard-layout
#test_uglib_cxx_CPPFLAGS = -I$(top_srcdir)/uglib
#test_uglib_cxx_CXXFLAGS = -std=c++11
#test_uglib_cxx_LDADD    = $(top_builddir)/uglib/libuglib.a  $(TESTS_LIBS)
#test_uglib_cxx_SOURCES  = test-uglib-cxx.cxx

## test_uget_cxx_CPPFLAGS = -I$(top_srcdir)/uglib  -I$(top_srcdir)/uget
#test_uget_cxx_CXXFLAGS = -std=c++11
#test_uget_cxx_LDADD    = $(top_builddir)/uget/libuget.a $(top_builddir)/uglib/libuglib.a  $(TESTS_LIBS)
#test_uget_cxx_SOURCES  = test-uget-cxx.cxx


noinst_PROGRAMS = \
	test-json  test-jsonrpc  test-plugin+app  test-info  \
	test-uglib  test-uget
#	test-uglib-cxx  test-uget-cxx
TESTS_LIBS = @PTHREAD_LIBS@  @CURL_LIBS@  @GLIB_LIBS@

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

# test_plugin_app_CPPFLAGS = -I$(top_srcdir)/uglib -I$(top_srcdir)/uget
test_plugin_app_LDADD    = $(top_builddir)/uget/libuget.a $(top_builddir)/uglib/libuglib.a  $(TESTS_LIBS)
test_plugin_app_SOURCES  = test-plugin+app.c

# test_info_CPPFLAGS   = -I$(top_srcdir)/uglib
test_info_LDADD    = $(top_builddir)/uglib/libuglib.a  $(TESTS_LIBS)
test_info_SOURCES  = test-info.c

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


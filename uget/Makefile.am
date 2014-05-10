## To enable LFS (Large File Support) in 32bit platform
## add `getconf LFS_CFLAGS`  to CFLAGS
## add `getconf LFS_LDFLAGS` to LDFLAGS

## static library ---

#  lib_LIBRARIES = libuget.a
noinst_LIBRARIES = libuget.a

libuget_a_CPPFLAGS = -I$(top_srcdir)/uglib  -I$(top_srcdir)/uget
libuget_a_CFLAGS   = @PTHREAD_CFLAGS@  @GLIB_CFLAGS@

libuget_a_SOURCES = \
	UgetRss.c     \
	UgetIpc.c     \
	UgetData.c    \
	UgetNode.c  UgetNode-compare.c  \
	UgetTask.c    \
	UgetHash.c    \
	UgetApp.c     \
	UgetEvent.c   \
	UgetPlugin.c  \
	UgetA2cf.c    \
	UgetCurl.c    \
	UgetAria2.c   \
	UgetPluginEmpty.c  \
	UgetPluginAria2.c  \
	UgetPluginCurl.c

noinst_HEADERS = \
	UgetRss.h     \
	UgetIpc.h     \
	UgetData.h    \
	UgetNode.h    \
	UgetTask.h    \
	UgetHash.h    \
	UgetApp.h     \
	UgetEvent.h   \
	UgetPlugin.h  \
	UgetA2cf.h    \
	UgetCurl.h    \
	UgetAria2.h   \
	UgetPluginEmpty.h  \
	UgetPluginAria2.h  \
	UgetPluginCurl.h


## To enable LFS (Large File Support) in 32bit platform
## add `getconf LFS_CFLAGS`  to CFLAGS
## add `getconf LFS_LDFLAGS` to LDFLAGS

## static library ---

#  lib_LIBRARIES = libuglib.a
noinst_LIBRARIES = libuglib.a

libuglib_a_CPPFLAGS = -I$(top_srcdir)/uglib
libuglib_a_CFLAGS   = @PTHREAD_CFLAGS@  @LFS_CFLAGS@  @CURL_CFLAGS@  @GLIB_CFLAGS@

libuglib_a_SOURCES = \
	UgStdio.c  \
	UgString.c  \
	UgThread.c  \
	UgSocket.c  \
	UgUtil.c  \
	UgFileUtil.c  \
	UgArray.c  \
	UgList.c  \
	UgSLink.c  \
	UgOption.c  \
	UgUri.c  \
	UgNode.c  \
	UgData.c  \
	UgInfo.c  \
	UgRegistry.c  \
	UgValue.c  \
	UgEntry.c  \
	UgBuffer.c  \
	UgJson.c  \
	UgJson-custom.c  \
	UgJsonFile.c  \
	UgJsonrpc.c  \
	UgJsonrpcSocket.c  \
	UgJsonrpcCurl.c  \
	UgHtml.c  \
	UgHtmlEntry.c  \
	UgHtmlFilter.c

noinst_HEADERS = \
	UgDefine.h   \
	UgStdio.h   \
	UgString.h  \
	UgThread.h  \
	UgSocket.h  \
	UgUtil.h  \
	UgFileUtil.h  \
	UgArray.h  \
	UgList.h  \
	UgSLink.h  \
	UgOption.h  \
	UgUri.h  \
	UgNode.h  \
	UgData.h  \
	UgInfo.h  \
	UgRegistry.h  \
	UgValue.h  \
	UgEntry.h  \
	UgBuffer.h  \
	UgJson.h  \
	UgJson-custom.h  \
	UgJsonFile.h  \
	UgJsonrpc.h  \
	UgJsonrpcSocket.h  \
	UgJsonrpcCurl.h  \
	UgHtml.h  \
	UgHtmlEntry.h  \
	UgHtmlFilter.h

EXTRA_DIST = \
	Android.mk     \
	CMakeLists.txt

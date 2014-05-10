## To enable LFS (Large File Support) in 32bit platform
## add `getconf LFS_CFLAGS`  to CFLAGS
## add `getconf LFS_LDFLAGS` to LDFLAGS

## static library ---

#  lib_LIBRARIES = libuglib.a
noinst_LIBRARIES = libuglib.a

libuglib_a_CPPFLAGS = -I$(top_srcdir)/uglib
libuglib_a_CFLAGS   = @PTHREAD_CFLAGS@  @LFS_CFLAGS@  @CURL_CFLAGS@  @GLIB_CFLAGS@

libuglib_a_SOURCES = \
	UgJson.c  UgBuffer.c  UgValue.c  UgEntry.c  UgArray.c  UgList.c  \
	UgJson-custom.c  UgJsonFile.c  \
	UgJsonrpc.c  UgJsonrpcSocket.c  UgJsonrpcCurl.c  \
	UgData.c  UgRegistry.c  UgSLink.c  UgInfo.c  \
	UgNode.c  UgThread.c  UgStdio.c  UgUri.c  \
	UgUtil.c  UgString.c  UgSocket.c  \
	UgHtml.c  UgHtmlEntry.c  UgHtmlFilter.c  \
	UgOption.c

noinst_HEADERS = \
	UgDefine.h   \
	UgJson.h  UgBuffer.h  UgValue.h  UgEntry.h  UgArray.h  UgList.h  \
	UgJson-custom.h  UgJsonFile.h  \
	UgJsonrpc.h  UgJsonrpcSocket.h  UgJsonrpcCurl.h  \
	UgData.h  UgRegistry.h  UgSLink.h  UgInfo.h  \
	UgNode.h  UgThread.h  UgStdio.h  UgUri.h  \
	UgUtil.h  UgString.h  UgSocket.h  \
	UgHtml.h  UgHtmlEntry.h  UgHtmlFilter.h  \
	UgOption.h

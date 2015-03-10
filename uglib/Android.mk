LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE     := libuglib
LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/../curl/include
LOCAL_SRC_FILES  := \
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

include $(BUILD_STATIC_LIBRARY)

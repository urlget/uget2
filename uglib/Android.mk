LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE     := libuglib
LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/../curl/include
LOCAL_SRC_FILES  := \
	UgJson.c  UgBuffer.c  UgValue.c  UgEntry.c  UgArray.c  UgList.c  \
	UgJson-custom.c  UgJsonFile.c  \
	UgJsonrpc.c  UgJsonrpcSocket.c  UgJsonrpcCurl.c  \
	UgData.c  UgRegistry.c  UgSLink.c  UgInfo.c  \
	UgNode.c  UgThread.c  UgStdio.c  UgUri.c  \
	UgUtil.c  UgString.c  UgSocket.c  \
	UgOption.c

include $(BUILD_STATIC_LIBRARY)

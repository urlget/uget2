LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE     := libuget
LOCAL_CPPFLAGS   += -DNDEBUG
##                  -DNO_URI_HASH
##                  -DNO_RETRY_IF_CONNECT_FAILED

LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/../uglib $(LOCAL_PATH)/../curl/include
LOCAL_SRC_FILES  := \
	UgetRpc.c     \
	UgetOption.c  \
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
	UgetMedia.c   \
	UgetPluginCurl.c   \
	UgetPluginAria2.c  \
	UgetPluginMedia.c

include $(BUILD_STATIC_LIBRARY)


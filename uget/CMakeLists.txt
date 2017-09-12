cmake_minimum_required(VERSION 3.4.1)

project(uget)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNDEBUG")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNO_RETRY_IF_CONNECT_FAILED")

include_directories( ${CURL_INCLUDE_DIR}
                     ${OPENSSL_INCLUDE_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}/../uglib
                   )

add_library( uget
             STATIC

             UgetSequence.c
             UgetRpc.c
             UgetOption.c
             UgetData.c
             UgetNode.c
             UgetNode-compare.c
             UgetTask.c
             UgetHash.c
             UgetSite.c
             UgetApp.c
             UgetEvent.c
             UgetPlugin.c
             UgetA2cf.c
             UgetCurl.c
             UgetAria2.c
             UgetMedia.c
             UgetMedia-youtube.c
             UgetPluginCurl.c
             UgetPluginAria2.c
             UgetPluginMedia.c
             UgetPluginAgent.c
             UgetPluginMega.c
           )


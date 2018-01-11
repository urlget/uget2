cmake_minimum_required(VERSION 3.4.1)

project(uglib)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNDEBUG")

include_directories( ${CURL_INCLUDE_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
                   )

add_library( uglib
             STATIC

             UgStdio.c
             UgString.c
             UgThread.c
             UgSocket.c
             UgUtil.c
             UgFileUtil.c
             UgArray.c
             UgList.c
             UgSLink.c
             UgOption.c
             UgUri.c
             UgNode.c
             UgData.c
             UgInfo.c
             UgRegistry.c
             UgValue.c
             UgEntry.c
             UgBuffer.c
             UgJson.c
             UgJson-custom.c
             UgJsonFile.c
             UgJsonrpc.c
             UgJsonrpcSocket.c
             UgJsonrpcCurl.c
             UgHtml.c
             UgHtmlEntry.c
             UgHtmlFilter.c
           )


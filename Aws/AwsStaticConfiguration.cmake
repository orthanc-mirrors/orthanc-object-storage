# Cloud storage plugins for Orthanc
# Copyright (C) 2020-2021 Osimis S.A., Belgium
#
# This program is free software: you can redistribute it and/or
# modify it under the terms of the GNU Affero General Public License
# as published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.


# version numbers can be obtained from https://github.com/aws/aws-sdk-cpp/blob/main/prefetch_crt_dependency.sh

SET(AWS_C_COMMON_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-common-0.9.3)  # source =  https://github.com/awslabs/aws-c-common/archive/refs/tags/v0.9.3.tar.gz
SET(AWS_C_COMMON_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-c-common-0.9.3.tar.gz")
SET(AWS_C_COMMON_MD5 "d872ff682d5beca8ada6b19464235ca0")
DownloadPackage(${AWS_C_COMMON_MD5} ${AWS_C_COMMON_URL} "${AWS_C_COMMON_SOURCES_DIR}")

SET(AWS_CHECKSUMS_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-checksums-0.1.17)  # source =  https://github.com/awslabs/aws-checksums/archive/refs/tags/v0.1.17.tar.gz
SET(AWS_CHECKSUMS_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-checksums-0.1.17.tar.gz")
SET(AWS_CHECKSUMS_MD5 "c256144404dc74349a8344662111e353")
DownloadPackage(${AWS_CHECKSUMS_MD5} ${AWS_CHECKSUMS_URL} "${AWS_CHECKSUMS_SOURCES_DIR}")

SET(AWS_C_AUTH_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-auth-0.7.1)  # source =  https://github.com/awslabs/aws-c-auth/archive/refs/tags/v0.7.1.tar.gz
SET(AWS_C_AUTH_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-c-auth-0.7.1.tar.gz")
SET(AWS_C_AUTH_MD5 "aa690622a5f697d47ca3095ba2ef3cca")
DownloadPackage(${AWS_C_AUTH_MD5} ${AWS_C_AUTH_URL} "${AWS_C_AUTH_SOURCES_DIR}")

SET(AWS_C_CAL_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-cal-0.6.1)   # source =  https://github.com/awslabs/aws-c-cal/archive/refs/tags/v0.6.1.tar.gz
SET(AWS_C_CAL_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-c-cal-0.6.1.tar.gz")
SET(AWS_C_CAL_MD5 "6d7c05e2f1c173b923e77b6021cb660d")
DownloadPackage(${AWS_C_CAL_MD5} ${AWS_C_CAL_URL} "${AWS_C_CAL_SOURCES_DIR}")

SET(AWS_C_COMPRESSION_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-compression-0.2.17)  # source =  https://github.com/awslabs/aws-c-compression/archive/refs/tags/v0.2.17.tar.gz
SET(AWS_C_COMPRESSION_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-c-compression-0.2.17.tar.gz")
SET(AWS_C_COMPRESSION_MD5 "b8bc8bc9cf3749eb2d3f36098c9ead27")
DownloadPackage(${AWS_C_COMPRESSION_MD5} ${AWS_C_COMPRESSION_URL} "${AWS_C_COMPRESSION_SOURCES_DIR}")

SET(AWS_C_EVENT_STREAM_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-event-stream-0.3.1)  # source =  https://github.com/awslabs/aws-c-event-stream/archive/refs/tags/v0.3.1.tar.gz
SET(AWS_C_EVENT_STREAM_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-c-event-stream-0.3.1.tar.gz")
SET(AWS_C_EVENT_STREAM_MD5 "e3261d89598a3331eebcd0476b523c73")
DownloadPackage(${AWS_C_EVENT_STREAM_MD5} ${AWS_C_EVENT_STREAM_URL} "${AWS_C_EVENT_STREAM_SOURCES_DIR}")

SET(AWS_C_HTTP_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-http-0.7.11)  # source =  https://github.com/awslabs/aws-c-http/archive/refs/tags/v0.7.11.tar.gz
SET(AWS_C_HTTP_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-c-http-0.7.11.tar.gz")
SET(AWS_C_HTTP_MD5 "b8ef3268d4d64b7fabe743b03518d2d7")
DownloadPackage(${AWS_C_HTTP_MD5} ${AWS_C_HTTP_URL} "${AWS_C_HTTP_SOURCES_DIR}")

SET(AWS_C_IO_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-io-0.13.31)  # source =  https://github.com/awslabs/aws-c-io/archive/refs/tags/v0.13.31.tar.gz
SET(AWS_C_IO_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-c-io-0.13.31.tar.gz")
SET(AWS_C_IO_MD5 "9ae415cb81404e1b76d8e8267e585e34")
DownloadPackage(${AWS_C_IO_MD5} ${AWS_C_IO_URL} "${AWS_C_IO_SOURCES_DIR}")

SET(AWS_C_MQTT_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-mqtt-0.9.5)  # source =  https://github.com/awslabs/aws-c-mqtt/archive/refs/tags/v0.9.5.tar.gz
SET(AWS_C_MQTT_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-c-mqtt-0.9.5.tar.gz")
SET(AWS_C_MQTT_MD5 "76d59d9f14f3999b0f9618e386e71db6")
DownloadPackage(${AWS_C_MQTT_MD5} ${AWS_C_MQTT_URL} "${AWS_C_MQTT_SOURCES_DIR}")

SET(AWS_C_S3_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-s3-0.3.14)  # source =  https://github.com/awslabs/aws-c-s3/archive/refs/tags/v0.3.14.tar.gz
SET(AWS_C_S3_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-c-s3-0.3.14.tar.gz")
SET(AWS_C_S3_MD5 "564bb0df1184ebe9ce41c23a0c18cf1a")
DownloadPackage(${AWS_C_S3_MD5} ${AWS_C_S3_URL} "${AWS_C_S3_SOURCES_DIR}")

SET(AWS_C_SDKUTILS_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-sdkutils-0.1.11)  # source =  https://github.com/awslabs/aws-c-sdkutils/archive/refs/tags/v0.1.11.tar.gz
SET(AWS_C_SDKUTILS_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-c-sdkutils-0.1.11.tar.gz")
SET(AWS_C_SDKUTILS_MD5 "713cac3392aa20d5dda852cf21ac1957")
DownloadPackage(${AWS_C_SDKUTILS_MD5} ${AWS_C_SDKUTILS_URL} "${AWS_C_SDKUTILS_SOURCES_DIR}")

SET(AWS_CRT_CPP_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-crt-cpp-0.24.1)  # source =  https://github.com/awslabs/aws-crt-cpp/archive/refs/tags/v0.24.1.tar.gz
SET(AWS_CRT_CPP_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-crt-cpp-0.24.1.tar.gz")
SET(AWS_CRT_CPP_MD5 "fa2cda44386bd56f1d4609c6a54a59f9")
DownloadPackage(${AWS_CRT_CPP_MD5} ${AWS_CRT_CPP_URL} "${AWS_CRT_CPP_SOURCES_DIR}")

SET(AWS_SDK_CPP_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-sdk-cpp-1.11.178)  # source =  https://github.com/aws/aws-sdk-cpp/archive/refs/tags/1.11.178.tar.gz
SET(AWS_SDK_CPP_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-sdk-cpp-1.11.178.tar.gz")
SET(AWS_SDK_CPP_MD5 "a3f45888e939bb71506e0f7eaa630e48")
DownloadPackage(${AWS_SDK_CPP_MD5} ${AWS_SDK_CPP_URL} "${AWS_SDK_CPP_SOURCES_DIR}")


configure_file(
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/include/aws/core/SDKConfig.h.in
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/include/aws/core/SDKConfig.h
  )

configure_file(
  ${AWS_C_COMMON_SOURCES_DIR}/include/aws/common/config.h.in
  ${AWS_C_COMMON_SOURCES_DIR}/include/aws/common/config.h
  )

configure_file(
  ${AWS_CRT_CPP_SOURCES_DIR}/include/aws/crt/Config.h.in
  ${AWS_CRT_CPP_SOURCES_DIR}/include/aws/crt/Config.h
  )


include_directories(
  ${AWS_C_COMMON_SOURCES_DIR}/include/
  ${AWS_C_AUTH_SOURCES_DIR}/include/
  ${AWS_C_CAL_SOURCES_DIR}/include/
  ${AWS_C_COMPRESSION_SOURCES_DIR}/include/
  ${AWS_C_IO_SOURCES_DIR}/include/
  ${AWS_C_HTTP_SOURCES_DIR}/include/
  ${AWS_C_MQTT_SOURCES_DIR}/include/
  ${AWS_C_S3_SOURCES_DIR}/include/
  ${AWS_C_SDKUTILS_SOURCES_DIR}/include/
  ${AWS_C_EVENT_STREAM_SOURCES_DIR}/include/
  ${AWS_CHECKSUMS_SOURCES_DIR}/include/
  ${AWS_CRT_CPP_SOURCES_DIR}/include/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/include/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-transfer/include/
  ${AWS_SDK_CPP_SOURCES_DIR}/generated/src/aws-cpp-sdk-s3/include/
  # ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-s3/include/
  # ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-connect/include/
  )


list(APPEND AWS_SOURCES_SUBDIRS
  ${AWS_C_COMMON_SOURCES_DIR}/source/
  ${AWS_C_COMMON_SOURCES_DIR}/source/external/

  ## C libraries
  
  ${AWS_CHECKSUMS_SOURCES_DIR}/source/
  ${AWS_C_AUTH_SOURCES_DIR}/source/
  ${AWS_C_CAL_SOURCES_DIR}/source/
  ${AWS_C_COMPRESSION_SOURCES_DIR}/source/
  ${AWS_C_EVENT_STREAM_SOURCES_DIR}/source/
  ${AWS_C_HTTP_SOURCES_DIR}/source/
  ${AWS_C_IO_SOURCES_DIR}/source/
  ${AWS_C_MQTT_SOURCES_DIR}/source/
  ${AWS_C_S3_SOURCES_DIR}/source/
  ${AWS_C_SDKUTILS_SOURCES_DIR}/source/

  ## C++ libraries
  
  ${AWS_CRT_CPP_SOURCES_DIR}/source/
  ${AWS_CRT_CPP_SOURCES_DIR}/source/auth/
  ${AWS_CRT_CPP_SOURCES_DIR}/source/crypto/
  ${AWS_CRT_CPP_SOURCES_DIR}/source/endpoints/
  ${AWS_CRT_CPP_SOURCES_DIR}/source/http/
  ${AWS_CRT_CPP_SOURCES_DIR}/source/io/
  ${AWS_CRT_CPP_SOURCES_DIR}/source/external/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-connect/source/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-connect/source/model/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/auth/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/auth/signer/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/auth/signer-provider/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/auth/bearer-token-provider/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/client/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/config/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/config/defaults/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/endpoint/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/endpoint/internal/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/external/cjson/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/external/tinyxml2/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/http/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/http/curl/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/http/standard/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/internal/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/monitoring/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/net/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/platform/linux-shared/
  # ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/platform/windows/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/smithy/tracing/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/base64/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/component-registry/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/crypto/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/crypto/factory/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/crypto/openssl/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/event/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/json/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/logging/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/memory/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/memory/stl/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/stream/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/threading/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/xml/
  ${AWS_SDK_CPP_SOURCES_DIR}/generated/src/aws-cpp-sdk-s3/source/
  ${AWS_SDK_CPP_SOURCES_DIR}/generated/src/aws-cpp-sdk-s3/source/model/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-transfer/source/transfer/
  )


add_definitions(
  -DAWS_AFFINITY_METHOD=AWS_AFFINITY_METHOD_PTHREAD
  -DBYO_CRYPTO  # To have "aws_tls_server_ctx_new()" defined
  -DENABLE_OPENSSL_ENCRYPTION=1
  -DENABLE_CURL_CLIENT=1
  )

list(APPEND AWS_SOURCES_SUBDIRS
  #${AWS_C_CAL_SOURCES_DIR}/source/unix/
  #${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/net/linux-shared/
  ${AWS_C_COMMON_SOURCES_DIR}/source/posix/
  ${AWS_C_IO_SOURCES_DIR}/source/linux/
  ${AWS_C_IO_SOURCES_DIR}/source/posix/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/platform/linux-shared/
  )


foreach(d ${AWS_SOURCES_SUBDIRS})
  aux_source_directory(${d} AWS_SOURCES)
endforeach()


list(APPEND AWS_SOURCES
  ${AWS_C_COMMON_SOURCES_DIR}/source/arch/generic/cpuid.c
  ${AWS_CHECKSUMS_SOURCES_DIR}/source/generic/crc32c_null.c
  )

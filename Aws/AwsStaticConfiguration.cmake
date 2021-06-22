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


SET(AWS_C_COMMON_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-common-0.6.2)
SET(AWS_C_COMMON_URL "http://orthanc.osimis.io/ThirdPartyDownloads/aws/aws-c-common-0.6.2.tar.gz")
SET(AWS_C_COMMON_MD5 "aa6dc4bde2d035ad84ec40e3ea1e2bff")
DownloadPackage(${AWS_C_COMMON_MD5} ${AWS_C_COMMON_URL} "${AWS_C_COMMON_SOURCES_DIR}")

SET(AWS_CHECKSUMS_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-checksums-0.1.11)
SET(AWS_CHECKSUMS_URL "http://orthanc.osimis.io/ThirdPartyDownloads/aws/aws-checksums-0.1.11.tar.gz")
SET(AWS_CHECKSUMS_MD5 "4ec8e8065d0d3cd53c86e16692e79dcb")
DownloadPackage(${AWS_CHECKSUMS_MD5} ${AWS_CHECKSUMS_URL} "${AWS_CHECKSUMS_SOURCES_DIR}")

SET(AWS_C_AUTH_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-auth-0.6.0)
SET(AWS_C_AUTH_URL "http://orthanc.osimis.io/ThirdPartyDownloads/aws/aws-c-auth-0.6.0.tar.gz")
SET(AWS_C_AUTH_MD5 "e82d991dbd1c1acee210d4ac1fd518dc")
DownloadPackage(${AWS_C_AUTH_MD5} ${AWS_C_AUTH_URL} "${AWS_C_AUTH_SOURCES_DIR}")

SET(AWS_C_CAL_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-cal-0.5.11)
SET(AWS_C_CAL_URL "http://orthanc.osimis.io/ThirdPartyDownloads/aws/aws-c-cal-0.5.11.tar.gz")
SET(AWS_C_CAL_MD5 "2099fc945a02feda56b8ecd1ea77acf9")
DownloadPackage(${AWS_C_CAL_MD5} ${AWS_C_CAL_URL} "${AWS_C_CAL_SOURCES_DIR}")

SET(AWS_C_COMPRESSION_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-compression-0.2.13)
SET(AWS_C_COMPRESSION_URL "http://orthanc.osimis.io/ThirdPartyDownloads/aws/aws-c-compression-0.2.13.tar.gz")
SET(AWS_C_COMPRESSION_MD5 "e96ed91a832116a05a60e49e9946623e")
DownloadPackage(${AWS_C_COMPRESSION_MD5} ${AWS_C_COMPRESSION_URL} "${AWS_C_COMPRESSION_SOURCES_DIR}")

SET(AWS_C_EVENT_STREAM_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-event-stream-0.2.7)
SET(AWS_C_EVENT_STREAM_URL "http://orthanc.osimis.io/ThirdPartyDownloads/aws/aws-c-event-stream-0.2.7.tar.gz")
SET(AWS_C_EVENT_STREAM_MD5 "5cf71f97962e22c4bc7fd75b9a8daf49")
DownloadPackage(${AWS_C_EVENT_STREAM_MD5} ${AWS_C_EVENT_STREAM_URL} "${AWS_C_EVENT_STREAM_SOURCES_DIR}")

SET(AWS_C_HTTP_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-http-0.6.4)
SET(AWS_C_HTTP_URL "http://orthanc.osimis.io/ThirdPartyDownloads/aws/aws-c-http-0.6.4.tar.gz")
SET(AWS_C_HTTP_MD5 "7ac847811ff27f1671b31514e3493913")
DownloadPackage(${AWS_C_HTTP_MD5} ${AWS_C_HTTP_URL} "${AWS_C_HTTP_SOURCES_DIR}")

SET(AWS_C_IO_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-io-0.10.5)
SET(AWS_C_IO_URL "http://orthanc.osimis.io/ThirdPartyDownloads/aws/aws-c-io-0.10.5.tar.gz")
SET(AWS_C_IO_MD5 "2a450a918184aaf7e80afc80b515397b")
DownloadPackage(${AWS_C_IO_MD5} ${AWS_C_IO_URL} "${AWS_C_IO_SOURCES_DIR}")

SET(AWS_C_MQTT_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-mqtt-0.7.6)
SET(AWS_C_MQTT_URL "http://orthanc.osimis.io/ThirdPartyDownloads/aws/aws-c-mqtt-0.7.6.tar.gz")
SET(AWS_C_MQTT_MD5 "29aca202d29b9934f630a204f6731031")
DownloadPackage(${AWS_C_MQTT_MD5} ${AWS_C_MQTT_URL} "${AWS_C_MQTT_SOURCES_DIR}")

SET(AWS_C_S3_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-s3-0.1.19)
SET(AWS_C_S3_URL "http://orthanc.osimis.io/ThirdPartyDownloads/aws/aws-c-s3-0.1.19.tar.gz")
SET(AWS_C_S3_MD5 "ae5e98f70075fd40093af856b72a04fb")
DownloadPackage(${AWS_C_S3_MD5} ${AWS_C_S3_URL} "${AWS_C_S3_SOURCES_DIR}")

SET(AWS_CRT_CPP_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-crt-cpp-0.14.0)
SET(AWS_CRT_CPP_URL "http://orthanc.osimis.io/ThirdPartyDownloads/aws/aws-crt-cpp-0.14.0.tar.gz")
SET(AWS_CRT_CPP_MD5 "4298ab05b95380f960c7b0184df5c078")
DownloadPackage(${AWS_CRT_CPP_MD5} ${AWS_CRT_CPP_URL} "${AWS_CRT_CPP_SOURCES_DIR}")

SET(AWS_SDK_CPP_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-sdk-cpp-1.9.45)
SET(AWS_SDK_CPP_URL "http://orthanc.osimis.io/ThirdPartyDownloads/aws/aws-sdk-cpp-1.9.45.tar.gz")
SET(AWS_SDK_CPP_MD5 "f2f2d48b119fda2ef17d2604c0cca52e")
DownloadPackage(${AWS_SDK_CPP_MD5} ${AWS_SDK_CPP_URL} "${AWS_SDK_CPP_SOURCES_DIR}")


configure_file(
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/include/aws/core/SDKConfig.h.in
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/include/aws/core/SDKConfig.h
  )

configure_file(
  ${AWS_C_COMMON_SOURCES_DIR}/include/aws/common/config.h.in
  ${AWS_C_COMMON_SOURCES_DIR}/include/aws/common/config.h
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
  ${AWS_C_EVENT_STREAM_SOURCES_DIR}/include/
  ${AWS_CHECKSUMS_SOURCES_DIR}/include/
  ${AWS_CRT_CPP_SOURCES_DIR}/include/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/include/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-s3/include/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-connect/include/
  )


list(APPEND AWS_SOURCES_SUBDIRS
  ${AWS_C_COMMON_SOURCES_DIR}/source/

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

  ## C++ libraries
  
  ${AWS_CRT_CPP_SOURCES_DIR}/source/
  ${AWS_CRT_CPP_SOURCES_DIR}/source/auth/
  ${AWS_CRT_CPP_SOURCES_DIR}/source/crypto/
  ${AWS_CRT_CPP_SOURCES_DIR}/source/http/
  ${AWS_CRT_CPP_SOURCES_DIR}/source/io/
  ${AWS_CRT_CPP_SOURCES_DIR}/source/external/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-connect/source/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-connect/source/model/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/auth/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/client/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/config/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/external/cjson/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/external/tinyxml2/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/http/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/http/curl/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/http/standard/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/internal/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/monitoring/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/net/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/utils/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/utils/base64/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/utils/crypto/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/utils/crypto/factory/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/utils/crypto/openssl/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/utils/event/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/utils/json/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/utils/logging/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/utils/memory/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/utils/stream/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/utils/threading/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/utils/xml/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-s3/source/
  ${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-s3/source/model/
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

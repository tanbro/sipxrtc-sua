cmake_minimum_required(VERSION 3.15)

if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)


# pkg-config 是必须的!
find_package(PkgConfig REQUIRED)

# If the use of the -pthread compiler and linker flag is preferred then the caller can set this variable to TRUE.
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

# 导出 Clang compile_commands.json 给 IDE 使用
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 全局语言标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(
  WITH_PJ_STATIC
  "Link to libpjproject's shared/dynamic objects, which requires a configure with `--enable-shared`  and `sudo make install`"
  ON
)

option(
  WITH_OPENSSL
  "Whether to use openssl library. When static link pjproject, and openssl is enabled in it, the option must be \"ON\" here"
  ON
)

option(
  WITH_G729
  "Whether to use libbcg729. When static link pjproject, and the audio codec is enabled in it, the option must be \"ON\" here"
  ON
)
option(
  WITH_G729_STATIC
  "Whether to link libbcg729 staticly. Only avaiable when `WITH_G729` is \"ON\""
  ON
)

option(
  WITH_OPUS
  "Whether to use libopus. When static link pjproject, and the audio codec is enabled in it, the option must be \"ON\" here!"
  OFF
)

execute_process (
  COMMAND git describe
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  OUTPUT_VARIABLE GIT_DESCRIBE_OUTPUT
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process (
  COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  OUTPUT_VARIABLE GIT_REV_PARSE_OUTPUT
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# 项目：这个执行文件
project(sipxsua)

string(TOLOWER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME_LOWERCASE)
string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} SYSTEM_PROCESSOR_LOWERCASE)
string(TOLOWER ${CMAKE_CXX_COMPILER_ID} CXX_COMPILER_ID_LOWERCASE)

# source 目录
set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
# 源文件列表
set (SOURCES
  ${SRC_DIR}/main.cc
  ${SRC_DIR}/global.cc
  ${SRC_DIR}/SipXAccount.cc
  ${SRC_DIR}/SipXCall.cc
  ${SRC_DIR}/UdsBase.cc
  ${SRC_DIR}/UdsReader.cc
  ${SRC_DIR}/UdsWriter.cc
  ${SRC_DIR}/AudioMediaUdsReader.cc
  ${SRC_DIR}/AudioMediaUdsWriter.cc
  ${SRC_DIR}/SuaLogWriter.cc
  ${SRC_DIR}/Poller.cc
  ${SRC_DIR}/version.cc
)

# 加构建目标: 可执行
add_executable(${PROJECT_NAME} ${SOURCES})

# include 目录
target_include_directories(${PROJECT_NAME}
  PRIVATE ${SRC_DIR}
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/private
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include/public
)

if (GIT_DESCRIBE_OUTPUT)
  target_compile_definitions(${PROJECT_NAME}
    PRIVATE __GIT_DESC__="${GIT_DESCRIBE_OUTPUT}"
  )
endif ()
if (GIT_REV_PARSE_OUTPUT)
  target_compile_definitions(${PROJECT_NAME}
  PRIVATE __GIT_REV__="${GIT_REV_PARSE_OUTPUT}"
  )
endif ()

# output dir, name, rpath, ...
set(OUTDIR ${CMAKE_CURRENT_LIST_DIR}/out/${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_CXX_COMPILER_ID}/${CMAKE_BUILD_TYPE})
set_target_properties(${PROJECT_NAME}
    PROPERTIES
    # -Wl,-rpath
    BUILD_RPATH ../lib
    # 输出目录
    RUNTIME_OUTPUT_DIRECTORY ${OUTDIR}/bin
)

# libpjproject
if (${WITH_PJ_STATIC})
  message("Static link pjproject!")
  # libpjproject ， 使用本地 submodules/pjproject 中的静态库
  if (PJDIR)
    message("PJDIR was set explicitly: \"${PJDIR}\"")
  else ()
    if (DEFINED ENV{PJDIR})
      set(PJDIR $ENV{PJDIR})
      message("PJDIR was set from environment: \"${PJDIR}\"")
    else ()
      set(PJDIR ${CMAKE_CURRENT_LIST_DIR}/submodules/pjproject)
      message("Use default PJDIR: \"${PJDIR}\"")
    endif ()
  endif()

  # 参照其 configure 的 build.mak 文件
  # as: export TARGET_NAME := x86_64-unknown-linux-gnu
  set(PJ_TARGET_NAME ${SYSTEM_PROCESSOR_LOWERCASE}-unknown-${SYSTEM_NAME_LOWERCASE}-${CXX_COMPILER_ID_LOWERCASE})
  set(PJ_LIB_SUFFIX ${PJ_TARGET_NAME}.a)

  set(PJ_CFLAGS
    -DPJ_AUTOCONF=1
    -DPJ_IS_BIG_ENDIAN=0
    -DPJ_IS_LITTLE_ENDIAN=1
  )

  set(PJ_INCLUDE_DIRS
    ${PJDIR}/pjlib/include
    ${PJDIR}/pjlib-util/include
    ${PJDIR}/pjnath/include
    ${PJDIR}/pjmedia/include
    ${PJDIR}/pjsip/include
  )

  set(PJ_LIB_FILES
    ${PJDIR}/pjsip/lib/libpjsua2-${PJ_LIB_SUFFIX}
    ${PJDIR}/pjsip/lib/libpjsua-${PJ_LIB_SUFFIX}
    ${PJDIR}/pjsip/lib/libpjsip-ua-${PJ_LIB_SUFFIX}
    ${PJDIR}/pjsip/lib/libpjsip-simple-${PJ_LIB_SUFFIX}
    ${PJDIR}/pjsip/lib/libpjsip-${PJ_LIB_SUFFIX}
    ${PJDIR}/pjmedia/lib/libpjmedia-codec-${PJ_LIB_SUFFIX}
    ${PJDIR}/pjmedia/lib/libpjmedia-videodev-${PJ_LIB_SUFFIX}
    ${PJDIR}/pjmedia/lib/libpjmedia-${PJ_LIB_SUFFIX}
    ${PJDIR}/pjmedia/lib/libpjmedia-audiodev-${PJ_LIB_SUFFIX}
    ${PJDIR}/pjnath/lib/libpjnath-${PJ_LIB_SUFFIX}
    ${PJDIR}/pjlib-util/lib/libpjlib-util-${PJ_LIB_SUFFIX}
    ${PJDIR}/pjlib/lib/libpj-${PJ_LIB_SUFFIX}
  )

  set(PJ_THIRD_PARTY_LIB_FILES
    ${PJDIR}/third_party/lib/libsrtp-${PJ_LIB_SUFFIX}
    ${PJDIR}/third_party/lib/libresample-${PJ_LIB_SUFFIX}
    ${PJDIR}/third_party/lib/libgsmcodec-${PJ_LIB_SUFFIX}
    ${PJDIR}/third_party/lib/libspeex-${PJ_LIB_SUFFIX}
    ${PJDIR}/third_party/lib/libilbccodec-${PJ_LIB_SUFFIX}
    ${PJDIR}/third_party/lib/libg7221codec-${PJ_LIB_SUFFIX}
    # ${PJDIR}/third_party/lib/libyuv-${PJ_LIB_SUFFIX}
    # ${PJDIR}/third_party/lib/libwebrtc-${PJ_LIB_SUFFIX}
    # ${PJDIR}/third_party/lib/libwebrtc-aec3-${PJ_LIB_SUFFIX}
  )

  list(APPEND PJ_LIB_FILES ${PJ_THIRD_PARTY_LIB_FILES})
  # list(APPEND PJ_LIB_FILES ssl crypto uuid m rt pthread)

  target_compile_options(${PROJECT_NAME} PRIVATE ${PJ_CFLAGS})
  target_include_directories(${PROJECT_NAME} PRIVATE ${PJ_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME} ${PJ_LIB_FILES})

  # pjproject Requires pthread
  find_package(Threads REQUIRED)
  target_link_libraries(${PROJECT_NAME} Threads::Threads)

  # pjproject Requires UUID
  pkg_check_modules(UUID REQUIRED uuid)
  target_link_libraries(${PROJECT_NAME} ${UUID_LIBRARIES})
  target_include_directories(${PROJECT_NAME} PRIVATE ${UUID_INCLUDE_DIRS})
  target_compile_options(${PROJECT_NAME} PRIVATE ${UUID_CFLAGS})
  target_link_directories(${PROJECT_NAME} PRIVATE ${UUID_LIBRARY_DIRS})
  target_link_options(${PROJECT_NAME} PRIVATE ${UUID_LDFLAGS})

  # openssl
  if (${WITH_OPENSSL})
    # openssl-libssl
    pkg_check_modules(LIBSSL REQUIRED libssl)
    target_link_libraries(${PROJECT_NAME} ${LIBSSL_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${LIBSSL_INCLUDE_DIRS})
    target_compile_options(${PROJECT_NAME} PRIVATE ${LIBSSL_CFLAGS})
    target_link_directories(${PROJECT_NAME} PRIVATE ${LIBSSL_LIBRARY_DIRS})
    target_link_options(${PROJECT_NAME} PRIVATE ${LIBSSL_LDFLAGS})
    # openssl-libcrypto
    pkg_check_modules(LIBCRYPTO REQUIRED libcrypto)
    target_link_libraries(${PROJECT_NAME} ${LIBCRYPTO_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${LIBCRYPTO_INCLUDE_DIRS})
    target_compile_options(${PROJECT_NAME} PRIVATE ${LIBCRYPTO_CFLAGS})
    target_link_directories(${PROJECT_NAME} PRIVATE ${LIBCRYPTO_LIBRARY_DIRS})
    target_link_options(${PROJECT_NAME} PRIVATE ${LIBCRYPTO_LDFLAGS})
  endif ()

  # g729
  if (${WITH_G729})
    pkg_check_modules(LIBBCG729 REQUIRED libbcg729)
    if (${WITH_G729_STATIC})
      target_link_libraries(${PROJECT_NAME} ${LIBBCG729_LIBRARIES}.a)
    else()
      target_link_libraries(${PROJECT_NAME} ${LIBBCG729_LIBRARIES})
      target_include_directories(${PROJECT_NAME} PRIVATE ${LIBBCG729_INCLUDE_DIRS})
      target_compile_options(${PROJECT_NAME} PRIVATE ${LIBBCG729_CFLAGS})
      target_link_directories(${PROJECT_NAME} PRIVATE ${LIBBCG729_LIBRARY_DIRS})
      target_link_options(${PROJECT_NAME} PRIVATE ${LIBBCG729_LDFLAGS})
    endif()
  endif()

  # opus
  if (${WITH_OPUS})
    pkg_check_modules(OPUS REQUIRED opus)
    target_link_libraries(${PROJECT_NAME} ${OPUS_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${OPUS_INCLUDE_DIRS})
    target_compile_options(${PROJECT_NAME} PRIVATE ${OPUS_CFLAGS})
    target_link_directories(${PROJECT_NAME} PRIVATE ${OPUS_LIBRARY_DIRS})
    target_link_options(${PROJECT_NAME} PRIVATE ${OPUS_LDFLAGS})
  endif()

else ()

  # 动态链接 PJ，需要把它安装到操作系统，提供 pc
  pkg_check_modules(LIBPJPROJECT REQUIRED libpjproject)
  target_link_libraries(${PROJECT_NAME} ${LIBPJPROJECT_LIBRARIES})
  target_include_directories(${PROJECT_NAME} PRIVATE ${LIBPJPROJECT_INCLUDE_DIRS})
  target_compile_options(${PROJECT_NAME} PRIVATE ${LIBPJPROJECT_CFLAGS})
  target_link_directories(${PROJECT_NAME} PRIVATE ${LIBPJPROJECT_LIBRARY_DIRS})
  target_link_options(${PROJECT_NAME} PRIVATE ${LIBPJPROJECT_LDFLAGS})

endif()

# dependency of this project: libsamplerate
pkg_check_modules(SAMPLERATE REQUIRED samplerate)
target_link_libraries(${PROJECT_NAME} ${SAMPLERATE_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${SAMPLERATE_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME} PRIVATE ${SAMPLERATE_CFLAGS})
target_link_directories(${PROJECT_NAME} PRIVATE ${SAMPLERATE_LIBRARY_DIRS})
target_link_options(${PROJECT_NAME} PRIVATE ${SAMPLERATE_LDFLAGS})

# dependency of this project: gflags
pkg_check_modules(GFLAGS REQUIRED gflags)
target_link_libraries(${PROJECT_NAME} ${GFLAGS_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${GFLAGS_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME} PRIVATE ${GFLAGS_CFLAGS})
target_link_directories(${PROJECT_NAME} PRIVATE ${GFLAGS_LIBRARY_DIRS})
target_link_options(${PROJECT_NAME} PRIVATE ${GFLAGS_LDFLAGS})

# dependency of this project: glog
pkg_check_modules(LIBGLOG REQUIRED libglog)
target_link_libraries(${PROJECT_NAME} ${LIBGLOG_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${LIBGLOG_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME} PRIVATE ${LIBGLOG_CFLAGS})
target_link_directories(${PROJECT_NAME} PRIVATE ${LIBGLOG_LIBRARY_DIRS})
target_link_options(${PROJECT_NAME} PRIVATE ${LIBGLOG_LDFLAGS})

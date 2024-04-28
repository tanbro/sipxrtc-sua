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
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(
  PJPROJECT_SHARED
  "When link to pjproject shared, it should configure with `--enable-shared`"
  OFF
)

option(
  CMAKE_FIND_GFLAGS
  "Whether to find gflags with cmake. If not, try to find it with pkg-config"
  OFF
)
set(
  CMAKE_FIND_GFLAGS_TARGET "" CACHE STRING
  "To request a particular imported gflags library target to link against. Only availvable when `CMAKE_FIND_GFLAGS` is ON."
)

option(
  GFLAGS_SHARED
  "link gflags static with `pkg-config --libs --statc`"
  ON
)

option(
  CMAKE_FIND_GLOG
  "Whether to find glog with cmake. If not, try to find it with pkg-config"
  OFF
)

option(
  GLOG_SHARED
  "link glog static with `pkg-config --libs --statc`"
  ON
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

# output dir, name, rpath, ...
if (OUTDIR)
  message("OUTDIR was set explicitly: \"${OUTDIR}\"")
else ()
  if (${CMAKE_SYSTEM_NAME} STREQUAL Linux)
    execute_process (
      COMMAND bash -c "source /etc/os-release && echo $ID-$VERSION_ID"
      OUTPUT_STRIP_TRAILING_WHITESPACE
      OUTPUT_VARIABLE DISTRO_VERSION
    )
    set(OUTDIR ${CMAKE_CURRENT_LIST_DIR}/out/${DISTRO_VERSION}/${CMAKE_SYSTEM_PROCESSOR}/${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}/${CMAKE_BUILD_TYPE})
  else ()
    message(FATAL_ERROR "${CMAKE_SYSTEM_NAME} is NOT supported yet.")
  endif ()
  message("OUTDIR: \"${OUTDIR}\"")
endif ()

# source 目录
set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
# 源文件列表
set (SOURCES
  ${SRC_DIR}/main.cc
  ${SRC_DIR}/SipXAccount.cc
  ${SRC_DIR}/SipXCall.cc
  ${SRC_DIR}/UdsBase.cc
  ${SRC_DIR}/UdsReader.cc
  ${SRC_DIR}/UdsWriter.cc
  ${SRC_DIR}/EventPub.cc
  ${SRC_DIR}/AudioMediaUdsReader.cc
  ${SRC_DIR}/AudioMediaUdsWriter.cc
  ${SRC_DIR}/SuaLogWriter.cc
  ${SRC_DIR}/Poller.cc
  ${SRC_DIR}/global.cc
  ${SRC_DIR}/AppFlags.cc
  ${SRC_DIR}/IpcFlags.cc
  ${SRC_DIR}/SipFlags.cc
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

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    # -Wl,-rpath
    BUILD_RPATH ../lib
    # 输出目录
    RUNTIME_OUTPUT_DIRECTORY ${OUTDIR}/bin
)

# libpjproject
pkg_check_modules(LIBPJPROJECT REQUIRED libpjproject)
if (${PJPROJECT_SHARED})
  target_link_libraries(${PROJECT_NAME} ${LIBPJPROJECT_LIBRARIES})
else()
  target_link_libraries(${PROJECT_NAME} ${LIBPJPROJECT_STATIC_LIBRARIES})
endif()
target_include_directories(${PROJECT_NAME} PRIVATE ${LIBPJPROJECT_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME} PRIVATE ${LIBPJPROJECT_CFLAGS})
target_link_directories(${PROJECT_NAME} PRIVATE ${LIBPJPROJECT_LIBRARY_DIRS})
target_link_options(${PROJECT_NAME} PRIVATE ${LIBPJPROJECT_LDFLAGS})

# dependency of this project: libsamplerate
pkg_check_modules(SAMPLERATE REQUIRED samplerate)
target_link_libraries(${PROJECT_NAME} ${SAMPLERATE_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${SAMPLERATE_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME} PRIVATE ${SAMPLERATE_CFLAGS})
target_link_directories(${PROJECT_NAME} PRIVATE ${SAMPLERATE_LIBRARY_DIRS})
target_link_options(${PROJECT_NAME} PRIVATE ${SAMPLERATE_LDFLAGS})

# dependency of this project: gflags
if (${CMAKE_FIND_GFLAGS})
  if ("${CMAKE_FIND_GFLAGS_TARGET}" STREQUAL "")
    find_package(gflags REQUIRED)
  else ()
    find_package(gflags REQUIRED COMPONENTS "${CMAKE_FIND_GFLAGS_TARGET}")
  endif()
  target_link_libraries(${PROJECT_NAME} gflags)
else ()
  pkg_check_modules(GFLAGS REQUIRED gflags)
  if (${GFLAGS_SHARED})
    target_link_libraries(${PROJECT_NAME} ${GFLAGS_LIBRARIES})
  else()
    target_link_libraries(${PROJECT_NAME} ${GFLAGS_STATIC_LIBRARIES})
  endif()
  target_include_directories(${PROJECT_NAME} PRIVATE ${GFLAGS_INCLUDE_DIRS})
  target_compile_options(${PROJECT_NAME} PRIVATE ${GFLAGS_CFLAGS})
  target_link_directories(${PROJECT_NAME} PRIVATE ${GFLAGS_LIBRARY_DIRS})
  target_link_options(${PROJECT_NAME} PRIVATE ${GFLAGS_LDFLAGS})
endif()

# dependency of this project: glog
if (${CMAKE_FIND_GLOG})
  find_package (glog REQUIRED)
  target_link_libraries (${PROJECT_NAME} glog::glog)
else ()
  pkg_check_modules(LIBGLOG REQUIRED libglog)
  if (${GLOG_SHARED})
    target_link_libraries(${PROJECT_NAME} ${LIBGLOG_LIBRARIES})
  else()
    target_link_libraries(${PROJECT_NAME} ${LIBGLOG_STATIC_LIBRARIES})
  endif()
  target_include_directories(${PROJECT_NAME} PRIVATE ${LIBGLOG_INCLUDE_DIRS})
  target_compile_options(${PROJECT_NAME} PRIVATE ${LIBGLOG_CFLAGS})
  target_link_directories(${PROJECT_NAME} PRIVATE ${LIBGLOG_LIBRARY_DIRS})
  target_link_options(${PROJECT_NAME} PRIVATE ${LIBGLOG_LDFLAGS})
endif()

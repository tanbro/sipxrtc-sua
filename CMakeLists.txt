cmake_minimum_required(VERSION 3.15)

if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

# 导出 Clang compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# 全局语言标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# if (${CMAKE_COMPILER_IS_GNU_CXX})
#   add_compile_options(-fsigned-char)
# endif ()

add_compile_options("$<$<CONFIG:DEBUG>:-D_DEBUG>")

# pkg-config 是必须的!
find_package(PkgConfig REQUIRED)

# 项目：这个执行文件
project(main)

# source 目录
set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
# 源文件列表
set (SOURCES
    ${SRC_DIR}/main.cxx
    ${SRC_DIR}/myaccount.cxx
    ${SRC_DIR}/mycall.cxx
)

# 加构建目标: 可执行
add_executable(${PROJECT_NAME} ${SOURCES})

# include 目录
target_include_directories(${PROJECT_NAME}
  PRIVATE "${CMAKE_CURRENT_LIST_DIR}/include/private"
  PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include/public"
)

# output dir, name, rpath, ...
set_target_properties(${PROJECT_NAME}
    PROPERTIES
    BUILD_RPATH lib
    # 输出目录
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin/${CMAKE_BUILD_TYPE}
)

# libpjproject
pkg_check_modules(PJPROJECT REQUIRED libpjproject)
target_include_directories(${PROJECT_NAME} PRIVATE ${PJPROJECT_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME} PRIVATE ${PJPROJECT_CFLAGS})
target_link_libraries(${PROJECT_NAME} ${PJPROJECT_LIBRARIES})
target_link_directories(${PROJECT_NAME} PRIVATE ${PJPROJECT_LIBRARY_DIRS})
target_link_options(${PROJECT_NAME} PRIVATE ${PJPROJECT_LDFLAGS_OTHER})

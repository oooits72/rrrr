cmake_minimum_required(VERSION 2.8.4)
project(ansi)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra -Wall -ansi -pedantic -ggdb3 -DRRRR_VALGRIND -DRRRR_STRICT -O3 -march=native")
if("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Weverything")
endif("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Weverything")
endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")


SET(PROTO_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/gtfs-realtime.proto
)

SET(PROTO_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/gtfs-realtime.pb-c.h
)

SET(PROTO_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/gtfs-realtime.pb-c.c
)

add_custom_command(OUTPUT ${PROTO_HDRS} ${PROTO_SRCS}
    COMMAND protoc-c ARGS ${PROTO_FILES} -I ${CMAKE_CURRENT_SOURCE_DIR}
    --c_out=${CMAKE_CURRENT_SOURCE_DIR}
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/." VERBATIM
    DEPENDS ${PROTO_FILES})

ENABLE_TESTING()

set(SOURCE_FILES
    api.c
    api.h
    bitset.c
    bitset.h
    cli.c
    config.h
    geometry.c
    geometry.h
    gtfs-realtime.pb-c.c
    gtfs-realtime.pb-c.h
    hashgrid.c
    hashgrid.h
    json.c
    json.h
    linkedlist.c
    linkedlist.h
    plan_render_otp.c
    plan_render_otp.h
    plan_render_text.c
    plan_render_text.h
    polyline.c
    polyline.h
    radixtree.c
    radixtree.h
    router.c
    router.h
    router_dump.c
    router_dump.h
    router_request.c
    router_request.h
    router_result.c
    router_result.h
    rrrr_types.h
    set.c
    set.h
    string_pool.c
    string_pool.h
    tdata.c
    tdata.h
    tdata_io_v4.h
    tdata_io_v4_dynamic.c
    tdata_io_v4_mmap.c
    tdata_realtime_alerts.c
    tdata_realtime_alerts.h
    tdata_realtime_expanded.c
    tdata_realtime_expanded.h
    tdata_validation.c
    tdata_validation.h
    util.c
    util.h)

include_directories(/usr/local/include)

link_directories(/usr/local/lib)
link_libraries(m)
link_libraries(protobuf-c)

add_executable(cli ${SOURCE_FILES})

add_subdirectory(tests)

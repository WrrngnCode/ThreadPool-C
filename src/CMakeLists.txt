
# Based on:
# https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html
# https://cmake.org/cmake/help/latest/guide/tutorial/Adding%20Export%20Configuration.html

add_library(${THREADPOOL} STATIC "threadpool.c" "threadpool.h")

if(${THREADPOOL_DEBUG} STREQUAL "ON")
    # add -DTHREADPOOL_DEBUG compile definitions in debug mode only
    target_compile_definitions(${THREADPOOL} PRIVATE "$<$<CONFIG:Debug>:THREADPOOL_DEBUG>")
endif()

target_link_libraries(${THREADPOOL}
    PRIVATE $<BUILD_INTERFACE:compiler_warnings>
    PUBLIC $<BUILD_INTERFACE:pthread>
)

target_include_directories(${THREADPOOL} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> # cmake will prepend CMAKE_INSTALL_PREFIX
)

install(TARGETS ${THREADPOOL} compiler_warnings
    EXPORT ThreadPoolTargets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/${THREADPOOL}
)

install(FILES
    ${PROJECT_SOURCE_DIR}/src/threadpool.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${THREADPOOL}
)
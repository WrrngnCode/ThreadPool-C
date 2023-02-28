set(TEST_BASIC "test_basic")
set(TEST_HEAVY "test_heavy")
set(TEST_SHUTDOWN "test_shutdown")
set(TEST_SHUTDOWN_2 "test_shutdown_2")
set(TEST_TIMEOUT_FORCED_SHUTDOWN "test_timeout_forced_shutdown")
set(TEST_TIMEOUT_GRACEFUL_SHUTDOWN "test_timeout_graceful_shutdown")

#find_package(ThreadPool 1.0.0)
#MESSAGE(STATUS ThreadPool_FOUND = ${ThreadPool_FOUND})

list(APPEND tests
    ${TEST_BASIC}
    ${TEST_HEAVY}
    ${TEST_SHUTDOWN}
    ${TEST_SHUTDOWN_2}
    ${TEST_TIMEOUT_FORCED_SHUTDOWN}
    ${TEST_TIMEOUT_GRACEFUL_SHUTDOWN})

foreach(testname IN LISTS tests)
    # Executable
    add_executable(${testname} "${testname}.cpp")

    set_target_properties(${testname} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out/test
        RUNTIME_OUTPUT_NAME ${testname}
    )

    target_include_directories(${testname}
        PRIVATE ${PROJECT_SOURCE_DIR}/src
    )

    target_link_libraries(${testname}
        #PRIVATE ThreadPool::ThreadPool
        PRIVATE ${THREADPOOL}
    )

    add_test(NAME ${testname} COMMAND $<TARGET_FILE:${testname}>)
endforeach()

# Coverage
if(ENABLE_COVERAGE)
    set(COVERAGE_MAIN "${PROJECT}_coverage")
    set(COVERAGE_LCOV_EXCLUDES
        "${PROJECT_SOURCE_DIR}/cmake/*"
        "${PROJECT_SOURCE_DIR}/tests/*"
        "${PROJECT_SOURCE_DIR}/build/*"
    )

    setup_target_for_coverage_lcov(
        NAME ${COVERAGE_MAIN}
        EXECUTABLE ctest
        DEPENDENCIES ${TEST_BASIC} )

endif()
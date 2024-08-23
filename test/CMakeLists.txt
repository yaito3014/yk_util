cmake_minimum_required(VERSION 3.24)

include(CTest)
if(BUILD_TESTING)
    add_executable(yk_util_test test.cpp)
    target_compile_features(yk_util_test PUBLIC cxx_std_23)

    set(YK_COMMON_FLAG $<$<CXX_COMPILER_ID:Clang>:-stdlib=libc++>)
    target_compile_options(yk_util_test PRIVATE ${YK_COMMON_FLAG})
    target_link_options(yk_util_test PRIVATE ${YK_COMMON_FLAG})

    find_package(TBB)
    if(${TBB_FOUND})
        target_link_libraries(yk_util_test PRIVATE TBB::tbb)
    endif()

    find_package(Boost REQUIRED CONFIG)
    target_link_libraries(yk_util_test PUBLIC yk_util Boost::headers)
    add_test(NAME yk_util_test COMMAND yk_util_test)
endif()

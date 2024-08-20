cmake_minimum_required(VERSION 3.24)

include(CTest)
if(BUILD_TESTING)
    find_package(Boost REQUIRED CONFIG)
    add_executable(test test.cpp)
    target_compile_features(test PUBLIC cxx_std_23)

    set(YK_COMMON_FLAG $<$<CXX_COMPILER_ID:Clang>:-stdlib=libc++>)
    target_compile_options(test PRIVATE ${YK_COMMON_FLAG})
    target_link_options(test PRIVATE ${YK_COMMON_FLAG})

    target_link_libraries(test PUBLIC yk_util Boost::headers)
    add_test(NAME yk_util_test COMMAND test)
endif()

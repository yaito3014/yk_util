cmake_minimum_required(VERSION 3.24)

include(CTest)
if(BUILD_TESTING)
  add_executable(yk_util_test
    compare.cpp
    enum_bitops.cpp
    hash.cpp
    meta.cpp
    util.cpp
    colorize.cpp
    fixed_string.cpp
    variant_view.cpp
    atomic_queue.cpp
    concurrency.cpp
    scheduler.cpp
    main.cpp
  )
  target_compile_features(yk_util_test PUBLIC cxx_std_23)
  set_target_properties(yk_util_test PROPERTIES CXX_EXTENSIONS OFF)

  set(YK_COMMON_FLAG
    # We temporarily disable UBSan on GCC due to the rejects-valid bug;
    # see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=71962
    $<$<CXX_COMPILER_ID:GNU>:-fsanitize=address>

    $<$<CXX_COMPILER_ID:Clang>:-fsanitize=address,undefined>
    $<$<CXX_COMPILER_ID:Clang>:-stdlib=libc++ -fexperimental-library>
  )
  target_compile_definitions(
    yk_util_test
    PRIVATE $<$<CXX_COMPILER_ID:Clang>:YK_BUILD_UNIT_TEST_FRAMEWORK=1>
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:NOMINMAX>
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:WIN32_LEAN_AND_MEAN>
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_UNICODE>
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:UNICODE>
  )
  target_compile_options(
    yk_util_test
    PRIVATE ${YK_COMMON_FLAG}
    PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic-errors>
    PRIVATE $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic-errors>
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /EHsc /Zc:__cplusplus /Zc:preprocessor /sdl /utf-8>
  )
  if(MSVC)
    target_compile_options(
      yk_util_test
      PUBLIC
        /fsanitize=address /wd5072 # ASan intentionally enabled on Release
    )
    target_link_options(
      yk_util_test
      PUBLIC
        /ignore:4302 # ASan intentionally enabled on Release
        /INCREMENTAL:NO # required for ASan
    )
  endif()
  target_link_options(yk_util_test PRIVATE ${YK_COMMON_FLAG})

  if(NOT MSVC)
    find_package(TBB REQUIRED)
    target_link_libraries(yk_util_test PRIVATE TBB::tbb)
  endif()

  find_package(Boost REQUIRED CONFIG COMPONENTS unit_test_framework exception)
  target_link_libraries(
    yk_util_test PUBLIC yk_util Boost::headers
                        $<$<NOT:$<CXX_COMPILER_ID:Clang>>:Boost::unit_test_framework> Boost::exception
  )
  add_test(NAME yk_util_test COMMAND yk_util_test)
endif()

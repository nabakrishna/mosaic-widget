# # Mosaic's test suite: unit tests for the modules that are genuinely
# # pure logic with no rendering/OS-window dependency -- LayoutEngine (spec
# # section 32 explicitly asks for this to be testable without the UI) and
# # the ActivityDateTime parser. Everything else in src/ (Direct2D
# # rendering, Win32 message handling, DPAPI, WinRT toasts) is UI/OS
# # integration that's exercised by the Phase N acceptance-test checklists
# # in READ.md instead -- not fakeable in a meaningful way without
# # reimplementing the thing under test.
# #
# # GoogleTest via FetchContent mirrors the project's existing SQLite
# # vendoring philosophy (see third_party/sqlite/README.txt): a pinned,
# # reliable source rather than expecting the developer to have vcpkg or
# # system-wide GTest already installed.
# include(FetchContent)
# FetchContent_Declare(
#     googletest
#     URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
# )
# # gtest links against the static CRT by default on Windows, which
# # mismatches Mosaic's own default (dynamic CRT) unless overridden --
# # this is the documented fix.
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(googletest)

# add_executable(MosaicTests
#     layout/LayoutEngineTests.cpp
#     widgets/ActivityDateTimeTests.cpp
#     # LayoutEngine.cpp is pure grid-unit logic (see its own comment) and
#     # ActivityDateTime.cpp only touches <ctime>, so both compile as
#     # ordinary console-app code without pulling in D2D/DComp/WinRT or
#     # linking the widget/rendering object graph above them.
#     "${CMAKE_SOURCE_DIR}/src/layout/LayoutEngine.cpp"
#     "${CMAKE_SOURCE_DIR}/src/widgets/ActivityDateTime.cpp"
# )

# target_include_directories(MosaicTests PRIVATE "${CMAKE_SOURCE_DIR}/src")
# target_link_libraries(MosaicTests PRIVATE gtest_main)

# if(MSVC)
#     target_compile_options(MosaicTests PRIVATE /W4 /permissive- /EHsc)
# endif()

# include(GoogleTest)
# gtest_discover_tests(MosaicTests)







#new
# Mosaic's test suite: unit tests for the modules that are genuinely
# pure logic with no rendering/OS-window dependency -- LayoutEngine (spec
# section 32 explicitly asks for this to be testable without the UI) and
# the ActivityDateTime parser. Everything else in src/ (Direct2D
# rendering, Win32 message handling, DPAPI, WinRT toasts) is UI/OS
# integration that's exercised by the Phase N acceptance-test checklists
# in READ.md instead -- not fakeable in a meaningful way without
# reimplementing the thing under test.
#
# GoogleTest via FetchContent mirrors the project's existing SQLite
# vendoring philosophy (see third_party/sqlite/README.txt): a pinned,
# reliable source rather than expecting the developer to have vcpkg or
# system-wide GTest already installed.
include(FetchContent)
# GIT_REPOSITORY + GIT_TAG (a shallow clone) rather than a raw GitHub
# archive URL: GitHub's codeload.github.com zip endpoint has documented,
# recurring intermittent failures on hosted CI runners (timeouts, empty
# archives) that a git clone from the same host doesn't share. GIT_SHALLOW
# keeps this about as fast as the zip download was.
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
    GIT_SHALLOW TRUE
)
# gtest links against the static CRT by default on Windows, which
# mismatches Mosaic's own default (dynamic CRT) unless overridden --
# this is the documented fix.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(MosaicTests
    layout/LayoutEngineTests.cpp
    widgets/ActivityDateTimeTests.cpp
    # LayoutEngine.cpp is pure grid-unit logic (see its own comment) and
    # ActivityDateTime.cpp only touches <ctime>, so both compile as
    # ordinary console-app code without pulling in D2D/DComp/WinRT or
    # linking the widget/rendering object graph above them.
    "${CMAKE_SOURCE_DIR}/src/layout/LayoutEngine.cpp"
    "${CMAKE_SOURCE_DIR}/src/widgets/ActivityDateTime.cpp"
)

target_include_directories(MosaicTests PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(MosaicTests PRIVATE gtest_main)

if(MSVC)
    target_compile_options(MosaicTests PRIVATE /W4 /permissive- /EHsc)
endif()

include(GoogleTest)
gtest_discover_tests(MosaicTests)
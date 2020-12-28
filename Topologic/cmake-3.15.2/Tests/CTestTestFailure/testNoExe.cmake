cmake_minimum_required(VERSION 2.4)

# Settings:
set(CTEST_DASHBOARD_ROOT                "/root/wasim-cpython-topologic/Topologic/cmake-3.15.2/Tests/CTestTest")
set(CTEST_SITE                          "name")
set(CTEST_BUILD_NAME                    "CTestTest-Linux-g++-NoExe")

set(CTEST_SOURCE_DIRECTORY              "/root/wasim-cpython-topologic/Topologic/cmake-3.15.2/Tests/CTestTestFailure")
set(CTEST_BINARY_DIRECTORY              "/root/wasim-cpython-topologic/Topologic/cmake-3.15.2/Tests/CTestTestFailure")
set(CTEST_CVS_COMMAND                   "CVSCOMMAND-NOTFOUND")
set(CTEST_CMAKE_GENERATOR               "Unix Makefiles")
set(CTEST_CMAKE_GENERATOR_PLATFORM      "")
set(CTEST_CMAKE_GENERATOR_TOOLSET       "")
set(CTEST_BUILD_CONFIGURATION           "$ENV{CMAKE_CONFIG_TYPE}")
set(CTEST_COVERAGE_COMMAND              "/usr/bin/gcov")
set(CTEST_NOTES_FILES                   "${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}")

#CTEST_EMPTY_BINARY_DIRECTORY(${CTEST_BINARY_DIRECTORY})

CTEST_START(Experimental)
CTEST_TEST(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res)
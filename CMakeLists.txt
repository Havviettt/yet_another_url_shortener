cmake_minimum_required(VERSION 3.21...3.31)

project(yet_another_url_shortener VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(DownloadUserver)

find_package(userver COMPONENTS core postgresql QUIET)
if(NOT userver_FOUND)
    download_userver(TRY_DIR third_party/userver GIT_TAG v3.0)
endif()

userver_setup_environment()

add_library(
    ${PROJECT_NAME}_objs OBJECT
    src/handlers/create_url_handler.cpp
    src/handlers/redirect_handler.cpp
    src/handlers/url_management_handler.cpp
    src/http/http_utils.cpp
    src/observability/app_metrics.cpp
    src/repositories/url_repository.cpp
    src/services/url_service.cpp
    src/url_utils.cpp
)
target_include_directories(${PROJECT_NAME}_objs PUBLIC src)
target_link_libraries(${PROJECT_NAME}_objs PUBLIC userver::core userver::postgresql)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs)

include(CTest)
if(BUILD_TESTING)
    add_executable(${PROJECT_NAME}_unittest tests/url_utils_test.cpp)
    target_link_libraries(${PROJECT_NAME}_unittest PRIVATE ${PROJECT_NAME}_objs userver::utest)
    add_google_tests(${PROJECT_NAME}_unittest)

    userver_testsuite_add_simple()
endif()

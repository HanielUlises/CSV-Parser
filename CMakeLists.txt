cmake_minimum_required(VERSION 3.16)

project(csvparser
    VERSION 0.1.0
    LANGUAGES CXX
)

add_library(csvparser INTERFACE)

target_include_directories(csvparser
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(csvparser
    INTERFACE cxx_std_17
)

target_compile_definitions(csvparser
    INTERFACE
        CSVPARSER_HEADER_ONLY=1
)

option(CSVPARSER_BUILD_TESTS "Build CSVParser tests" ON)

if (CSVPARSER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

include(GNUInstallDirs)

install(
    TARGETS csvparser
    EXPORT csvparserTargets
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT csvparserTargets
    FILE csvparserTargets.cmake
    NAMESPACE csvparser::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/csvparser
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    csvparserConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/csvparserConfig.cmake.in
    csvparserConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/csvparser
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/csvparserConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/csvparserConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/csvparser
)

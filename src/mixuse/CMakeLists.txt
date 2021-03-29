enable_language(C ASM CXX)

set(SOURCES 
    aaa.cpp
    aaa_c_connector.cpp
)

set(HEADERS
    aaa_c_connector.h 
)
set(MIXUSE_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)
add_library(aaa_c_connector SHARED ${SOURCES})
set_target_properties(aaa_c_connector PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(aaa_c_connector PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/inc>
  $<INSTALL_INTERFACE:include>
)

install(TARGETS aaa_c_connector EXPORT aaa_c_connector
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)
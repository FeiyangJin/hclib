enable_language(C ASM CXX)

set(SOURCES 
    aaa.cpp
    aaa_c_connector.cpp
)

set(HEADERS
    aaa_c_connector.h 
)

add_library(aaa_c_connector SHARED ${SOURCES})
set_target_properties(aaa_c_connector PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(aaa_c_connector PROPERTIES PUBLIC_HEADER ${HEADERS})

install(TARGETS aaa_c_connector EXPORT aaa_c_connector
        PUBLIC_HEADER DESTINATION include
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)

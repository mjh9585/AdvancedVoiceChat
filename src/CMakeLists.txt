# project(AdvancedVoice)

add_executable(AdvancedVoice main.cpp)
set_target_properties(AdvancedVoice PROPERTIES 
    CXX_STANDARD 17
    OUTPUT_NAME server
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

target_link_libraries(AdvancedVoice datachannel-static nlohmann_json opus)
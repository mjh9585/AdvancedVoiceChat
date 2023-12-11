# project(AdvancedVoice)

add_executable(AdvancedVoice main2.cpp)
set_target_properties(AdvancedVoice PROPERTIES 
    CXX_STANDARD 17
    OUTPUT_NAME server
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

target_link_libraries(AdvancedVoice datachannel-static nlohmann_json opus)

add_executable(RingBufferTest BufferTesting.cpp RingBuffer.hpp)
set_target_properties(RingBufferTest PROPERTIES 
    CXX_STANDARD 17
    OUTPUT_NAME RingBufferTest
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# project(AdvancedVoice)

set(sources 
    audioBuffer.hpp
    client.hpp
    mixingMatrix.hpp
    mixingMatrix.cpp
    RingBuffer.hpp
)

add_executable(AdvancedVoice main2.cpp ${sources})
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

add_executable(MixingMatrixTest MatrixTest.cpp ${sources})
set_target_properties(MixingMatrixTest PROPERTIES 
    CXX_STANDARD 17
    OUTPUT_NAME MixingMatrixTest
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

target_link_libraries(MixingMatrixTest datachannel-static nlohmann_json opus)

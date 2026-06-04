cmake_minimum_required(VERSION 3.20)
project(ScreenshotTranslator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)  
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON) 
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)
add_executable(screenshot-translator WIN32 ${SOURCES} resources/resources.qrc)

target_link_libraries(screenshot-translator PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

add_custom_command(TARGET screenshot-translator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/config.json"
        "$<TARGET_FILE_DIR:screenshot-translator>/config.json"
    COMMENT "Copying config.json to build output"
)

if(WIN32)
    target_sources(screenshot-translator PRIVATE resources/icon.rc)
    target_link_libraries(screenshot-translator PRIVATE Qt6::EntryPointPrivate)
endif()

install(TARGETS screenshot-translator DESTINATION bin)
install(FILES config.json DESTINATION .)

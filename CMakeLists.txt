cmake_minimum_required(VERSION 3.20)

project(GHT60_Report)

function(add_subdirectory_path path)
    file(GLOB SUBPATH "${path}/*")

    foreach(ITEM ${SUBPATH})
        if(ITEM MATCHES ".*\\.((rar)|(7z)|(zip))$")
            get_filename_component(ITEM_PATH ${ITEM} DIRECTORY)
            message(STATUS "Extrace File:${ITEM}")
            execute_process(
                COMMAND ${CMAKE_COMMAND} -E tar xzf ${ITEM}
                WORKING_DIRECTORY ${ITEM_PATH}
            )
        endif()
    endforeach(ITEM ${SUBPATH})

    file(GLOB SUBPATH "${path}/*")

    foreach(ITEM ${SUBPATH})
        if(IS_DIRECTORY "${ITEM}" AND EXISTS "${ITEM}/CMakeLists.txt")
            file(RELATIVE_PATH ITEM_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${ITEM})
            message(STATUS "Add Subdirectory: ${ITEM_PATH}")
            add_subdirectory(${ITEM_PATH})
        endif(IS_DIRECTORY "${ITEM}" AND EXISTS "${ITEM}/CMakeLists.txt")
    endforeach(ITEM ${SUBPATH})
endfunction(add_subdirectory_path path)

find_package(Qt5 5.15 REQUIRED COMPONENTS Core SerialPort Widgets)

add_subdirectory_path("components")

add_subdirectory(components/QXlsx/QXlsx)
target_link_libraries(misc_utils PUBLIC QXlsx::QXlsx)
target_link_libraries(UnionLib PUBLIC misc_utils Qt5::Core Qt5::SerialPort Qt5::Widgets)

set(CMAKE_CXX_STANDARD 23)

add_compile_options(
    "$<$<C_COMPILER_ID:MSVC>:/utf-8>"
    "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
    "$<$<C_COMPILER_ID:MSVC>:/W4>"
    "$<$<CXX_COMPILER_ID:MSVC>:/W4>"
)

aux_source_directory(src CURRENT_SRC)
add_executable(${PROJECT_NAME} ${CURRENT_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC UnionLib)

install(TARGETS ${PROJECT_NAME})
install(FILES "template/GHT-2BReportTemplate.xlsx" DESTINATION bin)
include(CPack)

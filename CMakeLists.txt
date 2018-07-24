cmake_minimum_required( VERSION 3.1 )

#NASM for gcc+nasm (tested on Windows and Linux), MASM for Visual Studio
set(ASM_LANG "MASM" CACHE STRING "Choose ASM language (NASM, MASM)")
set_property(CACHE ASM_LANG PROPERTY STRINGS NASM MASM)


#Set project name and languages
project(DEC64 C ASM_${ASM_LANG})

#Set output folders
set(CMAKE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_OUTPUT_DIRECTORY}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_OUTPUT_DIRECTORY}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_OUTPUT_DIRECTORY}/bin)
set(CMAKE_HEADER_OUTPUT_DIRECTORY  ${CMAKE_OUTPUT_DIRECTORY}/include)

#Copy header files from source to build
FILE(GLOB_RECURSE include_files ${PROJECT_SOURCE_DIR}/src/*.h) #just copying every single header into ns3 include folder
file(COPY ${include_files} DESTINATION ${CMAKE_HEADER_OUTPUT_DIRECTORY})

#Set common include folder
include_directories(${CMAKE_HEADER_OUTPUT_DIRECTORY})

#Include the appropriate assembly source
if (${ASM_LANG} STREQUAL NASM)
    set(ASM_SOURCE src/dec64n.asm)
elseif(${ASM_LANG} STREQUAL MASM)
    set(ASM_SOURCE src/dec64m.asm)
else()
    message(ERROR "Unknown ASM_LANG")
endif()


#Add library
add_library(dec64
                src/dec64.h
                src/dec64_math.h
                src/dec64_math.c
                src/dec64_string.h
                src/dec64_string.c
                ${ASM_SOURCE})

#Select proper copy command for each OS and export definition of Windows/UNIX calling convention
if (WIN32 OR MINGW)
    set(COPY_COMMAND copy .\\src\\*.h .\\build\\include)#damned windows backward slash
    if(${ASM_LANG} STREQUAL NASM)
        add_definitions(-D UNIX=0)
    else()
        add_definitions(/D UNIX=0)
    endif()
#Linux and Mac
else()
    set(COPY_COMMAND cp ./src/*.h ./build/include)
    add_definitions(-D UNIX=1)
endif()

#Terrible, but copies the headers from source to include before every build
add_custom_command(
        TARGET dec64
        PRE_BUILD
        COMMAND ${COPY_COMMAND}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
#Link static library
target_link_libraries(dec64)



#Add tests
add_executable(dec64_test ./test/dec64_test.c)
target_link_libraries(dec64_test dec64)

add_executable(dec64_math_test ./test/dec64_math_test.c)
target_link_libraries(dec64_math_test dec64)

add_executable(dec64_string_test ./test/dec64_string_test.c)
target_link_libraries(dec64_string_test dec64)


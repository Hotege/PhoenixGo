project(yao)
add_definitions(-DUNICODE)
file(
    GLOB win_srcs
    win/controls/window.cpp
    win/main.cpp
)
if(WIN32)
    if(DEBUG)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
        add_executable(yao-win ${win_srcs} win/yw.rc)
    endif(DEBUG)
    if(RELEASE)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s -Os -static")
        add_executable(yao-win WIN32 ${win_srcs} win/yw.rc)
    endif(RELEASE)
    target_link_libraries(
        yao-win
        PGCommon
        msimg32
    )
    install(TARGETS yao-win DESTINATION "bin")
endif(WIN32)
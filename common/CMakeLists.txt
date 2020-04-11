file(
    GLOB srcs
    go_comm.cc
    go_state.cc
    str_utils.cc
    thread_conductor.cc
    timer.cc
    wait_group.cc
)

add_library(PGCommon ${srcs})
install(TARGETS PGCommon DESTINATION "lib")
install(
    FILES go_comm.h go_state.h str_utils.h task_queue.h thread_conductor.h timer.h wait_group.h
    DESTINATION "include/common"
)
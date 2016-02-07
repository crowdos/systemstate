find_package(Boost REQUIRED system)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(../lib)

function(state_executable ...)
  add_executable(${ARGV0} common.cpp ${ARGN})
  target_link_libraries(${ARGV0} systemstate ${Boost_LIBRARIES} pthread)
install(TARGETS ${ARGV0} DESTINATION /usr/bin)
endfunction(state_executable)

state_executable(systemstate-read read.cpp)
state_executable(systemstate-write write.cpp)
state_executable(systemstate-ls ls.cpp)
state_executable(systemstate-watch watch.cpp)
state_executable(systemstate-ping ping.cpp)

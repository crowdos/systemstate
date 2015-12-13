function(state_plugin ...)
  include_directories(../lib)
  add_library(${ARGV0} MODULE ${ARGN})
  target_link_libraries(${ARGV0} systemstate)
endfunction(state_plugin)

state_plugin(testplugin test.cpp)

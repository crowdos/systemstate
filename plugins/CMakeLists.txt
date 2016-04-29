function(state_plugin ...)
  include_directories(../lib)
  add_library(${ARGV0} MODULE ${ARGN})
  target_link_libraries(${ARGV0} systemstate)
  install(TARGETS ${ARGV0} LIBRARY DESTINATION /usr/lib/systemstate/plugins/)
  install(FILES ${ARGV0}.ini DESTINATION /usr/lib/systemstate/plugins/)
endfunction(state_plugin)

state_plugin(testplugin test.cpp)
state_plugin(testplugin2 test2.cpp)

state_plugin(hwhal hwhal.cpp)
target_link_libraries(hwhal -lhwhal -lhwhal-loopintegration-asio)

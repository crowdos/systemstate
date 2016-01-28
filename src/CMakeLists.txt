find_package(Boost REQUIRED system)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(../lib)

add_executable(systemstated
  main.cpp
  pluginloader.cpp
  plugindb.cpp
  server.cpp
  )
target_link_libraries(systemstated systemstate dl ${Boost_LIBRARIES})
install(TARGETS systemstated DESTINATION /usr/bin)

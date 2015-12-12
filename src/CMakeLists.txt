include_directories(../lib)

add_executable(systemstated
  main.cpp
  pluginloader.cpp
  server.cpp
  )
target_link_libraries(systemstated systemstate ixp dl)
install(TARGETS systemstated DESTINATION /usr/bin)

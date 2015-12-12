include_directories(../lib)

add_executable(systemstated
  main.cpp
  server.cpp
  )
target_link_libraries(systemstated systemstate ixp)
install(TARGETS systemstated DESTINATION /usr/bin)

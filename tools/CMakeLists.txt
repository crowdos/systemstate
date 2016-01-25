find_package(Boost REQUIRED system)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(../lib)

add_executable(systemstate-read
  common.cpp
  read.cpp
  )
target_link_libraries(systemstate-read systemstate ${Boost_LIBRARIES})
install(TARGETS systemstate-read DESTINATION /usr/bin)

add_executable(systemstate-ls
  common.cpp
  ls.cpp
  )
target_link_libraries(systemstate-ls systemstate ${Boost_LIBRARIES})
install(TARGETS systemstate-ls DESTINATION /usr/bin)

add_executable(systemstate-watch
  common.cpp
  watch.cpp
  )
target_link_libraries(systemstate-watch systemstate ${Boost_LIBRARIES})
install(TARGETS systemstate-watch DESTINATION /usr/bin)

add_executable(systemstate-ping
  common.cpp
  ping.cpp
  )
target_link_libraries(systemstate-ping systemstate ${Boost_LIBRARIES})
install(TARGETS systemstate-ping DESTINATION /usr/bin)

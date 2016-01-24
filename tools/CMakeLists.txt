find_package(Boost REQUIRED system)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(../lib)

add_executable(systemstate-get
  common.cpp
  get.cpp
  )
target_link_libraries(systemstate-get systemstate ${Boost_LIBRARIES})
install(TARGETS systemstate-get DESTINATION /usr/bin)

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

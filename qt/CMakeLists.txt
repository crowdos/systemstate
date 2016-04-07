find_package(Boost REQUIRED system)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core REQUIRED)

add_library(contextsubscriber SHARED contextproperty.cpp connection.cpp)
include_directories(../lib)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(contextsubscriber Qt5::Core systemstate ${Boost_LIBRARIES})

set_target_properties(contextsubscriber PROPERTIES SOVERSION 1 VERSION 1)

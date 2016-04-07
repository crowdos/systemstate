find_package(Boost REQUIRED system)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core REQUIRED)

add_library(contextsubscriber SHARED contextproperty.cpp connection.cpp)
include_directories(../lib)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(contextsubscriber Qt5::Core systemstate ${Boost_LIBRARIES})

set_target_properties(contextsubscriber PROPERTIES SOVERSION 1 VERSION 1)

set(_PKG_CONFIG_PREFIX "${CMAKE_INSTALL_PREFIX}")
set(_PKG_CONFIG_LIBDIR "\${prefix}/lib")
set(_PKG_CONFIG_INCLUDEDIR "\${prefix}/include")
set(_PKG_CONFIG_NAME "contextsubscriber-1.0")
set(_PKG_CONFIG_DESCRIPTION "Contextkit subscriber Qt5 interface to systemstate")
set(_PKG_CONFIG_VERSION "0.1")
set(_PKG_CONFIG_LIBS "-lcontextsubscriber")
set(_PKG_CONFIG_CFLAGS "")

configure_file(
  "${PROJECT_SOURCE_DIR}/qt/contextsubscriber-1.0.pc.template"
  "${CMAKE_CURRENT_BINARY_DIR}/contextsubscriber-1.0.pc"
  )

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/contextsubscriber-1.0.pc"
  DESTINATION /usr/lib/pkgconfig
  PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE)

install(TARGETS contextsubscriber LIBRARY DESTINATION /usr/lib)
install(FILES contextproperty.h DESTINATION /usr/include)

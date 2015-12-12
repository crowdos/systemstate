add_library(systemstate SHARED
  utils.cpp
  stateplugin.cpp
  )

install(TARGETS systemstate DESTINATION /usr/lib)

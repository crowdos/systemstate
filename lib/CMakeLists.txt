add_library(systemstate SHARED
  utils.cpp
  )

install(TARGETS systemstate DESTINATION /usr/lib)

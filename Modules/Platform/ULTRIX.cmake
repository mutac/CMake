SET(CMAKE_SHARED_LIBRARY_C_FLAGS "-G 0")
SET(CMAKE_SHARED_LIBRARY_SUFFIX "..o")
SET(CMAKE_DL_LIBS "")
SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-Wl,-D,08000000")
INCLUDE(Platform/UnixPaths)

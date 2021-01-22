# Install script for directory: C:/Users/naomi/Desktop/CG_homework1/CG_homework1/External/assimp-5.0.1

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Users/naomi/Desktop/CG_homework1/CG_homework1/out/install/x64-Debug (default)")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibassimp5.0.0-devx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/assimp-5.0" TYPE FILE FILES
    "C:/Users/naomi/Desktop/CG_homework1/CG_homework1/out/build/x64-Debug (default)/External/assimp-5.0.1/assimp-config.cmake"
    "C:/Users/naomi/Desktop/CG_homework1/CG_homework1/out/build/x64-Debug (default)/External/assimp-5.0.1/assimp-config-version.cmake"
    "C:/Users/naomi/Desktop/CG_homework1/CG_homework1/out/build/x64-Debug (default)/External/assimp-5.0.1/assimpTargets.cmake"
    "C:/Users/naomi/Desktop/CG_homework1/CG_homework1/out/build/x64-Debug (default)/External/assimp-5.0.1/assimpTargets-debug.cmake"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibassimp5.0.0-devx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "C:/Users/naomi/Desktop/CG_homework1/CG_homework1/out/build/x64-Debug (default)/External/assimp-5.0.1/assimp.pc")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/Users/naomi/Desktop/CG_homework1/CG_homework1/out/build/x64-Debug (default)/External/assimp-5.0.1/contrib/zlib/cmake_install.cmake")
  include("C:/Users/naomi/Desktop/CG_homework1/CG_homework1/out/build/x64-Debug (default)/External/assimp-5.0.1/contrib/cmake_install.cmake")
  include("C:/Users/naomi/Desktop/CG_homework1/CG_homework1/out/build/x64-Debug (default)/External/assimp-5.0.1/code/cmake_install.cmake")

endif()


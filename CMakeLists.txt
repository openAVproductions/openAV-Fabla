
# Change this as needed - modern CMake's complain that they won't work before 3.5.
# This is a very primitive CMake file, and works with almost all versions from 2.x onwards.
cmake_minimum_required (VERSION 3.5)

project (fabla)

set(FABLA_VERSION_MAJOR "1")
set(FABLA_VERSION_MINOR "5")
set(FABLA_VERSION_PATCH "0")

option(BUILD_SSE "Build with SSE flags" ON)

set(FABLA_VERSION "${FABLA_VERSION_MAJOR}.${FABLA_VERSION_MINOR}.${FABLA_VERSION_PATCH}")

find_package(PkgConfig)

#SET(CMAKE_INSTALL_PREFIX "/usr" )

pkg_check_modules(LV2 lv2 REQUIRED)
include_directories( ${LV2_INCLUDE_DIRS}  )

pkg_check_modules(NTK ntk-static REQUIRED)
include_directories( ${NTK_INCLUDE_DIRS}  )
link_directories   ( ${NTK_LIBRARY_DIRS}  )

pkg_check_modules(SNDFILE sndfile REQUIRED)
include_directories( ${SNDFILE_INCLUDE_DIRS}  )
link_directories   ( ${SNDFILE_LIBRARY_DIRS}  )

SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fPIC -shared -Wl,-z,nodelete -Wl,--no-undefined")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -O3 -Wno-unused-variable -Wno-trigraphs")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O3 -Wno-unused-variable -fvisibility=hidden -ftree-vectorize -Wno-deprecated-declarations")

# print config
string( ASCII 27 _esc)

function(COLOR_MESSAGE TEXT)
    if(CMAKE_COLOR_MAKEFILE)
        MESSAGE(${TEXT})
    else()
        string(REGEX REPLACE "${_esc}.[0123456789;]*m" "" __TEXT ${TEXT} )
        MESSAGE(${__TEXT})
    endif()
endfunction()

SET(green     "${_esc}[1;32m")
SET(blue      "${_esc}[1;34m")
SET(reset     "${_esc}[1;0m")

COLOR_MESSAGE( "
  ${blue}Fabla Configuration : ${FABLA_VERSION}${reset}
-----------------------
* ${green}Install Directory${reset}               : ${CMAKE_INSTALL_PREFIX}\n" )
# /print config



FILE(GLOB sources dsp/fabla.cxx +
  gui/*.cxx +
  gui/*.c )

ADD_LIBRARY(fabla SHARED ${sources})

target_link_libraries( fabla ${NTK_LIBRARIES}     )
target_link_libraries( fabla ${SNDFILE_LIBRARIES} )

# Remove "lib" part before name (fabla.so, not libfabla.so)
set_target_properties(fabla PROPERTIES PREFIX "")

# add the install targets
install (TARGETS fabla DESTINATION lib/lv2/fabla.lv2/)

# install .ttl files
install(FILES "dsp/fabla.ttl"     DESTINATION lib/lv2/fabla.lv2/)
install(FILES "dsp/manifest.ttl"  DESTINATION lib/lv2/fabla.lv2/)

install( DIRECTORY "presets/fabla808.lv2"           DESTINATION lib/lv2 )
install( DIRECTORY "presets/fablaHardElectro.lv2"   DESTINATION lib/lv2 )
install( DIRECTORY "presets/fablaSavageDrums.lv2"   DESTINATION lib/lv2 )
install( DIRECTORY "presets/fablaEasternHop.lv2"    DESTINATION lib/lv2 )

# install presets
#install(COPY "presets/fabla808.lv2"  DESTINATION lib/lv2/ )

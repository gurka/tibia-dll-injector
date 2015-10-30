cmake_minimum_required(VERSION 3.0)

project(tibia-dll-injector)

include(GenerateExportHeader)

# Common compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

# tibia-dll-injector.exe
set(tibia-dll-injector_src
  "src/tibia-dll-injector/packet_buffer.cc"
  "src/tibia-dll-injector/rsa.cc"
  "src/tibia-dll-injector/server.cc"
  "src/tibia-dll-injector/tibia_dll_injector.cc"
  "src/tibia-dll-injector/xtea.cc"
)

set(tibia-dll-injector_inc
  "src/common"
)

set(tibia-dll-injector_lib
  "common"
  "modules"
  "Kernel32"
  "psapi"
  "ws2_32"
  "gmp"
)

add_executable(tibia-dll-injector ${tibia-dll-injector_src})
target_include_directories(tibia-dll-injector PUBLIC ${tibia-dll-injector_inc})
target_link_libraries(tibia-dll-injector ${tibia-dll-injector_lib})

# tibia-proxy-dll.dll
set(tibia-proxy-dll_src
  "src/tibia-proxy-dll/tibia_proxy_dll.cc"
)

set(tibia-proxy-dll_inc
  "src/common"
)

set(tibia-proxy-dll_lib
  "common"
  "user32"
)

add_library(tibia-proxy-dll MODULE ${tibia-proxy-dll_src})
target_include_directories(tibia-proxy-dll PUBLIC ${tibia-proxy-dll_inc})
target_link_libraries(tibia-proxy-dll ${tibia-proxy-dll_lib})

# Common
set(common_src
  "src/common/packet.cc"
)

add_library(common ${common_src})

# Modules
set(modules_src
  "src/tibia-dll-injector/modules/map.cc"
  "src/tibia-dll-injector/modules/market_data.cc"
  "src/tibia-dll-injector/modules/packet_dump.cc"
)

set(modules_inc
  "src/common"
)

add_library(modules ${modules_src})
target_include_directories(modules PUBLIC ${modules_inc})


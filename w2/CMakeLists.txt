cmake_minimum_required(VERSION 3.13)

project(w2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(W2_CLIENT_SOURCES
    client.cpp
    )

set(W2_LOBBY_SOURCES
    lobby.cpp
    )

set(W2_GAME_SOURCES
    game.cpp
    )


include_directories("../3rdParty/enet/include")

if(MSVC)
  # https://github.com/raysan5/raylib/issues/857
  add_compile_definitions(NOVIRTUALKEYCODES NOWINMESSAGES NOWINSTYLES NOSYSMETRICS NOMENUS NOICONS NOKEYSTATES NOSYSCOMMANDS NORASTEROPS NOSHOWWINDOW OEMRESOURCE NOATOM NOCLIPBOARD NOCOLOR NOCTLMGR NODRAWTEXT NOGDI NOKERNEL NOUSER NOMB NOMEMMGR NOMETAFILE NOMINMAX NOMSG NOOPENFILE NOSCROLL NOSERVICE NOSOUND NOTEXTMETRIC NOWH NOWINOFFSETS NOCOMM NOKANJI NOHELP NOPROFILER NODEFERWINDOWPOS NOMCX)
endif()

add_executable(w2_client ${W2_CLIENT_SOURCES})
target_link_libraries(w2_client PUBLIC project_options project_warnings)
target_link_libraries(w2_client PUBLIC raylib enet)

add_executable(w2_lobby ${W2_LOBBY_SOURCES})
target_link_libraries(w2_lobby PUBLIC project_options project_warnings)
target_link_libraries(w2_lobby PUBLIC enet)

add_executable(w2_game ${W2_GAME_SOURCES})
target_link_libraries(w2_game PUBLIC project_options project_warnings)
target_link_libraries(w2_game PUBLIC enet)

if(MSVC)
  target_link_libraries(w2_client PUBLIC ws2_32.lib winmm.lib)
  target_link_libraries(w2_lobby PUBLIC ws2_32.lib winmm.lib)
  target_link_libraries(w2_game PUBLIC ws2_32.lib winmm.lib)
endif()



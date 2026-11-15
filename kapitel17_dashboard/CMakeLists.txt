# Kapitel 17: ohdash – Terminal-Dashboard nach Model-View-Controller
set(CURSES_NEED_WIDE TRUE)          # ncursesw: UTF-8 (Umlaute, °C)
find_package(Curses)
add_library(ohdash_kern STATIC model.c controller.c)
target_link_libraries(ohdash_kern PUBLIC openhab::rest_client Threads::Threads)
target_include_directories(ohdash_kern PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(CURSES_FOUND)
  add_executable(ohdash main.c ansicht_text.c ansicht_ncurses.c)
  target_compile_definitions(ohdash PRIVATE MIT_NCURSES)
  target_include_directories(ohdash PRIVATE ${CURSES_INCLUDE_DIRS})
  target_link_libraries(ohdash PRIVATE ohdash_kern ${CURSES_LIBRARIES})
else()
  message(STATUS "ncurses fehlt – ohdash nur mit --text")
  add_executable(ohdash main.c ansicht_text.c)
  target_link_libraries(ohdash PRIVATE ohdash_kern)
endif()
set_target_properties(ohdash PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/kapitel17_dashboard)

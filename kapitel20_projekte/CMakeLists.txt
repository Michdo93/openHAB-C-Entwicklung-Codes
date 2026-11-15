# Kapitel 20: Projekte
add_executable(ohctl ohctl.c)
target_link_libraries(ohctl PRIVATE openhab::rest_client)

# ohplugin: eine gemeinsam genutzte Bibliothek (.so / .dll) mit c-openhab-rest-client darin
add_library(ohplugin SHARED ohplugin.c)
target_link_libraries(ohplugin PRIVATE openhab::rest_client Threads::Threads)
set_target_properties(ohplugin PROPERTIES C_VISIBILITY_PRESET hidden)   # nur OHPLUGIN_API
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # auch die Symbole der eingebundenen statischen Bibliotheken verbergen
  target_link_options(ohplugin PRIVATE -Wl,--exclude-libs,ALL)
endif()

set_target_properties(ohctl ohplugin PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/kapitel20_projekte
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/kapitel20_projekte)
install(TARGETS ohctl RUNTIME DESTINATION bin)

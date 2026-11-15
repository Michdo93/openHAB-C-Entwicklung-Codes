# Kapitel 19: Tests mit CTest – Unit-Tests ohne openHAB, Integrationstests mit openHAB
#   ctest --test-dir build -L unit            nur Unit-Tests
#   ctest --test-dir build -L integration     braucht OPENHAB_URL und OPENHAB_TOKEN
#   ctest --test-dir build -T memcheck        alles unter valgrind
add_library(testhilfe STATIC attrappe.c)
target_include_directories(testhilfe PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(testhilfe PUBLIC Threads::Threads)

function(buch_test name label)
  add_executable(${name} ${name}.c)
  target_link_libraries(${name} PRIVATE testhilfe ${ARGN})
  set_target_properties(${name} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/kapitel19_tests)
  add_test(NAME ${name} COMMAND ${name})
  set_tests_properties(${name} PROPERTIES LABELS ${label} SKIP_RETURN_CODE 77 TIMEOUT 120)
endfunction()

buch_test(test_messwerte unit ohgeraet_kern)
buch_test(test_model unit ohdash_kern)
buch_test(test_controller unit ohdash_kern)
buch_test(test_geraet integration ohgeraet_kern openhab::test_suite)
buch_test(vorabtest integration openhab::test_suite)

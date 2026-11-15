# Begleit-Repository zu »openHAB & C-Entwicklung«
#   cmake -S . -B build && cmake --build build -j
# Jede .c-Datei in kapitel08 … kapitel16 ist ein eigenes Programm; die Programme aus
# mehreren Dateien (Kapitel 17 bis 20) stehen in den CMakeLists.txt ihrer Ordner.
cmake_minimum_required(VERSION 3.16)
project(OpenHABCBuch VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra)
endif()
option(MIT_ASAN "AddressSanitizer und UndefinedBehaviorSanitizer einschalten" OFF)
if(MIT_ASAN)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

# ── Bibliotheken: c-openhab-test-suite 2.0.0 bringt c-openhab-rest-client 2.0.0 mit ──
include(FetchContent)
set(OPENHAB_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(OPENHAB_TESTSUITE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(c_openhab_test_suite
  GIT_REPOSITORY https://github.com/Michdo93/c-openhab-test-suite.git
  GIT_TAG        2.0.0)
FetchContent_MakeAvailable(c_openhab_test_suite)

find_package(Threads REQUIRED)
find_package(CURL REQUIRED)

# ── Einzeldateien: kapitel10_…/01_verbinden.c → build/kapitel10_…/01_verbinden
file(GLOB beispiele CONFIGURE_DEPENDS RELATIVE ${CMAKE_SOURCE_DIR}
     kapitel0[89]_*/*.c kapitel1[0-6]_*/*.c)
foreach(quelle ${beispiele})
  get_filename_component(ordner ${quelle} DIRECTORY)
  get_filename_component(name ${quelle} NAME_WE)
  set(ziel ${ordner}__${name})
  add_executable(${ziel} ${quelle})
  set_target_properties(${ziel} PROPERTIES OUTPUT_NAME ${name}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${ordner})
  target_link_libraries(${ziel} PRIVATE openhab::rest_client Threads::Threads
                        CURL::libcurl m)
endforeach()

# ── Programme aus mehreren Dateien und Tests ──────────────────────────────────
# include(CTest) statt enable_testing(): bringt auch "ctest -T memcheck" (valgrind) mit
set(MEMORYCHECK_COMMAND_OPTIONS
    "--leak-check=full --errors-for-leak-kinds=definite --error-exitcode=1")
include(CTest)
add_subdirectory(kapitel17_dashboard)
add_subdirectory(kapitel18_geraet)
add_subdirectory(kapitel19_tests)
add_subdirectory(kapitel20_projekte)

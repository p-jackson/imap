# Define the files in this project
file(GLOB_RECURSE includes "*.h")
file(GLOB_RECURSE sources "*.cc")

# Create the executable test project
add_executable(tests ${sources} ${includes})

include_directories(${CATCH_INCLUDE_DIR})

target_link_libraries(tests imap ${Boost_LIBRARIES})

add_dependencies(tests catch)

add_test(NAME imap-unit-tests COMMAND tests)
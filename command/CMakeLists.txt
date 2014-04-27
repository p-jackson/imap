file(GLOB_RECURSE INCS "*.h")
file(GLOB_RECURSE SRCS "*.cc")

add_executable(command ${SRCS} ${INCS})

target_link_libraries(command imaplib ${Boost_LIBRARIES})

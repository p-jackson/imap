file(GLOB_RECURSE INCS "*.h")
file(GLOB_RECURSE SRCS "*.cc")

add_library(imap ${SRCS} ${INCS})

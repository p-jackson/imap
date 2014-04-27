file(GLOB_RECURSE INCS "imap/*.h")

add_library(include ${INCS})
set_target_properties(include PROPERTIES LINKER_LANGUAGE CXX)
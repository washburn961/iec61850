# Create the library from libfile.c
add_library(iec61850 "goose.c" "ber.c")

# Optionally, specify include directories (if needed for external projects or headers)
target_include_directories(iec61850 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

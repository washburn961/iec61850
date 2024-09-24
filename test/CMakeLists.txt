# Create an executable from main.c
add_executable(main main.c)

# Link the executable with the library from the lib folder
target_link_libraries(main PRIVATE iec61850)

# Include the lib directory to find headers
target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/iec61850)

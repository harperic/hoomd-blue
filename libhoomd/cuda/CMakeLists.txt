# Maintainer: joaander

if (ENABLE_CUDA)

# find all header files in this directory
file(GLOB HDRS ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.cuh)

# install them
install(FILES ${HDRS} DESTINATION ${INC_INSTALL_DIR})

endif(ENABLE_CUDA)


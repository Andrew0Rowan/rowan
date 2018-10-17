cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

# Set project information
project(AlexaClientSDK VERSION 1.4.0 LANGUAGES CXX)
set(PROJECT_BRIEF "A cross-platform, modular SDK for interacting with the Alexa Voice Service")

if( ${CMAKE_INSTALL_LIBDIR} MATCHES "lib64" )
    set(AVS_ARCH aarch64)
else()
    set(AVS_ARCH cortexa7hf_neon_vfpv4)
endif()
message("avs arch is ${AVS_ARCH}")
set(AVS_RPM_FILE avs-1.0.0-r0.${AVS_ARCH}.rpm)
set(AVS_RPM_UNPACK_SCRIPT avs_rpm_unpack.sh)

# Copy files to build folder and unpack them
# Use two execute_process to make sure they're sequential
execute_process(
    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/${AVS_RPM_FILE} ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/${AVS_RPM_UNPACK_SCRIPT} ${CMAKE_CURRENT_BINARY_DIR}
)
execute_process(
    COMMAND sh ${AVS_RPM_UNPACK_SCRIPT}
)

# This is ../build/usr/bin
set(AVS_INSTALL_BINDIR ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
# This is ../build/usr/lib(64)
set(AVS_INSTALL_LIBDIR ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/lib/systemd/system/alexaapp.service 
    DESTINATION /lib/systemd/system
)
install(
    PROGRAMS ${AVS_INSTALL_BINDIR}/alexaapp.sh
             ${AVS_INSTALL_BINDIR}/AlexaApp
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(
    DIRECTORY ${AVS_INSTALL_BINDIR}/config
              ${AVS_INSTALL_BINDIR}/sensory
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(
    DIRECTORY ${AVS_INSTALL_LIBDIR}
    DESTINATION ${CMAKE_INSTALL_PREFIX}
    FILES_MATCHING PATTERN lib*.so
)
install(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/usr/misc/resources
    DESTINATION misc
    FILES_MATCHING PATTERN *.mp3
)

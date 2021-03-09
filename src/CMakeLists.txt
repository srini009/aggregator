# set source files
set (server-src-files
     provider.c)

set (client-src-files
     client.c)

set (admin-src-files
     admin.c)

set (dummy-src-files
     dummy/dummy-backend.c)

set (bedrock-module-src-files
     bedrock-module.c)

# load package helper for generating cmake CONFIG packages
include (CMakePackageConfigHelpers)

# where to install files for "find_package"
set (aggregator-pkg "share/cmake/aggregator")

# set version fo shared library
set (aggregator-vers "${AGGREGATOR_VERSION_MAJOR}.${AGGREGATOR_VERSION_MINOR}")

# server library
add_library (aggregator-server ${server-src-files} ${dummy-src-files})
target_link_libraries (aggregator-server
    PkgConfig::MARGO
    PkgConfig::ABTIO
    PkgConfig::UUID)
#    PkgConfig::JSONC)
target_include_directories (aggregator-server PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (aggregator-server BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (aggregator-server BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (aggregator-server
    PROPERTIES VERSION ${AGGREGATOR_VERSION}
    SOVERSION ${AGGREGATOR_VERSION_MAJOR})

# client library
add_library (aggregator-client ${client-src-files})
target_link_libraries (aggregator-client PkgConfig::MARGO PkgConfig::UUID)
target_include_directories (aggregator-client PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (aggregator-client BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (aggregator-client BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (aggregator-client
    PROPERTIES VERSION ${AGGREGATOR_VERSION}
    SOVERSION ${AGGREGATOR_VERSION_MAJOR})

# admin library
add_library (aggregator-admin ${admin-src-files})
target_link_libraries (aggregator-admin PkgConfig::MARGO PkgConfig::UUID)
target_include_directories (aggregator-admin PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (aggregator-admin BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (aggregator-admin BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (aggregator-admin
    PROPERTIES VERSION ${AGGREGATOR_VERSION}
    SOVERSION ${AGGREGATOR_VERSION_MAJOR})

if (${ENABLE_BEDROCK})
# bedrock-module library
add_library (aggregator-bedrock-module ${bedrock-module-src-files})
target_link_libraries (aggregator-bedrock-module aggregator-server aggregator-admin aggregator-client)
target_include_directories (aggregator-bedrock-module PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (aggregator-bedrock-module BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (aggregator-bedrock-module BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (aggregator-bedrock-module
    PROPERTIES VERSION ${AGGREGATOR_VERSION}
    SOVERSION ${AGGREGATOR_VERSION_MAJOR})
endif ()

# installation stuff (packaging and install commands)
write_basic_package_version_file(
    "aggregator-config-version.cmake"
    VERSION ${AGGREGATOR_VERSION}
    COMPATIBILITY AnyNewerVersion)

# generate our config file for find_package()
set(INCLUDE_INSTALL_DIR include/)
set(LIB_INSTALL_DIR lib/)
configure_package_config_file(aggregator-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/aggregator-config.cmake
  INSTALL_DESTINATION ${LIB_INSTALL_DIR}/aggregator/cmake
  PATH_VARS INCLUDE_INSTALL_DIR)

# some bits for the pkg-config file
set (DEST_DIR "${CMAKE_INSTALL_PREFIX}")
set (SERVER_PRIVATE_LIBS "-laggregator-server")
set (CLIENT_PRIVATE_LIBS "-laggregator-client")
set (ADMIN_PRIVATE_LIBS  "-laggregator-admin")
configure_file ("aggregator-server.pc.in" "aggregator-server.pc" @ONLY)
configure_file ("aggregator-client.pc.in" "aggregator-client.pc" @ONLY)
configure_file ("aggregator-admin.pc.in"  "aggregator-admin.pc"  @ONLY)

# configure config.h
configure_file ("config.h.in" "config.h" @ONLY)

# "make install" rules
install (TARGETS aggregator-server aggregator-client aggregator-admin
         EXPORT aggregator-targets
         ARCHIVE DESTINATION lib
         LIBRARY DESTINATION lib)
if (${ENABLE_BEDROCK})
    install (TARGETS aggregator-bedrock-module
             ARCHIVE DESTINATION lib
             LIBRARY DESTINATION lib)
endif ()
install (EXPORT aggregator-targets
         DESTINATION ${aggregator-pkg}
         FILE "aggregator-targets.cmake")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/aggregator-config.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/aggregator-config-version.cmake"
         DESTINATION ${aggregator-pkg} )
install (DIRECTORY ../include/aggregator
         DESTINATION include
         FILES_MATCHING PATTERN "*.h")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/aggregator-server.pc"
               "${CMAKE_CURRENT_BINARY_DIR}/aggregator-client.pc"
               "${CMAKE_CURRENT_BINARY_DIR}/aggregator-admin.pc"
         DESTINATION "lib/pkgconfig/")

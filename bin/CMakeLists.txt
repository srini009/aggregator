add_executable (aggregator-server ${CMAKE_CURRENT_SOURCE_DIR}/server.c)
target_link_libraries (aggregator-server aggregator-client aggregator-server)
install (TARGETS aggregator-server DESTINATION "bin")

#add_executable (example-admin ${CMAKE_CURRENT_SOURCE_DIR}/admin.c)
#target_link_libraries (example-admin aggregator-admin)

#add_executable (example-client ${CMAKE_CURRENT_SOURCE_DIR}/client.c)
#target_link_libraries (example-client aggregator-server aggregator-client)

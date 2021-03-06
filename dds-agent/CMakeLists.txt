# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-agent )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
#
configure_file( ${dds-agent_SOURCE_DIR}/src/version.h.in ${dds-agent_BINARY_DIR}/version.h @ONLY )

#
# Source files
#
set( SOURCE_FILES
	./src/main.cpp
	./src/CommanderChannel.cpp
	./src/UIChannel.cpp
	./src/AgentConnectionManager.cpp
	./src/UIConnectionManager.cpp
)

#
# Header files
#
set( HEADER_FILES
	./src/Options.h
	./src/CommanderChannel.h
	./src/UIChannel.h
	./src/AgentConnectionManager.h
	./src/UIConnectionManager.h
)

include_directories(
	${dds-agent_BINARY_DIR}
	${MiscCommon_LOCATION}
	${Boost_INCLUDE_DIRS}
)

#
# dds-agent executable
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-agent ${SOURCE_FILES} ${HEADER_FILES})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-agent ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
	dds-agent
	dds_protocol_lib
	dds_user_defaults_lib
	dds_intercom_lib
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_LOG_LIBRARY}
	${Boost_LOG_SETUP_LIBRARY}
	${Boost_THREAD_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
)

target_include_directories(dds-commander
	PRIVATE
		$<TARGET_PROPERTY:dds_user_defaults_lib,INTERFACE_INCLUDE_DIRECTORIES>
	PRIVATE
		$<TARGET_PROPERTY:dds_intercom_lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dds-agent DESTINATION bin)

#
# Unit tests
#
if (BUILD_TESTS)
    message (STATUS "Build dds-agent unit tests - YES")
 #   add_subdirectory ( ${dds-agent_SOURCE_DIR}/tests )
else (BUILD_TESTS)
    message (STATUS "Build dds-agent unit tests - NO")
endif (BUILD_TESTS)

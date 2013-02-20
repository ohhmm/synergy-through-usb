# synergy -- mouse and keyboard sharing utility
# Copyright (C) 2012 Synergy Si Ltd.
# Copyright (C) 2009 Nick Bolton
# 
# This package is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# found in the file COPYING that should have accompanied this file.
# 
# This package is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

if (WIN32)
	file(GLOB headers "MSWindows*.h")
	file(GLOB sources "MSWindows*.cpp")
elseif (APPLE)
	file(GLOB headers "OSX*.h")
	file(GLOB sources "OSX*.cpp" "OSX*.m")
elseif (UNIX)
	file(GLOB headers "XWindows*.h")
	file(GLOB sources "XWindows*.cpp")
endif()

if (SYNERGY_ADD_HEADERS)
	list(APPEND sources ${headers})
endif()

include_directories(
	../
)

if (APPLE)
	list(APPEND inc
		/System/Library/Frameworks
	)
endif()

include_directories(${inc})
add_library(platform STATIC ${sources})
target_link_libraries(platform client ${libs})

if (WIN32)
	add_library(synrgyhk SHARED ${inc_hook} ${src_hook})

	# copy the dlls (and supporting files) from the lib dir to 
	# the bin dir, so that synergyc and synergys can easily find them.
	# we should leave the other libraries compiling to the lib dir,
	# so that the bin dir remains tidy. the path is relative to the
	# build dir (in this case, that's: build\src\lib\platform).
	
	string(REGEX REPLACE "/" "\\\\" EXECUTABLE_OUTPUT_PATH_REVERSED ${EXECUTABLE_OUTPUT_PATH})
	string(REGEX REPLACE "/" "\\\\" LIBRARY_OUTPUT_PATH_REVERSED ${LIBRARY_OUTPUT_PATH})

	add_custom_command(
		TARGET synrgyhk 
		POST_BUILD
		COMMAND xcopy /Y /Q
			${LIBRARY_OUTPUT_PATH_REVERSED}\\${CMAKE_CFG_INTDIR}\\synrgyhk.*
			${EXECUTABLE_OUTPUT_PATH_REVERSED}\\${CMAKE_CFG_INTDIR}\\
	)

	if (GAME_DEVICE_SUPPORT)
		include_directories($ENV{DXSDK_DIR}/Include)
		add_library(synxinhk SHARED ${inc_xinhook} ${src_xinhook})

		add_custom_command(
			TARGET synxinhk
			POST_BUILD
			COMMAND xcopy /Y /Q
				${LIBRARY_OUTPUT_PATH_REVERSED}\\${CMAKE_CFG_INTDIR}\\synxinhk.*
				${EXECUTABLE_OUTPUT_PATH_REVERSED}\\${CMAKE_CFG_INTDIR}\\
			)

		# synergy xinput1_3.dll proxy
		include_directories($ENV{DXSDK_DIR}/Include)
		add_library(sxinpx13 SHARED ${inc_xinproxy13} ${src_xinproxy13})
		target_link_libraries(sxinpx13 synxinhk)
		add_custom_command(
			TARGET sxinpx13
			POST_BUILD
			COMMAND xcopy /Y /Q
				${LIBRARY_OUTPUT_PATH_REVERSED}\\${CMAKE_CFG_INTDIR}\\sxinpx13.*
				${EXECUTABLE_OUTPUT_PATH_REVERSED}\\${CMAKE_CFG_INTDIR}\\
			)
	endif()
endif()

if (UNIX)
	target_link_libraries(platform io net ipc synergylib client ${libs})
endif()

if (APPLE)
	find_library(COCOA_LIBRARY Cocoa)
	target_link_libraries(platform ${COCOA_LIBRARY})
endif()

# include lib into package
if (CONF_CPACK)
	install(TARGETS
		synrgyhk
		COMPONENT core
		DESTINATION bin)
endif(CONF_CPACK)

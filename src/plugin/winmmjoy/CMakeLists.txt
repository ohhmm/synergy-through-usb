# synergy -- mouse and keyboard sharing utility
# Copyright (C) 2012 Bolton Software Ltd.
# Copyright (C) 2012 Nick Bolton
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

set(inc
	winmmjoy.h
)

set(src
	winmmjoy.cpp
	${inc}
)

add_library(winmmjoy SHARED ${inc} ${src})


string(REGEX REPLACE "/" "\\\\" EXECUTABLE_OUTPUT_PATH_REVERSED ${EXECUTABLE_OUTPUT_PATH})
string(REGEX REPLACE "/" "\\\\" LIBRARY_OUTPUT_PATH_REVERSED ${LIBRARY_OUTPUT_PATH})

add_custom_command(
	TARGET winmmjoy 
	POST_BUILD
	COMMAND xcopy /Y /Q
		${LIBRARY_OUTPUT_PATH_REVERSED}\\${CMAKE_CFG_INTDIR}\\winmmjoy.*
		${EXECUTABLE_OUTPUT_PATH_REVERSED}\\${CMAKE_CFG_INTDIR}\\plugins\\
)

if (CONF_CPACK)
	install(TARGETS
		winmmjoy
		COMPONENT plugins
		DESTINATION bin\\plugins)
endif()
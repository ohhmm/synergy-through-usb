# synergy -- mouse and keyboard sharing utility
# Copyright (C) 2012 Bolton Software Ltd.
# Copyright (C) 2011 Nick Bolton
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

cmake_minimum_required ( VERSION 2.6 )
find_package (Qt4 REQUIRED QtCore QtGui QtNetwork)


include_directories (	.
	./src
	./res
	../lib
	${QT_INCLUDE_DIR}
	${QT_QTCORE_INCLUDE_DIR}
	${QT_QTGUI_INCLUDE_DIR}
	${QT_QTNETWORK_INCLUDE_DIR}
	${QT_QTSQL_INCLUDE_DIR}
	${CMAKE_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

# SYNERGY_SRCS variable that contains a list of all .cpp files
set (SYNERGYGUI_SRCS
	src/main.cpp
	src/MainWindow.cpp
	src/AboutDialog.cpp
	src/ServerConfig.cpp
	src/ServerConfigDialog.cpp
	src/ScreenSetupView.cpp
	src/Screen.cpp
	src/ScreenSetupModel.cpp
	src/NewScreenWidget.cpp
	src/TrashScreenWidget.cpp
	src/ScreenSettingsDialog.cpp
	src/BaseConfig.cpp
	src/HotkeyDialog.cpp
	src/ActionDialog.cpp
	src/Hotkey.cpp
	src/Action.cpp
	src/KeySequence.cpp
	src/KeySequenceWidget.cpp
	src/SettingsDialog.cpp
	src/AppConfig.cpp
	src/QSynergyApplication.cpp
	src/VersionChecker.cpp
	src/SetupWizard.cpp
	src/IpcClient.cpp
	src/IpcReader.cpp
	src/Ipc.cpp
	src/SynergyLocale.cpp
	src/QUtility.cpp
	src/ZeroconfServer.cpp
	src/ZeroconfThread.cpp
	src/ZeroconfRegister.cpp
	src/ZeroconfBrowser.cpp
	src/ZeroconfService.cpp
)

file(GLOB SYNERGYGUI_UIS "./res/*.ui")
file(GLOB SYNERGYGUI_RCS "./res/*.qrc")
file(GLOB SYNERGYGUI_MOC_HDRS "./src/*.h")
file(GLOB SYNERGYGUI_TRANS "./res/lang/*.ts")

# ADD_DEFINITIONS( -Wall )
set ( QT_USE_QTNETWORK TRUE )


qt4_add_resources ( SYNERGYGUI_RC_SRCS ${SYNERGYGUI_RCS} )
qt4_wrap_ui ( SYNERGYGUI_UI_HDRS ${SYNERGYGUI_UIS} )
qt4_wrap_cpp ( SYNERGYGUI_MOC_SRCS ${SYNERGYGUI_MOC_HDRS} )
qt4_create_translation ( SYNERGYGUI_TRANS_SRCS ${SYNERGYGUI_TRANS} )


add_executable ( synergy WIN32 ${SYNERGYGUI_SRCS}
		${SYNERGYGUI_MOC_SRCS}
		${SYNERGYGUI_UI_HDRS}
		${SYNERGYGUI_RC_SRCS}
		${SYNERGYGUI_TRANS_SRCS}
)

target_link_libraries ( synergy
	pthread
	arch
	base
	common
	net
	${QT_LIBRARIES}
	${QT_QTCORE_LIBRARIES}
	${QT_QTGUI_LIBRARIES}
	${QT_QTNETWORK_LIBRARIES}
	dns_sd
)



# Remade Solution from http://www.cmake.org/pipermail/cmake/2010-February/034971.html
# Copying required Qt libs into the build directory.

if (MSVC)   
	if (DEFINED QT_QMAKE_EXECUTABLE)
		set (QTLIBLIST QtCore QtGui QtNetwork)
		foreach (qtlib ${QTLIBLIST})
			get_filename_component (QT_DLL_PATH_tmp ${QT_QMAKE_EXECUTABLE} PATH)
                
			file (MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/Debug)
			file (MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/Release)
			file (MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/MinSizeRel)
			file (MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/RelWithDebInfo)

			set (TYPE "")
			add_custom_command(
				TARGET synergygui
				POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E
				copy_if_different ${QT_DLL_PATH_tmp}/${qtlib}${TYPE}4.dll
				${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/
				COMMENT "Copying ${qtlib}${TYPE}4.dll to
				${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/"
			)

			set (TYPE "d")
			add_custom_command(
				TARGET synergygui
				POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E
				copy_if_different ${QT_DLL_PATH_tmp}/${qtlib}${TYPE}4.dll
				${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/
				COMMENT "Copying ${qtlib}${TYPE}4.dll to
				${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/"
			)

		endforeach(qtlib)
	endif(DEFINED QT_QMAKE_EXECUTABLE)
endif()

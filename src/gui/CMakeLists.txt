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

CMAKE_MINIMUM_REQUIRED ( VERSION 2.6 )
FIND_PACKAGE (Qt4 REQUIRED QtCore QtGui QtNetwork)


INCLUDE_DIRECTORIES (	. 
		      	./src
		      	./res 
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

ADD_DEFINITIONS( -Wall )
SET ( QT_USE_QTNETWORK TRUE )

QT4_ADD_RESOURCES ( SYNERGYGUI_RC_SRCS ${SYNERGYGUI_RCS} )
QT4_WRAP_UI ( SYNERGYGUI_UI_HDRS ${SYNERGYGUI_UIS} )
QT4_WRAP_CPP ( SYNERGYGUI_MOC_SRCS ${SYNERGYGUI_MOC_HDRS} )
QT4_CREATE_TRANSLATION ( SYNERGYGUI_TRANS_SRCS ${SYNERGYGUI_TRANS} )


ADD_EXECUTABLE ( synergygui ${SYNERGYGUI_SRCS}
                ${SYNERGYGUI_MOC_SRCS}
                ${SYNERGYGUI_UI_HDRS}
                ${SYNERGYGUI_RC_SRCS}
                ${SYNERGYGUI_TRANS_SRCS}
)

TARGET_LINK_LIBRARIES ( synergygui ${QT_LIBRARIES} 
			${QT_QTCORE_LIBRARIES} 
			${QT_QTGUI_LIBRARIES} 
			${QT_QTNETWORK_LIBRARIES}
			dns_sd
)



# Solution from http://www.cmake.org/pipermail/cmake/2010-February/034971.html
# TODO: clean code

# Copy the needed Qt libraries into the Build directory. Also add installation
# and CPack code to support installer generation.
# this is a complete hack for Visual Studio to copy the Qt libraries.



if ( NOT Q_WS_MAC)
    if (DEFINED QT_QMAKE_EXECUTABLE)
        SET (QTLIBLIST QtCore QtGui QtNetwork)

        IF (MSVC)
            set(TYPE "d")
            FOREACH(qtlib ${QTLIBLIST})
              IF (WIN32)
                GET_FILENAME_COMPONENT(QT_DLL_PATH_tmp
${QT_QMAKE_EXECUTABLE} PATH)
                file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/Debug)
                file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/Release)
                file(MAKE_DIRECTORY
${EXECUTABLE_OUTPUT_PATH}/MinSizeRel)
                file(MAKE_DIRECTORY
${EXECUTABLE_OUTPUT_PATH}/RelWithDebInfo)
                INSTALL(FILES ${QT_DLL_PATH_tmp}/${qtlib}${type}d4.dll
                    DESTINATION ./
                    CONFIGURATIONS Debug
                    COMPONENT Applications)
                INSTALL(FILES ${QT_DLL_PATH_tmp}/${qtlib}4.dll
                    DESTINATION ./
                    CONFIGURATIONS Release
                    COMPONENT Applications)
                add_custom_target(ZZ_${qtlib}-Debug-Copy ALL
                            COMMAND ${CMAKE_COMMAND} -E
copy_if_different ${QT_DLL_PATH_tmp}/${qtlib}${TYPE}4.dll
                            ${EXECUTABLE_OUTPUT_PATH}/Debug/
                            COMMENT "Copying ${qtlib}${TYPE}4.dll to
${EXECUTABLE_OUTPUT_PATH}/Debug/")
                add_custom_target(ZZ_${qtlib}-Release-Copy ALL
                            COMMAND ${CMAKE_COMMAND} -E
copy_if_different ${QT_DLL_PATH_tmp}/${qtlib}4.dll
                            ${EXECUTABLE_OUTPUT_PATH}/Release/
                            COMMENT "Copying ${qtlib}4.dll to
${EXECUTABLE_OUTPUT_PATH}/Release/")
              ENDIF (WIN32)
            ENDFOREACH(qtlib)

        endif()
    endif(DEFINED QT_QMAKE_EXECUTABLE)
endif()

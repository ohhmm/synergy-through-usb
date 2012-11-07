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
FIND_PACKAGE (Qt4 REQUIRED )


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
SET ( SYNERGYGUI_SRCS
    ./src/main.cpp
    ./src/MainWindow.cpp
    ./src/AboutDialog.cpp
    ./src/ServerConfig.cpp
    ./src/ServerConfigDialog.cpp
    ./src/ScreenSetupView.cpp
    ./src/Screen.cpp
    ./src/ScreenSetupModel.cpp
    ./src/NewScreenWidget.cpp
    ./src/TrashScreenWidget.cpp
    ./src/ScreenSettingsDialog.cpp
    ./src/BaseConfig.cpp
    ./src/HotkeyDialog.cpp
    ./src/ActionDialog.cpp
    ./src/Hotkey.cpp
    ./src/Action.cpp
    ./src/KeySequence.cpp
    ./src/KeySequenceWidget.cpp
    ./src/SettingsDialog.cpp
    ./src/AppConfig.cpp
    ./src/QSynergyApplication.cpp
    ./src/VersionChecker.cpp
    ./src/SetupWizard.cpp
    ./src/IpcClient.cpp
    ./src/IpcReader.cpp
    ./src/Ipc.cpp

)

SET ( SYNERGYGUI_MOC_HDRS
    ./src/MainWindow.h
    ./src/AboutDialog.h
    ./src/ServerConfig.h
    ./src/ServerConfigDialog.h
    ./src/ScreenSetupView.h
    ./src/Screen.h
    ./src/ScreenSetupModel.h
    ./src/NewScreenWidget.h
    ./src/TrashScreenWidget.h
    ./src/ScreenSettingsDialog.h
    ./src/BaseConfig.h
    ./src/HotkeyDialog.h
    ./src/ActionDialog.h
    ./src/Hotkey.h
    ./src/Action.h
    ./src/KeySequence.h
    ./src/KeySequenceWidget.h
    ./src/SettingsDialog.h
    ./src/AppConfig.h
    ./src/QSynergyApplication.h
    ./src/VersionChecker.h
    ./src/SetupWizard.h
    ./src/IpcClient.h
    ./src/IpcReader.h
    ./src/Ipc.h
)

SET ( SYNERGYGUI_UIS
    ./res/MainWindowBase.ui
    ./res/AboutDialogBase.ui
    ./res/ServerConfigDialogBase.ui
    ./res/ScreenSettingsDialogBase.ui
    ./res/ActionDialogBase.ui
    ./res/HotkeyDialogBase.ui
    ./res/SettingsDialogBase.ui
    ./res/SetupWizardBase.ui
)

SET ( SYNERGYGUI_RCS
    ./res/Synergy.qrc
)

SET ( SYNERGYGUI_TRANS
    ./res/lang/nl_NL.ts
)

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
)

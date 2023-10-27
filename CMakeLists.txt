cmake_minimum_required(VERSION 3.16)
project(hddtest VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Widgets Xml)

qt_add_resources(RESOURCES resource.qrc)

add_executable(hddtest
	about.cpp
	about.ui
	definitions.cpp
	device.cpp
	file.cpp
	filerw.cpp
	filestructure.cpp
	hddtest.cpp
	hddtest.ui
	main.cpp
	randomgenerator.cpp
	readblock.cpp
	readcont.cpp
	readrnd.cpp
	seeker.cpp
	smallfiles.cpp
	testthread.cpp
	testwidget.cpp
	testwidget.ui
	timer.cpp
	${RESOURCES}
)

target_link_libraries(hddtest PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::Xml
)

set_property(TARGET hddtest PROPERTY CXX_STANDARD 23)

install(TARGETS hddtest
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

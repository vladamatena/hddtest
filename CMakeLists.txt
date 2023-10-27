cmake_minimum_required(VERSION 3.16)
project(hddtest VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Widgets Xml)

add_executable(hddtest
	about.cpp about.h about.ui
	definitions.cpp definitions.h
	device.cpp device.h
	file.cpp file.h
	filerw.cpp filerw.h
	filestructure.cpp filestructure.h
	hddtest.cpp hddtest.h hddtest.ui
	main.cpp
	randomgenerator.cpp randomgenerator.h
	readblock.cpp readblock.h
	readcont.cpp readcont.h
	readrnd.cpp readrnd.h
	seeker.cpp seeker.h
	smallfiles.cpp smallfiles.h
	testthread.cpp testthread.h
	testwidget.cpp testwidget.h testwidget.ui
	timer.cpp timer.h
)
target_link_libraries(hddtest PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::Xml
)

set_property(TARGET hddtest PROPERTY CXX_STANDARD 23)

# Resources:
set(resource_resource_files
    "icon/document-open.png"
    "icon/document-save.png"
    "icon/hdd.svg"
    "icon/help-about.png"
    "icon/process-stop.png"
)

qt_add_resources(hddtest "resource"
    PREFIX
        "/icon"
    FILES
        ${resource_resource_files}
)
set(resource1_resource_files
    "reference/KingMax 8G USB2.0.hddtest"
    "reference/Kingston 4GB CF.hddtest"
    "reference/Maxtor 6 Y080P0 USB.hddtest"
    "reference/MicroSD 8G.hddtest"
    "reference/TOSHIBA MK1652GSX.hddtest"
)

qt_add_resources(hddtest "resource1"
    PREFIX
        "/reference"
    FILES
        ${resource1_resource_files}
)

install(TARGETS hddtest
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

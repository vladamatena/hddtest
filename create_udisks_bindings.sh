#!/bin/sh

qdbusxml2cpp -v -c UDisksInterface -p ./udisksinterface.h:./udisksinterface.cpp org.freedesktop.UDisks.xml
qdbusxml2cpp -v -c UDisksDeviceInterface -p ./udisksdeviceinterface.h:./udisksdeviceinterface.cpp org.freedesktop.UDisks.Device.xml

#!/bin/sh

qdbusxml2cpp -v -c UDisksInterface -p ./udisksinterface.h:./udisksinterface.cpp org.freedesktop.UDisks.xml

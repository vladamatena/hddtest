/*******************************************************************************
*
*	HDDTest the graphical drive benchmarking tool.
*	Copyright (C) 2011  Vladimír Matěna <vlada.matena@gmail.com>
*
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
********************************************************************************/

#include "definitions.h"

using namespace HDDTest;

/**
 * Size to human readable format convertor
 */
QString Def::FormatSize(hddsize size) {
	QString value;
	QString unit;

	// get string representation of value
	if((size >= K) && (size < M)) {
		// return size in KB
		value = QString::number((qreal)size / K);
		unit = "KB";
	} else if((size >= M) && (size < G)) {
		// return size in MB
		value = QString::number((qreal)size / M);
		unit = "MB";
	} else if(size >= G) {
		// return size in GB
		value = QString::number((qreal)size / G);
		unit = "GB";
	} else {
		// default return size in bytes
		value = QString::number(size);
		unit = "B";
	}

	// trim to 2 digits
	int dotpos = value.lastIndexOf(".");
	if(dotpos > 0) {
		value.truncate(dotpos + 3);
	}

	return value + unit;
}

/// Speed to human readable format convertor
QString Def::FormatSpeed(hddsize size) {
	QString value;
	QString unit;

	// get string representation of value
	if((size >= K) && (size < M)) {
		// return size in KB
		value = QString::number((qreal)size / K);
		unit = "KB/s";
	} else if((size >= M) && (size < G)) {
		// return size in MB
		value = QString::number((qreal)size / M);
		unit = "MB/s";
	} else if(size >= G) {
		// return size in GB
		value = QString::number((qreal)size / G);
		unit = "GB/s";
	} else {
		// default return size in bytes
		value = QString::number(size);
		unit = "B/s";
	}

	// trim to 2 digits
	int dotpos = value.lastIndexOf(".");
	if(dotpos > 0) {
		value.truncate(dotpos + 3);
	}

	return value + unit;
}

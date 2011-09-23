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

#ifndef FILE_H
#define FILE_H

#include <iostream>
#include <stdio.h>
#include <fcntl.h>

#include <QObject>

#include "definitions.h"
#include "timer.h"
#include "device.h"

using namespace HDDTest;

/// Provides file red write benchmaring functions
/** File class handles file manipulation during file writting reading benchmark.
@see Device class **/
class File : public QObject
{
    Q_OBJECT
public:
	/** File constructor prepares file for reading and wtitting into/from it.
	  @param path to the file
	  @param device device the file resides on as pointer to Device class instance **/
	explicit File(QString path, Device *device, QObject *parent = 0);

	~File(); /// File destructor - closes open file

	void Close(); /// Closes file

	/** Set current position in file
	  @param pos the position **/
	void SetPos(hddsize pos);

	void Reopen(); /// Reopens file to clear caches

	/** Write at current position in file
	  @param size to be written
	  @return operation time **/
	hddtime Write(hddsize size);

	/** Read at current position in file
	  @param size to be read
	  @return operation time **/
	hddtime Read(hddsize size);

	Timer timer; /// Timer used for opeartion time measuring

private:
	void ReportError();	// reports error in test
	int fd;			// file`s file descriptor
	QString path;	// path to file
	void fdopen();	// open file by path stored internally

signals:
	void operationError();	/// Emited when error occures

public slots:

};

#endif // FILE_H

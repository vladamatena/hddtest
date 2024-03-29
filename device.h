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

/**
* \class Device
*
* This class provides abstraction to block device.
* It contains methods for:
* 1. querying and storing information about device
* 2. running basic benchmark operations (timed reading, writing, ...)
* Class can be used to access block device or regular file
*
* \author Vladimír Matěna vlada.matena@gmail.com
*
*/

#pragma once

#include <fcntl.h>
#include <linux/hdreg.h>
#include <sys/ioctl.h>
#include <mntent.h>
#include <unistd.h>
#include <sys/utsname.h>

#include <QtCore>
#include <QMessageBox>
#include <QtXml>

#include "definitions.h"
#include "timer.h"

using namespace HDDTest;

/// Class for device manipulation via device file
class Device: public QObject {
	Q_OBJECT
public:
	/// Stores one item in device selection combobox
	struct Item {
		/// Item type enumeration
        enum class Type	{ DEVICE, RESULT, NOTHING };

		/// Constructor for empty item
        Item():
            type(Type::NOTHING), path("") {}

		/// Constructor for type and path specified
		Item(Type type, QString path):
			type(type), path(path), label(path) {}

        /// Constructor for type, path and label specified
        Item(Type type, QString path, QString label):
            type(type), path(path), label(label) {}

        /// Construct from QStorageInfo
        Item(QStorageInfo info);

		static Item None();             /// Returns item of none type
		static Item Saved(QString path);/// Returns item of saved type

        Type type {};		/// Item type
        QString path {};	/// Item path
        QString label {};  /// Item label
        QStorageInfo info {};  /// Qt storage info
    };

	Device();									/// Device constructor
	~Device();									/// Device destructor - close device file descriptor

	// device listing
	QList<Item> GetDevices();					/// Gets list of devices

	// device access operations
    void Open(Item device, bool close);		/// Opens device specified by path
	void Close();								/// Close device file descriptor
	void DropCaches();							/// Disables some caches for device
	hddtime Sync();								/// Sync filesystem
	void Warmup();								/// Make device redy for operation
	void DriveInfo();							/// Read driveinfo from device
	void EraseDriveInfo();						/// Erase drive info to default values

	// raw disk operations
	void SetPos(hddsize pos);					/// Set actual position
	hddtime SeekTo(hddsize pos);				/// Seek to position returns operation time
	hddtime Read(hddsize size);					/// Read data at current position and return operation time
	hddtime ReadAt(hddsize size, hddsize pos);	/// Read data at position return operation time
	hddsize GetSize();							/// Get size of drive

	// fs operations
	hddtime MkDir(QString path);				/// Makes new directory in temp and returns operation time
	hddtime	MkFile(QString path, hddsize size);	/// Makes new file in temp and return operation time
	hddtime DelFile(QString path);				/// Delete file from temp and return operation time
	hddtime DelDir(QString path);				/// Delete dir from tmp path and return operation time
	hddtime ReadFile(QString path);				/// Reads file and return operation time

	Timer timer;								/// Timer for device operation measuring

	// temp directory operations
	QString GetSafeTemp();						/// Prepares and returns path to temp for FS tests
	void ClearSafeTemp();						/// Clears temp

	// drive info
    QStorageInfo info;  /// Qt storage info
	QString path;		/// Path to device
	QString model;		/// model of the device
	QString serial;		/// Serial number of the device
	QString firmware;	/// Firmware version of the device
	hddsize size;		/// Device capacity

	// fs info
	bool fs;			/// Whenever the device is mounted
	QString mountpoint;	/// Where mounted
	QString fstype;		/// Filesystem type
	QString fsversion;	/// Fileystem version
	QString fsoptions;	/// Filesystem options

	// kernel info
	QString kernel;		/// Kernel identification string

	QDomElement WriteInfo(QDomDocument &doc);	/// Store information to XML element
	void ReadInfo(QDomElement &root);			/// Read information from XML element

private:
	void ReportWarning();						/// Reports a problem with accessing device
	void ReportError();							/// Reports error in test

	// Device's file destriptor
	int fd;
	// Device's size
	hddsize device_size;
	// Whenever device access problem was reported
	bool problemReported;

	// UDisks2 DBus connection
//	QDBusInterface *udisks;

signals:
	void accessWarning();
	void operationError();
	void udisksUpdate();
};

Q_DECLARE_METATYPE(Device::Item)

/**
* \class Device
*
* This class provides abstraction to block device or file.
* It contains methods for:
* 1. querying and storing information about device
* 2. running basic benchmark operations (timed reading, writing, ...)
* Class can be used to access block device or regular file
*
* \author Vladimír Matěna vlada.matena@gmail.com
*
*/

#ifndef DEVICE_H
#define DEVICE_H

#include <fcntl.h>
#include <iostream>
#include <linux/hdreg.h>
#include <sys/ioctl.h>
#include <iostream>
#include <stdio.h>
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
class Device: public QObject
{
	Q_OBJECT
public:
	/// Stores one item in device selection combobox
	struct Item
	{
		/// Item type enumeration
		enum Type	{ HDD_ITEM_DEVICE, HDD_ITEM_OPEN, HDD_ITEM_SAVED, HDD_ITEM_NONE };

		/// Constructor for empty item
		Item():
			type(HDD_ITEM_NONE), path("") {}

		/// Constructor for type and path specified
		Item(Type type, QString path):
			type(type), path(path) {}

		static Item None();	/// Returns item of none type
		static Item Open();	/// Returns item of open type

		/// Return item of saved type and associtates path to saved results
		static Item Saved(QString path);

		Type type;		/// Item type
		QString path;	/// Results path
	};

	Device();									/// Device constructor
	~Device();									/// Device destructor - close device file descriptor

	// device listing
	QList<Item> GetDevices();					/// Gets list of devices

	// device access operations
	void Open(QString path, bool close);		/// Opens device specified by path
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
	bool temp_created;							/// Whenever tem directory was created

	// drive info
	QString path;		/// Path to device
	QString model;		/// model of the device
	QString serial;		/// Serial number of the device
	QString firmware;	/// Firmware version of the device
	hddsize size;		/// Device capacity

	// fs info
	bool fs;			/// Whenever ounted
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

	int __fd;				// device's file destriptor
	hddsize __device_size;	// device's size
	bool problemReported;	// whenever device access problem was reported

signals:
	void accessWarning();
	void operationError();
};

Q_DECLARE_METATYPE(Device::Item)

#endif // DEVICE_H

/**
* \class Device
*
* This class provides abstraction to block device or file.
* It contains methods for:
* 1. querying and storing information about device
* 2. running basic benchmark operations (timed reading, writing, ...)
* Class can be used to access block device or regular file
*
* \author Vladimír Matěna
*
* Contact vlada.matena@gmail.com
*
*/

#ifndef DEVICE_H
#define DEVICE_H

#include <fcntl.h>
#include <sys/time.h>
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

typedef qint64 hddtime; /// time interval in microseconds
typedef qint64 hddpos;	/// position on drive in bytes
typedef qint64 hddsize; /// size on drive in bytes


/// Class for device manipulation via device file
class Device: public QObject
{
	Q_OBJECT
public:
	Device();									/// Device constructor
	~Device();									/// Device destructor - close device file descriptor

	// device access operations
	void Open(QString path, bool close, bool rw = false);	/// Opens device specified by path
	void Close();								/// Close device file descriptor
	void ReportProblem();						/// Reports a problem with accessing device
	void DropCaches();							/// Disables some caches for device
	hddtime Sync();								/// Sync filesystem
	void Warmup();								/// Make device redy for operation
	void DriveInfo();							/// Read driveinfo from device
	void EraseDriveInfo();						/// Erase drive info to default values
	static QString Format(hddsize size);		/// Size to human readable format convertor

	// raw disk operations
	void SetPos(hddpos pos);					/// Set actual position
	hddtime SeekTo(hddpos pos);					/// Seek to position returns operation time
	hddtime Read(hddsize size);					/// Read data at current position and return operation time
	hddtime Write(hddsize size);				/// Write data at current position and return operation time
	hddtime ReadAt(hddsize size, hddpos pos);	/// Read data at position return operation time
	hddsize GetSize();							/// Get size of drive

	// fs operations
	hddtime MkDir(QString path);				/// Makes new directory in temp and returns operation time
	hddtime	MkFile(QString path, hddsize size);	/// Makes new file in temp and return operation time
	hddtime DelFile(QString path);				/// Delete file from temp and return operation time
	hddtime DelDir(QString path);				/// Delete dir from tmp path and return operation time
	hddtime ReadFile(QString path);				/// Reads file and return operation time

	// temp directory operations
	QString GetSafeTemp();						/// Prepares and returns path to temp for FS tests
	void ClearSafeTemp();						/// Clears temp
	bool temp_created;

	// drive info
	QString path;
	QString model;
	QString serial;
	QString firmware;
	hddsize size;

	// fs info
	bool fs;
	QString mountpoint;
	QString fstype;
	QString fsversion;
	QString fsoptions;

	// kernel info
	QString kernel;

	QDomElement WriteInfo(QDomDocument &doc);	// store information to XML element
	void ReadInfo(QDomElement &root);			// read information from XML element

	// size units
	static const hddsize G = 1024 * 1024 * 1024;
	static const hddsize M = 1024 * 1024;
	static const hddsize K = 1024;
	static const hddsize B = 1;

	// time units
	static const hddtime s	= 1000000;
	static const hddtime ms = 1000;
	static const hddtime us = 1;

private:
	int __fd;				// device's file destriptor
	hddsize __device_size;	// device's size
	bool problemReported;	// whenever device acces problem was reported

signals:
	void accessWarning();
};


#endif // DEVICE_H

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include<QtCore>

namespace HDDTest
{
	typedef qint64 hddtime; /// time interval in microseconds
	typedef qint64 hddsize; /// size on drive in bytes

	// size units
	static const hddsize G = 1024 * 1024 * 1024;
	static const hddsize M = 1024 * 1024;
	static const hddsize K = 1024;
	static const hddsize B = 1;

	// time units
	static const hddtime s	= 1000000;
	static const hddtime ms = 1000;
	static const hddtime us = 1;

	static QString Format(hddsize size)		/// Size to human readable format convertor
	{
		// return size in KB
		if((size >= K) && (size < M))
			return 	QString::number(size / K) + "KB";

		// return size in MB
		if((size >= M) && (size < G))
			return QString::number(size / M) + "MB";

		// return size in GB
		if(size >= G)
			return QString::number(size / G) + "GB";

		// default return size in bytes
		return QString::number(size) + "B";
	}
}
#endif // DEFINITIONS_H

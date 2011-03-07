#include "definitions.h"

using namespace HDDTest;

QString Def::FormatSize(hddsize size)		/// Size to human readable format convertor
{
	QString value;
	QString unit;

	// get string representation of value
	if((size >= K) && (size < M))
	{
		// return size in KB
		value = QString::number((qreal)size / K);
		unit = "KB";
	}

	else if((size >= M) && (size < G))
	{
		// return size in MB
		value = QString::number((qreal)size / M);
		unit = "MB";
	} else if(size >= G)
	{
		// return size in GB
		value = QString::number((qreal)size / G);
		unit = "GB";
	} else
	{
		// default return size in bytes
		value = QString::number(size);
		unit = "B";
	}

	// trim to 2 digits
	int dotpos = value.lastIndexOf(".");
	if(dotpos > 0)
		value.truncate(dotpos + 3);

	return value + unit;
}

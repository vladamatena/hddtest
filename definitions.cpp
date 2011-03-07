#include "definitions.h"

using namespace HDDTest;

QString Def::Format(hddsize size)		/// Size to human readable format convertor
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

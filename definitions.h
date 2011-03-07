#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include<QtCore>

namespace HDDTest {
	class Def;

	typedef qint64 hddtime; /// time interval in microseconds
	typedef qint64 hddsize; /// size on drive in bytes

	// size units
	static const hddsize B = 1;
	static const hddsize K = 1024 * B;
	static const hddsize M = 1024 * K;
	static const hddsize G = 1024 * M;

	// time units
	static const hddtime us = 1;
	static const hddtime ms = 1000 * us;
	static const hddtime s	= 1000 * ms;

	class Def
	{
	public:
		static QString FormatSize(hddsize size);	/// Size to human readable format convertor
		static QString FormatSpeed(hddsize size);	/// Speed to human readable format convertor
	};
}

#endif // DEFINITIONS_H

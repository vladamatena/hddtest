#ifndef FILE_H
#define FILE_H

#include <iostream>
#include <stdio.h>
#include <fcntl.h>

#include <QObject>

#include "definitions.h"
#include "timer.h"

using namespace HDDTest;

/// Class for file manipulation
class File : public QObject
{
    Q_OBJECT
public:
	explicit File(QString path, QObject *parent = 0);	/// Open file specified by path
	~File();						/// File destructor - close open file

	void Close();					/// Close file
	void SetPos(hddsize pos);		/// Set current position in file
	void Reopen();					/// Reopen file to clear caches

	hddtime Write(hddsize size);	/// Write at current position in file
	hddtime Read(hddsize size);		/// Read at current position in file

	Timer timer;					/// Timer used for opeartion time measuring

private:
	int fd;			// file`s file descriptor
	QString path;	// path to file
	void fdopen();	// open file by path stored internally

signals:

public slots:

};

#endif // FILE_H

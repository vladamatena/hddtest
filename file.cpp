/**
* \author Vladimír Matěna vlada.matena@gmail.com
*/

#include "file.h"

File::File(QString path, Device *device, QObject *parent) :
	QObject(parent), path(path)
{
	// connect operation error signal to matchong signal in backlaying device
	connect(this, SIGNAL(operationError()), device, SIGNAL(operationError()));

	fdopen();	// open file
}

File::~File()
{
	Close();
}

void File::Close()
{
	close(fd);
}

void File::fdopen()
{
	// open file
	fd = open(path.toUtf8(), O_CREAT | O_RDWR | O_SYNC | O_LARGEFILE, S_IRWXU);
	if(fd < 0)
	{
		std::cout << "Error opening file" << std::endl;
		ReportError();
	}

	// advice system not to cache file data
	if(posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED))
	{
		std::cout << "Error setting advice" << std::endl;
		ReportError();
	}
}

void File::Reopen()
{
	close(fd);
	fdopen();
}

void File::SetPos(hddsize pos)
{
	// set position
	if(lseek64(fd, pos, SEEK_SET) < 0)
		ReportError();
}

hddtime File::Read(hddsize size)
{
	char *buffer = new char[size];

	timer.MarkStart();

	// read data
	if(read(fd, buffer, sizeof(char) * size) <= 0)
	{
		std::cerr << "Read failed" << std::endl;
		ReportError();
	}

	timer.MarkEnd();

	delete [] buffer;

	return timer.GetFinalOffset();
}

hddtime File::Write(hddsize size)
{
	char *buffer = new char[size];

	timer.MarkStart();

	// read data
	if(write(fd, buffer, sizeof(char) * size) <= 0)
	{
		std::cerr << "Write failed" << std::endl;
		ReportError();
	}

	timer.MarkEnd();

	delete [] buffer;

	return timer.GetFinalOffset();
}

void File::ReportError()
{
	operationError();
}

#include "file.h"

File::File(QString path, QObject *parent) :
	QObject(parent), path(path)
{
	fdopen();
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
	fd = open(path.toUtf8(), O_CREAT | O_RDWR | O_SYNC | O_LARGEFILE, S_IRWXU);
	if(fd < 0)
		std::cout << "Error opening file" << std::endl;
	int ret = posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
	if(ret)
		std::cout << "Error setting advice" << std::endl;
}

void File::Reopen()
{
	close(fd);
	fdopen();
}

void File::SetPos(hddsize pos)
{
	// set position
	lseek64(fd, pos, SEEK_SET);
}

hddtime File::Read(hddsize size)
{
	timeval __start;
	timeval __end;
	char *buffer = new char[size];

	// get seek start timestamp
	gettimeofday(&__start, 0);

	// read data
	int ret = read(fd, buffer, sizeof(char) * size);
	if(ret <= 0)
		std::cerr << "Read failed" << std::endl;

	// get seek end timestamp
	gettimeofday(&__end, 0);

	// count seek duration
	hddtime timediff = (__end.tv_sec * 1000 * 1000 + __end.tv_usec) - (__start.tv_sec * 1000 * 1000 + __start.tv_usec);

	delete buffer;

	return timediff;
}

hddtime File::Write(hddsize size)
{
	timeval __start;
	timeval __end;
	char *buffer = new char[size];

	// get seek start timeout
	gettimeofday(&__start, 0);

	// read data
	int ret = write(fd, buffer, sizeof(char) * size);
	if(ret <= 0)
		std::cerr << "Write failed" << std::endl;

	// get seek end timeout
	gettimeofday(&__end, 0);

	// count seek duration
	hddtime timediff = (__end.tv_sec * 1000 * 1000 + __end.tv_usec) - (__start.tv_sec * 1000 * 1000 + __start.tv_usec);

	delete buffer;

	return timediff;
}

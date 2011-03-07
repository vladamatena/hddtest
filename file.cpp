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
	char *buffer = new char[size];

	timer.MarkStart();

	// read data
	int ret = read(fd, buffer, sizeof(char) * size);
	if(ret <= 0)
		std::cerr << "Read failed" << std::endl;

	timer.MarkEnd();

	delete buffer;

	return timer.GetFinalOffset();
}

hddtime File::Write(hddsize size)
{
	char *buffer = new char[size];

	timer.MarkStart();

	// read data
	int ret = write(fd, buffer, sizeof(char) * size);
	if(ret <= 0)
		std::cerr << "Write failed" << std::endl;

	timer.MarkEnd();

	delete buffer;

	return timer.GetFinalOffset();
}

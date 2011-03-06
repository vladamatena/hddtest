/**
*
* \author Vladimír Matěna
*
* Contact vlada.matena@gmail.com
*
*/

#include "device.h"

Device::Device()
{
	path = "";
	temp_created = false;
	problemReported = false;
	size = -1;
	fs = false;
	DriveInfo();	// TODO: is this useless
}

void Device::Open(QString path, bool close, bool rw)
{
	this->path = path;
	temp_created = false;
	problemReported = false;

	if(close)
		Close();

	// skip opening device pointing to result file
	if(path.length() == 0)
		return;

	// open device file
	if(!rw)
		__fd = open(path.toUtf8(), O_RDONLY | O_LARGEFILE);
	else
		__fd = open(path.toUtf8(), O_CREAT | O_RDWR | O_SYNC | O_LARGEFILE, S_IRWXU);

	if(__fd < 0)
		ReportProblem();

	DropCaches();

	// get drive size
	__device_size = lseek64(__fd, 0, SEEK_END);

	// set pos to begin of device
	SetPos(0);

	DriveInfo();
}

Device::~Device()
{
	Close();
}

void Device::Close()
{
	// close device file
	close(__fd);
}

void Device::DropCaches()
{
		// give advice to disable caching
		int ret = posix_fadvise(__fd, 0, 0, POSIX_FADV_DONTNEED);
		if(ret)
			ReportProblem();

		// empty caches
		QFile caches("/proc/sys/vm/drop_caches");
		caches.open(QIODevice::WriteOnly);
		if(caches.isOpen())
		{
			caches.putChar('3');
			caches.close();
		}
		else
			ReportProblem();
}

hddtime Device::Sync()
{
	timeval __start;
	timeval __end;

	// get sync start timestamp
	gettimeofday(&__start, 0);

	// sync
	sync();

	// get sync end timestamp
	gettimeofday(&__end, 0);

	// count sync time
	hddtime timediff = (__end.tv_sec * 1000 * 1000 + __end.tv_usec) - (__start.tv_sec * 1000 * 1000 + __start.tv_usec);

	return timediff;
}

void Device::Warmup()
{
	ReadAt(M, M);
}

void Device::ReportProblem()
{
	if(problemReported)
		return;

	problemReported = true;
	accessWarning();
}

void Device::SetPos(hddsize pos)
{
	// set position
	lseek64(__fd, pos, SEEK_SET);
}

hddtime Device::SeekTo(hddsize pos)
{
	timeval __start;
	timeval __end;
	char c;

	// get seek start timestamp
	gettimeofday(&__start, 0);

	// seek to new position
	SetPos(pos);
	read(__fd, &c, sizeof(char));

	// get seek end timestamp
	gettimeofday(&__end, 0);

	// count seek duration
	hddtime timediff = (__end.tv_sec * 1000 * 1000 + __end.tv_usec) - (__start.tv_sec * 1000 * 1000 + __start.tv_usec);

	return timediff;
}

hddsize Device::GetSize()
{
	// return private __device_size
	return __device_size;
}

hddtime Device::ReadAt(hddsize size, hddsize pos)
{
	timeval __start;
	timeval __end;
	char *buffer = new char[size];

	// get seek start timestamp
	gettimeofday(&__start, 0);

	// seek to new position
	SetPos(pos);
	int ret = read(__fd, buffer, sizeof(char) * size);
	if(ret <= 0)
		std::cerr << "Read failed" << std::endl;

	// get seek end timestamp
	gettimeofday(&__end, 0);

	// count seek duration
	hddtime timediff = (__end.tv_sec * 1000 * 1000 + __end.tv_usec) - (__start.tv_sec * 1000 * 1000 + __start.tv_usec);

	delete buffer;

	return timediff;
}

hddtime Device::Read(hddsize size)
{
	timeval __start;
	timeval __end;
	char *buffer = new char[size];

	// get seek start timestamp
	gettimeofday(&__start, 0);

	// read data
	int ret = read(__fd, buffer, sizeof(char) * size);
	if(ret <= 0)
		std::cerr << "Read failed" << std::endl;

	// get seek end timestamp
	gettimeofday(&__end, 0);

	// count seek duration
	hddtime timediff = (__end.tv_sec * 1000 * 1000 + __end.tv_usec) - (__start.tv_sec * 1000 * 1000 + __start.tv_usec);

	delete buffer;

	return timediff;
}

QString Device::Format(hddsize size)
{
	// return size in KB
	if((size >= 1024) && (size < 1024 * 1024))
		return 	QString::number(size / 1024) + " KB";

	// return size in MB
	if(size >= 1024 * 1024)
		return QString::number(size / 1024 / 1024) + "MB";

	// default return size in bytes
	return QString::number(size) + " B";
}

void Device::EraseDriveInfo()
{
	size = -1;

	model = "UNKNOWN";
	serial = "UNKNOWN";
	firmware = "UNKNOWN";

	mountpoint = "NOT MOUNTED";
	fstype = "NOT MOUNTED";
	fsoptions = "NOT MOUNTED";
	fs = false;

	// kernel is left unchanged
}

void Device::DriveInfo()
{
	EraseDriveInfo();

	hd_driveid id;

	// use device size
	size = this->__device_size;

	// get drive info from ioctl
	if (ioctl(__fd, HDIO_GET_IDENTITY, &id) == 0)
	{
		model = QString::fromAscii((char*)id.model, 40).trimmed();
		serial = QString::fromAscii((char*)id.serial_no, 20).trimmed();
		firmware = QString::fromAscii((char*)id.fw_rev, 8).trimmed();
	}

	QFile mounts("/proc/mounts");
	mounts.open(QFile::ReadOnly);

	while(true)	// read all mounts lines
	{
		QString line = mounts.readLine();	// get line
		if(line.length() == 0)	// no more lines -> end reading
			break;

		QStringList list = line.split(' ');		// split line to fields

		// if this line describes selected device
		if((list.size() == 6) &&
				(
						(QFile::symLinkTarget(list.at(0)).compare(this->path) == 0)
						||
						(path.compare(list.at(0)) == 0)
				)
			)
		{
			// assign information to apropriate fileds
			fs = true;	// TODO: check fs rw option
			mountpoint = (QString)(list.at(1)).trimmed();
			fstype = (QString)(list.at(2)).trimmed();
			fsoptions = (QString)(list.at(3)).trimmed().replace(",","\n");
		}
	}

	// get info about kernel
	utsname buf;

	if(!uname(&buf))
	{
		kernel = QString::fromAscii(buf.sysname) + " - " + QString::fromAscii(buf.release);
	}

	mounts.close();
}

QString Device::GetSafeTemp()
{
	if(!fs)		// no FS = no temp
		return "";

	// check / create tmp dir
	if(!QDir(mountpoint).exists("tmp"))
	{
		temp_created = true;
		QDir(mountpoint).mkdir("tmp");
	}

	// create final temp dir
	QDir(mountpoint + "/tmp").mkdir("hddtest.temp.dir");

	return mountpoint + "/tmp/hddtest.temp.dir";
}

hddtime Device::Write(hddsize size)
{
	// TODO: check for writting to block device
	std::cout << "Not writting to device - this is not implemented" << std::endl;
	std::cerr << "Not writting to device - this is not implemented" << std::endl;
	return 0;

	timeval __start;
	timeval __end;
	char *buffer = new char[size];

	// get seek start timeout
	gettimeofday(&__start, 0);

	// read data
	int ret = write(__fd, buffer, sizeof(char) * size);
	if(ret <= 0)
		std::cerr << "Write failed" << std::endl;

	// get seek end timeout
	gettimeofday(&__end, 0);

	// count seek duration
	hddtime timediff = (__end.tv_sec * 1000 * 1000 + __end.tv_usec) - (__start.tv_sec * 1000 * 1000 + __start.tv_usec);

	delete buffer;

	return timediff;
}

hddtime Device::MkDir(QString path)
{
	QString temp = GetSafeTemp();
	QDir dir(temp);

	timeval __start;
	timeval __end;

	// get start time
	gettimeofday(&__start, 0);

	// make dir
	dir.mkdir(path);

	// get end time
	gettimeofday(&__end, 0);

	// count seek duration
	hddtime timediff = (__end.tv_sec * 1000 * 1000 + __end.tv_usec) - (__start.tv_sec * 1000 * 1000 + __start.tv_usec);

	return timediff;
}

hddtime Device::MkFile(QString path, hddsize size)
{
	QFile file(path);

	timeval __start;
	timeval __end;

	// get start time
	gettimeofday(&__start, 0);

	// write file
	file.open(QIODevice::WriteOnly);
	if(size > 0)
	{
		char *data = new char[size];
		file.write(data);
		delete data;
	}
	file.close();

	// get end time
	gettimeofday(&__end, 0);

	// count seek duration
	hddtime timediff = (__end.tv_sec * 1000 * 1000 + __end.tv_usec) - (__start.tv_sec * 1000 * 1000 + __start.tv_usec);

	return timediff;
}

hddtime Device::DelDir(QString path)
{
	QString temp = GetSafeTemp();
	QDir dir(temp);

	timeval __start;
	timeval __end;

	// get start time
	gettimeofday(&__start, 0);

	// del dir
	dir.rmdir(path);

	// get end time
	gettimeofday(&__end, 0);

	// count seek duration
	hddtime timediff = (__end.tv_sec * 1000 * 1000 + __end.tv_usec) - (__start.tv_sec * 1000 * 1000 + __start.tv_usec);

	return timediff;
}

hddtime Device::DelFile(QString path)
{
	timeval __start;
	timeval __end;

	// get start time
	gettimeofday(&__start, 0);

	// del file
	QFile::remove(path);

	// get end time
	gettimeofday(&__end, 0);

	// count seek duration
	hddtime timediff = (__end.tv_sec * 1000 * 1000 + __end.tv_usec) - (__start.tv_sec * 1000 * 1000 + __start.tv_usec);

	return timediff;
}

hddtime Device::ReadFile(QString path)
{
	timeval __start;
	timeval __end;

	// get start time
	gettimeofday(&__start, 0);

	// read file
	QFile file(path);
	file.open(QIODevice::ReadOnly);
	char *buffer = new char[file.size()];
	file.read(buffer, file.size());
	delete buffer;
	file.close();

	// get end time
	gettimeofday(&__end, 0);

	// count seek duration
	hddtime timediff = (__end.tv_sec * 1000 * 1000 + __end.tv_usec) - (__start.tv_sec * 1000 * 1000 + __start.tv_usec);

	return timediff;
}

QDomElement Device::WriteInfo(QDomDocument &doc)
{
	// create main info element
	QDomElement master = doc.createElement("Info");
	doc.appendChild(master);

	// add fs info
	QDomElement fsi = doc.createElement("FS");
	fsi.setAttribute("fs", fs);
	fsi.setAttribute("mountpoint", mountpoint);
	fsi.setAttribute("fstype", fstype);
	fsi.setAttribute("fsversion", fsversion);
	fsi.setAttribute("fsoptions", fsoptions);
	master.appendChild(fsi);

	// add device info
	QDomElement dev = doc.createElement("Device");
	dev.setAttribute("path", path);
	dev.setAttribute("model", model);
	dev.setAttribute("serial", serial);
	dev.setAttribute("firmware", firmware);
	dev.setAttribute("size", size);
	master.appendChild(dev);

	// add kernel info
	QDomElement ker = doc.createElement("Kernel");
	ker.setAttribute("kernel", kernel);
	master.appendChild(ker);	

	return master;
}

void Device::ReadInfo(QDomElement &root)
{
	// Locate main seek element
	QDomElement info = root.firstChildElement("Info");

	// add fs info
	QDomElement fsi = info.firstChildElement("FS");
	if(fsi.isNull())
		return;
	fs = fsi.attribute("fs", "NO DATA").compare("1") == 0;
	mountpoint = fsi.attribute("mountpoint", "NO DATA");
	fstype = fsi.attribute("fstype", "NO DATA");
	fsversion = fsi.attribute("fsversion", "NO DATA");
	fsoptions = fsi.attribute("fsoptions", "NO DATA");

	// add device info
	QDomElement dev = info.firstChildElement("Device");
	if(dev.isNull())
		return;
	path = dev.attribute("path", "NO DATA");
	model = dev.attribute("model", "NO DATA");
	serial = dev.attribute("serial", "NO DATA");
	firmware = dev.attribute("firmware", "NO DATA");
	size = dev.attribute("size", "NO DATA").toLongLong();

	// add kernel info
	QDomElement ker = info.firstChildElement("Kernel");
	if(ker.isNull())
		return;
	kernel = ker.attribute("kernel", "NO DATA");
}

File::File(QString path):
	path(path)
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


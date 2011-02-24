/**
*
* \author Vladimír Matěna
*
* Contact vlada.matena@gmail.com
*
*/

#include "device.h"

Device::Device(QString path, bool rw)
{
	Open(path, false, rw);
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

	DisableCaches();

	// get drive size
	__device_size = lseek64(__fd, 0, SEEK_END);

	// set pos to begin of device
	SetPos(0);

	this->DriveInfo();
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

void Device::DisableCaches()
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

void Device::ReportProblem()
{
	if(problemReported)
		return;

	problemReported = true;
	QString user = QString::fromAscii(getenv("USER"));
	QMessageBox box;
	box.setText("You are running HDDTest as user: " + user +
				" most probably you do not have rights for HDDTest to operate properly." +
				" You you should have right to do following with device you want to be tested:" +
				"\n\tRead block device." +
				"\n\tRead and write device`s filesystem." +
				"\n\tAdvice device not to be cached." +
				"\n\tDrop system caches." +
				"\nFailing to do so will cause HDDTest to show incorrect results.");
	box.setInformativeText("Continue at your own risk.");
	box.exec();
}

void Device::SetPos(hddpos pos)
{
	// set position
	lseek64(__fd, pos, SEEK_SET);
}

hddtime Device::SeekTo(hddpos pos)
{
	timeval __start;
	timeval __end;
	char c;

	// get seek start timeout
	gettimeofday(&__start, 0);

	// seek to new position
	SetPos(pos);
	read(__fd, &c, sizeof(char));

	// get seek end timeout
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

hddtime Device::ReadAt(hddsize size, hddpos pos)
{
	timeval __start;
	timeval __end;
	char *buffer = new char[size];

	// get seek start timeout
	gettimeofday(&__start, 0);

	// seek to new position
	SetPos(pos);
	int ret = read(__fd, buffer, sizeof(char) * size);
	if(ret <= 0)
		std::cerr << "Read failed" << std::endl;

	// get seek end timeout
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

	// get seek start timeout
	gettimeofday(&__start, 0);

	// read data
	int ret = read(__fd, buffer, sizeof(char) * size);
	if(ret <= 0)
		std::cerr << "Read failed" << std::endl;

	// get seek end timeout
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

void Device::DriveInfo()
{
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
	else
	{
		model = "UNKNOWN";
		serial = "UNKNOWN";
		firmware = "UNKNOWN";
	}

	// get info from mounts
	mountpoint = "NOT MOUNTED";
	fstype = "NOT MOUNTED";
	fsoptions = "NOT MOUNTED";
	fs = false;

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



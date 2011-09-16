/**
* \author Vladimír Matěna vlada.matena@gmail.com
*/

#include "device.h"

Device::Device()
{
	path = "";
	temp_created = false;
	problemReported = false;
	size = -1;
	fs = false;
	fd = 0;

	udisks = new UDisksInterface("org.freedesktop.UDisks", "/org/freedesktop/UDisks", QDBusConnection::systemBus(), 0);
}

Device::~Device()
{
	delete udisks;
	Close();
}

void Device::Close()
{
	// close device file
	if(fd > 0)
		close(fd);
}

QList<Device::Item> Device::GetDevices()
{
	QList<Item> list;

	QList<QDBusObjectPath> devices = udisks->EnumerateDevices().value();
	for(int i = 0; i < devices.size(); ++i)
	{
		UDisksDeviceInterface device("org.freedesktop.UDisks", devices.at(i).path(), QDBusConnection::systemBus(), 0);

		if(!device.driveIsMediaEjectable())
			list.append(Device::Item(
						Device::Item::DEVICE,
						device.deviceFile(),
						device.deviceFile() + " (" + device.driveModel() + ")"));
	}

	return list;
}

void Device::Open(QString path, bool close)
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
	fd = open(path.toUtf8(), O_RDONLY | O_LARGEFILE | O_SYNC);
	if(fd < 0)
		ReportWarning();

	DropCaches();

	// get drive size
	device_size = lseek64(fd, 0, SEEK_END);

	// set pos to begin of device
	SetPos(0);

	DriveInfo();
}

void Device::DropCaches()
{
		// give advice to disable caching
		int ret = posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
		if(ret)
			ReportWarning();

		// empty caches
		QFile caches("/proc/sys/vm/drop_caches");
		caches.open(QIODevice::WriteOnly);
		if(caches.isOpen())
		{
			caches.putChar('3');
			caches.close();
		}
		else
			ReportWarning();
}

hddtime Device::Sync()
{
	timer.MarkStart();

	// sync
	sync();

	timer.MarkEnd();

	return timer.GetFinalOffset();
}

void Device::Warmup()
{
	ReadAt(M, M);
}

void Device::ReportWarning()
{
	if(problemReported)
		return;

	problemReported = true;
	accessWarning();
}

void Device::ReportError()
{
	operationError();
}

void Device::SetPos(hddsize pos)
{
	// set position
	if(lseek64(fd, pos, SEEK_SET) < 0)
		ReportError();
}

hddtime Device::SeekTo(hddsize pos)
{
	char c;

	timer.MarkStart();

	// seek to new position
	SetPos(pos);
		if(read(fd, &c, sizeof(char)) < 0)
		ReportError();

	timer.MarkEnd();

	return timer.GetFinalOffset();
}

hddsize Device::GetSize()
{
	// return private __device_size
	return device_size;
}

hddtime Device::ReadAt(hddsize size, hddsize pos)
{
	char *buffer = new char[size];

	timer.MarkStart();

	// seek to new position
	SetPos(pos);
	if(read(fd, buffer, sizeof(char) * size) <= 0)
	{
		std::cerr << "Read failed" << std::endl;
		ReportError();
	}

	timer.MarkEnd();

	delete [] buffer;

	return timer.GetFinalOffset();
}

hddtime Device::Read(hddsize size)
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

	QDBusObjectPath object = udisks->FindDeviceByDeviceFile(path).value();
	UDisksDeviceInterface device("org.freedesktop.UDisks", object.path(), QDBusConnection::systemBus(), 0);

	size = device.deviceSize();
	model = device.driveModel();
	serial = device.driveSerial();
	firmware = device.driveRevision();

	fs = device.deviceIsMounted();

	if(fs)
	{
		// List mountpoints
		QStringList mountpoints = device.deviceMountPaths();
		mountpoint = "";
		for(int i = 0; i < mountpoints.size(); ++i)
		{
			if(!mountpoint.isEmpty())
				mountpoint += ", ";
			mountpoint += mountpoints.at(i);
		}

		// get partition type
		fstype = device.idType();

		// TODO: Reading with udisks not possible?
	//	fsoptions = device.partitionFlags().first();
	}

	// Old way reading only for filesystem mount options
	QFile mounts("/proc/mounts");
	if(mounts.open(QFile::ReadOnly | QIODevice::Text))
	{
		while(true)	// read all mounts lines
		{
			QString line = mounts.readLine();	// get line

			// check line end - this is not normal file atEnd() won`t help
			if(line.length() == 0)
				break;

			QStringList list = line.split(' ');		// split line to fields

			// if this line describes selected device
			if((list.size() == 6) && ((!QFile::symLinkTarget(list[0]).compare(path)) || (!path.compare(list[0]))))
			{
				fsoptions = (QString)(list[3]).trimmed().replace(",","\n");
				break;
			}
		}
		mounts.close();
	}

	// get info about kernel
	utsname buf;
	memset(&buf, 0, sizeof(utsname));

	if(!uname(&buf))
		kernel = QString::fromAscii(buf.sysname) + " - " + QString::fromAscii(buf.release);
}

QString Device::GetSafeTemp()
{
	if(!fs)		// no FS => no temp
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

hddtime Device::MkDir(QString path)
{
	QString temp = GetSafeTemp();
	QDir dir(temp);

	timer.MarkStart();

	// make dir
	if(!dir.mkdir(path))
		ReportError();

	timer.MarkEnd();

	return timer.GetFinalOffset();
}

hddtime Device::MkFile(QString path, hddsize size)
{
	QFile file(path);

	timer.MarkStart();

	// write file
	if(!file.open(QIODevice::WriteOnly))
		ReportError();
	else
	{
		// if size should be more then zero write data to file
		if(size > 0)
		{
			char *data = new char[size];
			if(file.write(data, size) != size)
				ReportError();
			delete data;
		}

		file.close();
	}

	timer.MarkEnd();

	return timer.GetFinalOffset();
}

hddtime Device::DelDir(QString path)
{
	QString temp = GetSafeTemp();
	QDir dir(temp);

	timer.MarkStart();

	// del dir
	if(!dir.rmdir(path))
		ReportError();

	timer.MarkEnd();

	return timer.GetFinalOffset();
}

hddtime Device::DelFile(QString path)
{
	timer.MarkStart();

	// del file
	if(!QFile::remove(path))
		ReportError();

	timer.MarkEnd();

	return timer.GetFinalOffset();
}

hddtime Device::ReadFile(QString path)
{
	timer.MarkStart();

	// read file
	QFile file(path);
	if(!file.open(QIODevice::ReadOnly))
		ReportError();
	else
	{
		// read data from file
		char *buffer = new char[file.size()];
		if(file.read(buffer, file.size()) < 0)
			ReportError();
		delete buffer;

		file.close();
	}

	timer.MarkEnd();

	return timer.GetFinalOffset();
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

Device::Item Device::Item::None()
{
	return Item(NOTHING, "");
}

Device::Item Device::Item::Open()
{
	return Item(OPEN_DIALOG, "");
}

Device::Item Device::Item::Saved(QString path)
{
	return Item(RESULT, path);
}

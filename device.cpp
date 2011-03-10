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
	__fd = 0;
}

QList<Device::Item> Device::GetDevices()
{
	QList<Item> list;
	QDir block = QDir("/dev/disk/by-path");
	QFileInfoList devList = block.entryInfoList(QDir::Files | QDir::Readable);
	for(int i = 0; i < devList.size(); ++i)
	{
		QString path = QFile::symLinkTarget(devList[i].absoluteFilePath());
		Device::Item data = Device::Item(Device::Item::HDD_ITEM_DEVICE, path);
		list.append(data);
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
	__fd = open(path.toUtf8(), O_RDONLY | O_LARGEFILE);
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
	if(__fd > 0)
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
	char c;

	timer.MarkStart();

	// seek to new position
	SetPos(pos);
	read(__fd, &c, sizeof(char));

	timer.MarkEnd();

	return timer.GetFinalOffset();
}

hddsize Device::GetSize()
{
	// return private __device_size
	return __device_size;
}

hddtime Device::ReadAt(hddsize size, hddsize pos)
{
	char *buffer = new char[size];

	timer.MarkStart();

	// seek to new position
	SetPos(pos);
	int ret = read(__fd, buffer, sizeof(char) * size);
	if(ret <= 0)
		std::cerr << "Read failed" << std::endl;

	timer.MarkEnd();

	delete [] buffer;

	return timer.GetFinalOffset();
}

hddtime Device::Read(hddsize size)
{
	char *buffer = new char[size];

	timer.MarkStart();

	// read data
	int ret = read(__fd, buffer, sizeof(char) * size);
	if(ret <= 0)
		std::cerr << "Read failed" << std::endl;

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

	hd_driveid id;
	memset(&id, 0, sizeof(hd_driveid));

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
				// assign information to apropriate fileds
				fs = true;	// TODO: check fs rw option
				mountpoint = (QString)(list[1]).trimmed();
				fstype = (QString)(list[2]).trimmed();
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

hddtime Device::MkDir(QString path)
{
	QString temp = GetSafeTemp();
	QDir dir(temp);

	timer.MarkStart();

	// make dir
	dir.mkdir(path);

	timer.MarkEnd();

	return timer.GetFinalOffset();
}

hddtime Device::MkFile(QString path, hddsize size)
{
	QFile file(path);

	timer.MarkStart();

	// write file
	file.open(QIODevice::WriteOnly);
	if(size > 0)
	{
		char *data = new char[size];
		file.write(data);
		delete data;
	}
	file.close();

	timer.MarkEnd();

	return timer.GetFinalOffset();
}

hddtime Device::DelDir(QString path)
{
	QString temp = GetSafeTemp();
	QDir dir(temp);

	timer.MarkStart();

	// del dir
	dir.rmdir(path);

	timer.MarkEnd();

	return timer.GetFinalOffset();
}

hddtime Device::DelFile(QString path)
{
	timer.MarkStart();

	// del file
	QFile::remove(path);

	timer.MarkEnd();

	return timer.GetFinalOffset();
}

hddtime Device::ReadFile(QString path)
{
	timer.MarkStart();

	// read file
	QFile file(path);
	file.open(QIODevice::ReadOnly);
	char *buffer = new char[file.size()];
	file.read(buffer, file.size());
	delete buffer;
	file.close();

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
	return Item::Item(HDD_ITEM_NONE, "");
}

Device::Item Device::Item::Open()
{
	return Item::Item(HDD_ITEM_OPEN, "");
}

Device::Item Device::Item::Saved(QString path)
{
	return Item::Item(HDD_ITEM_SAVED, path);
}


/**
*
* \author Vladimír Matěna
*
* Contact vlada.matena@gmail.com
*
*/

#include <QMessageBox>
#include <iostream>

#include "hddtest.h"
#include "ui_hddtest.h"

#include "testwidget.h"
#include "randomgenerator.h"
#include "seeker.h"


HDDTest::HDDTest(QWidget *parent) :
    QDialog(parent),
	ui(new Ui::HDDTest)
{
    ui->setupUi(this);

	connect(&device, SIGNAL(accessWarning()), this, SLOT(device_accessWarning()));
	connect(&refDevice, SIGNAL(accessWarning()), this, SLOT(refDevice_accessWarning()));

	// drive selection fill-in
	QDir block = QDir("/dev/disk/by-path");
	QFileInfoList devList = block.entryInfoList();
	for(int i = 0; i < devList.size(); ++i)
		if(devList.at(i).baseName().length() > 0)
			ui->drive->addItem(
						QFile::symLinkTarget(devList.at(i).absoluteFilePath()),
						QVariant::fromValue(DeviceItemData(DeviceItemData::HDD_ITEM_DEVICE, QFile::symLinkTarget(devList.at(i).absoluteFilePath()))));
	ui->drive->insertSeparator(ui->drive->count());
	ui->reference->addItem("Nothing", QVariant(DeviceItemData::HDD_ITEM_NONE));
	// saved results
	QDir saved = QDir();
	saved.setFilter(QDir::Files);
	saved.setNameFilters(QStringList("*.xml"));
	QFileInfoList savedList = saved.entryInfoList();
	for(int i = 0; i < savedList.size(); ++i)
		if(savedList.at(i).baseName().length() > 0)
		{
			ui->drive->addItem(
						"Saved: " + savedList.at(i).fileName(),
						QVariant::fromValue(DeviceItemData(DeviceItemData::HDD_ITEM_SAVED, savedList.at(i).absoluteFilePath())));
			ui->reference->addItem(
						"Saved: " + savedList.at(i).fileName(),
						QVariant::fromValue(DeviceItemData(DeviceItemData::HDD_ITEM_SAVED, savedList.at(i).absoluteFilePath())));
		}
	ui->drive->insertSeparator(ui->drive->count());
	ui->reference->insertSeparator(ui->reference->count());
	ui->drive->addItem("--- Launch file open dialog ---", QVariant::fromValue(DeviceItemData(DeviceItemData::HDD_ITEM_OPEN, "")));
	ui->reference->addItem("--- Launch file open dialog ---", QVariant::fromValue(DeviceItemData(DeviceItemData::HDD_ITEM_OPEN, "")));

	// initialize tests
	ui->filerwwidget->SetDevice(&device);
	ui->filestructurewidget->SetDevice(&device);
	ui->readblockwidget->SetDevice(&device);
	ui->readcontwidget->SetDevice(&device);
	ui->readrndwidget->SetDevice(&device);
	ui->seekwidget->SetDevice(&device);
	ui->smallfileswidget->SetDevice(&device);
}

HDDTest::~HDDTest()
{
    delete ui;
}

void HDDTest::on_drive_currentIndexChanged(QString)
{	
	QVariant data = ui->drive->itemData(ui->drive->currentIndex());

	switch(data.value<DeviceItemData>().type)
	{
		case DeviceItemData::HDD_ITEM_OPEN:
		{
			QString filename = QFileDialog::getOpenFileName(this, tr("Open Saved results"), "", tr("Results (*.xml)"));
			if(filename.length() > 0)
			{
				int index = ui->drive->count();
				ui->drive->insertItem(
							index,
							"Saved: " + filename,
							QVariant::fromValue(DeviceItemData(DeviceItemData::HDD_ITEM_SAVED, filename)));
				ui->drive->setCurrentIndex(index);
			}
		}
		break;
		case DeviceItemData::HDD_ITEM_DEVICE:
		{
			device.Open(data.value<DeviceItemData>().path, true);
			ReloadTests(false);
		}
		break;
		// TODO: handle dynamic added devices without DeviceItemData
		case DeviceItemData::HDD_ITEM_SAVED:
		{
			device.Open("", true);
			OpenResultFile(data.value<DeviceItemData>().path);
			ReloadTests(true);
		}
		break;
		default:
			;// TODO: handle error
	}
}

void HDDTest::on_reference_currentIndexChanged(QString )
{
	QVariant data = ui->reference->itemData(ui->reference->currentIndex());

	switch(data.value<DeviceItemData>().type)
	{
		case DeviceItemData::HDD_ITEM_OPEN:
		{
			QString filename = QFileDialog::getOpenFileName(this, tr("Open Saved results"), "", tr("Results (*.xml)"));
			if(filename.length() > 0)
			{
				int index = ui->reference->count();
				ui->reference->insertItem(
							index,
							"Saved: " + filename,
							QVariant::fromValue(DeviceItemData(DeviceItemData::HDD_ITEM_SAVED, filename)));
				ui->reference->setCurrentIndex(index);
			}
		}
		break;
		case DeviceItemData::HDD_ITEM_SAVED:
		{
			refDevice.Open("", true);

			OpenResultFile(data.value<DeviceItemData>().path, true);
			UpdateInfo(true);
		}
		break;
		default:
			;// TODO: handle error
	}
}

void HDDTest::device_accessWarning()
{
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

void HDDTest::refDevice_accessWarning() {}

void HDDTest::UpdateInfo(bool reference)
{
	if(!reference)
	{
		// update info tab - tested device
		this->ui->model->setText(device.model);
		this->ui->serial->setText(device.serial);
		this->ui->firmware->setText(device.firmware);
		this->ui->size->setText(Device::Format(device.size));
		this->ui->mountpoint->setText(device.mountpoint);
		this->ui->fstype->setText(device.fstype);
		this->ui->fsoptions->setText(device.fsoptions);
		this->ui->kernel->setText(device.kernel);
	}
	else
	{
		// update info tab - reference devices
		this->ui->reference_model->setText(refDevice.model);
		this->ui->reference_serial->setText(refDevice.serial);
		this->ui->reference_firmware->setText(refDevice.firmware);
		this->ui->reference_size->setText(Device::Format(refDevice.size));
		this->ui->reference_mountpoint->setText(refDevice.mountpoint);
		this->ui->reference_fstype->setText(refDevice.fstype);
		this->ui->reference_fsoptions->setText(refDevice.fsoptions);
		this->ui->reference_kernel->setText(refDevice.kernel);
	}
}

void HDDTest::ReloadTests(bool loaded)
{
	// check test modes - FS, Valid
	bool fs = device.fs;
	bool valid = device.size > 0;

	UpdateInfo(false);

	// Raw device test
	ui->readblockwidget->SetStartEnabled(!loaded && valid);
	ui->readcontwidget->SetStartEnabled(!loaded && valid);
	ui->readrndwidget->SetStartEnabled(!loaded && valid);
	ui->seekwidget->SetStartEnabled(!loaded && valid);

	// Filesystem tests
	ui->smallfileswidget->SetStartEnabled(!loaded && fs);
	ui->filerwwidget->SetStartEnabled(!loaded && fs);
	ui->filestructurewidget->SetStartEnabled(!loaded && fs);
}

void HDDTest::on_save_clicked()
{
	// Create base document for tests results
	QDomDocument doc("HddTest");

	QDomElement results = doc.createElement("Results");
	doc.appendChild(results);

	// save drive info
	results.appendChild(device.WriteInfo(doc));

	// save tests results
	results.appendChild(filerw.WriteResults(doc));
	results.appendChild(filestruct.WriteResults(doc));
	results.appendChild(readblock.WriteResults(doc));
	results.appendChild(readcont.WriteResults(doc));
	results.appendChild(ui->readrndwidget->WriteResults(doc));
	results.appendChild(ui->seekwidget->WriteResults(doc));
	results.appendChild(smallfiles.WriteResults(doc));

	// write document to file
	QString filename = QFileDialog::getSaveFileName(this, tr("Save results"), "", tr("Results (*.xml)"));
	if(!filename.contains("xml"))
		filename = filename + ".xml";

	if(filename.length() == 0)
		return;

	QFile file(filename);
	file.open(QIODevice::WriteOnly);
	QTextStream stream(&file);
	stream << doc.toString();
	file.close();
}

void HDDTest::OpenResultFile(QString filename, bool reference)
{
	if(filename.length() == 0)
		return;

	// open file
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly))
	{
		std::cerr << "can't open file" << std::endl;
		file.close();
		return;
	}

	// Open document
	QDomDocument doc("HddTest");
	QString err;
	int row,col;
	if(!doc.setContent(&file, &err, &row, &col))
	{
		std::cerr << "Can't' set document content:" << err.toAscii().constData() << " at row: " << row << " col:" << col << std::endl;
		file.close();
		return;
	}
	file.close();
	QDomElement root = doc.documentElement();
	QDomElement results = root.firstChildElement("Results");

	// restore device info
	reference?refDevice.ReadInfo(root):device.ReadInfo(root);
	// restore tests result
	ui->seekwidget->RestoreResults(root, reference);
	ui->filerwwidget->RestoreResults(root, reference);
	ui->filestructurewidget->RestoreResults(root, reference);
	ui->smallfileswidget->RestoreResults(root, reference);
	ui->readblockwidget->RestoreResults(root, reference);
	ui->readrndwidget->RestoreResults(root, reference);
	ui->readcontwidget->RestoreResults(root, reference);
}

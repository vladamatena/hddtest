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
	ui(new Ui::HDDTest),
	device(NULL), refDevice(NULL)
{
    ui->setupUi(this);

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
	ui->filerwwidget->SetDevice(device);
	ui->filestructurewidget->SetDevice(device);
	ui->readblockwidget->SetDevice(device);
	ui->readcontwidget->SetDevice(device);
	ui->readrndwidget->SetDevice(device);
	ui->seekwidget->SetDevice(device);
	ui->smallfileswidget->SetDevice(device);
}

HDDTest::~HDDTest()
{
	delete device;
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
			if(!device)
				device = new Device(data.value<DeviceItemData>().path, false);
			else
				device->Open(data.value<DeviceItemData>().path, true);
			ReloadTests(false);
		}
		break;
		// TODO: handle dynamic added devices without DeviceItemData
		case DeviceItemData::HDD_ITEM_SAVED:
		{
			if(!device)
				device = new Device(data.value<DeviceItemData>().path, false);
			else
				device->Open(data.value<DeviceItemData>().path, true);
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
			if(!refDevice)
				refDevice = new Device(data.value<DeviceItemData>().path, false);
			else
				refDevice->Open(data.value<DeviceItemData>().path, true);
			OpenResultFile(data.value<DeviceItemData>().path, true);
			UpdateInfo(true);
		}
		break;
		default:
			;// TODO: handle error
	}
}

void HDDTest::UpdateInfo(bool reference)
{
	if(!reference)
	{
		// update info tab - tested device
		this->ui->model->setText(device->model);
		this->ui->serial->setText(device->serial);
		this->ui->firmware->setText(device->firmware);
		this->ui->size->setText(Device::Format(device->size));
		this->ui->mountpoint->setText(device->mountpoint);
		this->ui->fstype->setText(device->fstype);
		this->ui->fsoptions->setText(device->fsoptions);
		this->ui->kernel->setText(device->kernel);
	}
	else
	{
		// update info tab - reference devices
		this->ui->reference_model->setText(refDevice->model);
		this->ui->reference_serial->setText(refDevice->serial);
		this->ui->reference_firmware->setText(refDevice->firmware);
		this->ui->reference_size->setText(Device::Format(refDevice->size));
		this->ui->reference_mountpoint->setText(refDevice->mountpoint);
		this->ui->reference_fstype->setText(refDevice->fstype);
		this->ui->reference_fsoptions->setText(refDevice->fsoptions);
		this->ui->reference_kernel->setText(refDevice->kernel);
	}
}

void HDDTest::ReloadTests(bool loaded)
{
	// check test modes - FS, Valid
	bool fs = (device)?device->fs:false;
	bool valid = (device)?(device->size > 0):false;

	UpdateInfo(false);

/****** not used since tab locking not fit secondary results loading
	// enable if device is valid or data are loaded
	for(int i = 0; i <= 4; ++i)
		this->ui->Tabs->setTabEnabled(i, valid || loaded);

	// enable fs tests if device is mounted or results are loaded
	for(int i = 5; i <= 7; ++i)
		this->ui->Tabs->setTabEnabled(i, (valid && fs) || loaded);
*/

	// disable test start in loaded mode | enable otherwise
	bool start_enabled = !loaded;
	ui->readblockwidget->SetStartEnabled(start_enabled);
	ui->readcontwidget->SetStartEnabled(start_enabled);
	ui->readrndwidget->SetStartEnabled(start_enabled);
	ui->seekwidget->SetStartEnabled(start_enabled);

	// disable fs tests on nonmounted filesystems
	start_enabled = (!loaded && fs);
	ui->smallfileswidget->SetStartEnabled(start_enabled);
	ui->filerwwidget->SetStartEnabled(start_enabled);
	ui->filestructurewidget->SetStartEnabled(start_enabled);
}

void HDDTest::on_save_clicked()
{
	// Create base document for tests results
	QDomDocument doc("HddTest");

	QDomElement results = doc.createElement("Results");
	doc.appendChild(results);

	// save drive info
	results.appendChild(device->WriteInfo(doc));

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

	// restore device info and store index
	reference?refDevice->ReadInfo(root):device->ReadInfo(root);
	int index = ui->Tabs->currentIndex();

	// restore seek results
	ui->Tabs->setCurrentIndex(4);
	ui->seekwidget->RestoreResults(root, reference);

	// restore File RW results
	ui->Tabs->setCurrentIndex(5);
	ui->filerwwidget->RestoreResults(root, reference);

	// restore File Structure
	ui->Tabs->setCurrentIndex(6);
	ui->filestructurewidget->RestoreResults(root, reference);

	// restore small files
	ui->Tabs->setCurrentIndex(7);
	ui->smallfileswidget->RestoreResults(root, reference);

	// restore read block
	ui->Tabs->setCurrentIndex(3);
	ui->readblockwidget->RestoreResults(root, reference);

	// restore read random
	ui->Tabs->setCurrentIndex(1);
	ui->readrndwidget->RestoreResults(root, reference);

	// restore read cont
	ui->Tabs->setCurrentIndex(2);
	ui->readcontwidget->RestoreResults(root, reference);

	// restore tabs to saved index
	ui->Tabs->setCurrentIndex(index);
}

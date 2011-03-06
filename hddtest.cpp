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
	QDialog(parent), ui(new Ui::HDDTest)
{
    ui->setupUi(this);

	connect(&device, SIGNAL(accessWarning()), this, SLOT(device_accessWarning()));
	connect(&refDevice, SIGNAL(accessWarning()), this, SLOT(refDevice_accessWarning()));

	// Add drive selection to results combo box
	QDir block = QDir("/dev/disk/by-path");
	QFileInfoList devList = block.entryInfoList(QDir::Files | QDir::Readable);
	for(int i = 0; i < devList.size(); ++i)
	{
		QString path = QFile::symLinkTarget(devList[i].absoluteFilePath());
		DeviceItem data = DeviceItem(DeviceItem::HDD_ITEM_DEVICE, path);
		ui->drive->addItem(path, QVariant::fromValue(data));
	}

	// Add nothing opetion to reference combo box
	ui->drive->insertSeparator(ui->drive->count());
	ui->reference->addItem(
				"Nothing",
				QVariant::fromValue(DeviceItem(DeviceItem::HDD_ITEM_NONE, "")));

	// Add saved resutls for both combo boxes
	QFileInfoList savedList = QDir().entryInfoList(QStringList("*.xml"), QDir::Files);
	for(int i = 0; i < savedList.size(); ++i)
	{
		QString label = "Saved: " + savedList[i].fileName();
		QString path = savedList[i].absoluteFilePath();
		DeviceItem item = DeviceItem(DeviceItem::HDD_ITEM_SAVED, path);

		ui->drive->addItem(label, QVariant::fromValue(item));
		ui->reference->addItem(label, QVariant::fromValue(item));
	}

	// Add file open dialog for both combo boxes
	ui->drive->insertSeparator(ui->drive->count());
	ui->reference->insertSeparator(ui->reference->count());
	DeviceItem item = DeviceItem(DeviceItem::HDD_ITEM_OPEN, "");
	ui->drive->addItem("--- Launch file open dialog ---", QVariant::fromValue(item));
	ui->reference->addItem("--- Launch file open dialog ---", QVariant::fromValue(item));

	// Initialize tests
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

	switch(data.value<DeviceItem>().type)
	{
		case DeviceItem::HDD_ITEM_OPEN:
		{
			QString filename = QFileDialog::getOpenFileName(this, tr("Open Saved results"), "", tr("Results (*.xml)"));
			if(filename.length() > 0)
			{
				int index = ui->drive->count();
				ui->drive->insertItem(
							index,
							"Saved: " + filename,
							QVariant::fromValue(DeviceItem(DeviceItem::HDD_ITEM_SAVED, filename)));
				ui->drive->setCurrentIndex(index);
			}
		}
		break;
		case DeviceItem::HDD_ITEM_DEVICE:
		{
			EraseResults(TestWidget::RESULTS);
			device.Open(data.value<DeviceItem>().path, true);
			ReloadTests(false);
		}
		break;
		// TODO: handle dynamic added devices without DeviceItem
		case DeviceItem::HDD_ITEM_SAVED:
		{
			device.Open("", true);
			EraseResults(TestWidget::RESULTS);
			OpenResultFile(data.value<DeviceItem>().path, TestWidget::RESULTS);
			ReloadTests(true);
		}
		break;
		case DeviceItem::HDD_ITEM_NONE:
			EraseResults(TestWidget::RESULTS);
			device.Open(ui->drive->currentText(), true);
			ReloadTests(false);
		break;
	}
}

void HDDTest::on_reference_currentIndexChanged(QString )
{
	QVariant data = ui->reference->itemData(ui->reference->currentIndex());

	switch(data.value<DeviceItem>().type)
	{
		case DeviceItem::HDD_ITEM_OPEN:
		{
			QString filename = QFileDialog::getOpenFileName(this, tr("Open Saved results"), "", tr("Results (*.xml)"));
			if(filename.length() > 0)
			{
				int index = ui->reference->count();
				ui->reference->insertItem(
							index,
							"Saved: " + filename,
							QVariant::fromValue(DeviceItem(DeviceItem::HDD_ITEM_SAVED, filename)));
				ui->reference->setCurrentIndex(index);
			}
		}
		break;
		case DeviceItem::HDD_ITEM_SAVED:
		{
			refDevice.Open("", true);
			EraseResults(TestWidget::REFERENCE);
			OpenResultFile(data.value<DeviceItem>().path, TestWidget::REFERENCE);
			UpdateInfo(TestWidget::REFERENCE);
		}
		break;
		case DeviceItem::HDD_ITEM_NONE:
			EraseResults(TestWidget::REFERENCE);
			UpdateInfo(TestWidget::REFERENCE);
		break;
		case  DeviceItem::HDD_ITEM_DEVICE:
			// silently ignore
			// this should not happend
		break;
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

void HDDTest::UpdateInfo(TestWidget::DataSet dataset)
{
	if(dataset == TestWidget::RESULTS)
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

	UpdateInfo(TestWidget::RESULTS);

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

void HDDTest::EraseResults(TestWidget::DataSet dataset)
{
	if(dataset == TestWidget::RESULTS)
		device.EraseDriveInfo();
	else
		refDevice.EraseDriveInfo();

	ui->readblockwidget->EraseResults(dataset);
	ui->readcontwidget->EraseResults(dataset);
	ui->readrndwidget->EraseResults(dataset);
	ui->seekwidget->EraseResults(dataset);
	ui->smallfileswidget->EraseResults(dataset);
	ui->filerwwidget->EraseResults(dataset);
	ui->filestructurewidget->EraseResults(dataset);
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
	results.appendChild(ui->filerwwidget->WriteResults(doc));
	results.appendChild(ui->filestructurewidget->WriteResults(doc));
	results.appendChild(ui->readblockwidget->WriteResults(doc));
	results.appendChild(ui->readcontwidget->WriteResults(doc));
	results.appendChild(ui->readrndwidget->WriteResults(doc));
	results.appendChild(ui->seekwidget->WriteResults(doc));
	results.appendChild(ui->smallfileswidget->WriteResults(doc));

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

void HDDTest::OpenResultFile(QString filename, TestWidget::DataSet dataset)
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
		file.close();
		return;
	}
	file.close();
	QDomElement root = doc.documentElement();
	QDomElement results = root.firstChildElement("Results");

	// restore device info
	(dataset == TestWidget::REFERENCE)?refDevice.ReadInfo(root):device.ReadInfo(root);
	// restore tests result
	ui->seekwidget->RestoreResults(root, dataset);
	ui->filerwwidget->RestoreResults(root, dataset);
	ui->filestructurewidget->RestoreResults(root, dataset);
	ui->smallfileswidget->RestoreResults(root, dataset);
	ui->readblockwidget->RestoreResults(root, dataset);
	ui->readrndwidget->RestoreResults(root, dataset);
	ui->readcontwidget->RestoreResults(root, dataset);
}

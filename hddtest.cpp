/*******************************************************************************
*
*	HDDTest the graphical drive benchmarking tool.
*	Copyright (C) 2011  Vladimír Matěna <vlada.matena@gmail.com>
*
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
********************************************************************************/

#include <QMessageBox>
#include <iostream>

#include "hddtest.h"
#include "ui_hddtest.h"

#include "testwidget.h"
#include "randomgenerator.h"
#include "seeker.h"


HDDTestWidget::HDDTestWidget(QWidget *parent) :
	QDialog(parent), ui(new Ui::HDDTestWidget)
{
    ui->setupUi(this);

	// connect access warning and error signals
	connect(&device, SIGNAL(accessWarning()), this, SLOT(device_accessWarning()));
	connect(&refDevice, SIGNAL(accessWarning()), this, SLOT(refDevice_accessWarning()));
	connect(&device, SIGNAL(operationError()), this, SLOT(device_operationError()));
	connect(&refDevice, SIGNAL(operationError()), this, SLOT(device_operationError()));

	// connect device refresh signal
	connect(&device, SIGNAL(udisksUpdate()), this, SLOT(device_list_refresh()), Qt::QueuedConnection);

	device_list_refresh();

	// Initialize tests
	ui->filerwwidget->SetDevice(&device);
	ui->filestructurewidget->SetDevice(&device);
	ui->readblockwidget->SetDevice(&device);
	ui->readcontwidget->SetDevice(&device);
	ui->readrndwidget->SetDevice(&device);
	ui->seekwidget->SetDevice(&device);
	ui->smallfileswidget->SetDevice(&device);

	// set icons to buttons
	ui->save->setIcon(QIcon::fromTheme("document-save"));
	ui->about->setIcon(QIcon::fromTheme("help-about"));
}

HDDTestWidget::~HDDTestWidget()
{
    delete ui;
}

void HDDTestWidget::on_drive_currentIndexChanged(QString)
{	
	QVariant data = ui->drive->itemData(ui->drive->currentIndex());

	switch(data.value<Device::Item>().type)
	{
	case Device::Item::OPEN_DIALOG:
		{
			QString filename = QFileDialog::getOpenFileName(this, tr("Open Saved results"), "", tr("Results (*.hddtest)"));
			if(filename.length() > 0)
			{
				int index = ui->drive->count();
				ui->drive->insertItem(
							index,
							"Saved: " + filename,
							QVariant::fromValue(Device::Item::Saved(filename)));
				ui->drive->setCurrentIndex(index);
			}
		}
		break;

	case Device::Item::DEVICE:
		EraseResults(TestWidget::RESULTS);
		device.Open(data.value<Device::Item>().path, true);
		ReloadTests(false);
		break;

	case Device::Item::RESULT:
		device.Open("", true);
		EraseResults(TestWidget::RESULTS);
		OpenResultFile(data.value<Device::Item>().path, TestWidget::RESULTS);
		ReloadTests(true);
		break;

	default:
		// this should not happend
		std::cerr << "WARNING: Bad item selected in device combo box." << std::endl;
		break;
	}
}

void HDDTestWidget::on_reference_currentIndexChanged(QString )
{
	QVariant data = ui->reference->itemData(ui->reference->currentIndex());

	switch(data.value<Device::Item>().type)
	{
	case Device::Item::OPEN_DIALOG:
		{
			QString filename = QFileDialog::getOpenFileName(this, tr("Open Saved results"), "", tr("Results (*.hddtest)"));
			if(filename.length() > 0)
			{
				int index = ui->reference->count();
				ui->reference->insertItem(
							index,
							"Saved: " + filename,
							QVariant::fromValue(Device::Item::Saved(filename)));
				ui->reference->setCurrentIndex(index);
			}
		}
		break;

	case Device::Item::RESULT:
		refDevice.Open("", true);
		EraseResults(TestWidget::REFERENCE);
		OpenResultFile(data.value<Device::Item>().path, TestWidget::REFERENCE);
		UpdateInfo(TestWidget::REFERENCE);
		break;

	case Device::Item::NOTHING:
		EraseResults(TestWidget::REFERENCE);
		UpdateInfo(TestWidget::REFERENCE);
		break;

	default:
		// this should not happend
		std::cerr << "WARNING: Device selected in reference combo box." << std::endl;
		break;
	}
}

void HDDTestWidget::device_accessWarning()
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

void HDDTestWidget::device_operationError()
{
	bool running = false;
	if(ui->filerwwidget->testState == TestWidget::STARTED)
	{
		ui->filerwwidget->StopTest();
		running = true;
	}
	if(ui->filestructurewidget->testState == TestWidget::STARTED)
	{
		ui->filestructurewidget->StopTest();
		running = true;
	}
	if(ui->readblockwidget->testState == TestWidget::STARTED)
	{
		ui->readblockwidget->StopTest();
		running = true;
	}
	if(ui->readcontwidget->testState == TestWidget::STARTED)
	{
		ui->readcontwidget->StopTest();
		running = true;
	}
	if(ui->readrndwidget->testState == TestWidget::STARTED)
	{
		ui->readrndwidget->StopTest();
		running = true;
	}
	if(ui->seekwidget->testState == TestWidget::STARTED)
	{
		ui->seekwidget->StopTest();
		running = true;
	}
	if(ui->smallfileswidget->testState == TestWidget::STARTED)
	{
		ui->smallfileswidget->StopTest();
		running = true;
	}

	if(running)
	{
		QMessageBox box;
		box.setText(QString() + "An error occured while running test. I/O operation failed in test function." +
					" This can be caused by missing permissions for operating device you are testing" +
					" as well as hardware failture. Please check following:" +
					"\n\tDo you have sufficent rights to read block device." +
					"\n\tDo you have sufficent rights to read/write devices filesystem." +
					"\n\tIs the device big enouh for selected test type." +
					"\n\tIf you are running filesystem test check free space on target filesystem." +
					"\n\tCheck you system logs for I/O errors indicating hardware failture.");
		box.setInformativeText("Running test will be stopped.");
		box.exec();
	}
}

void HDDTestWidget::refDevice_accessWarning() {}

void HDDTestWidget::UpdateInfo(TestWidget::DataSet dataset)
{
	if(dataset == TestWidget::RESULTS)
	{
		// update info tab - tested device
		ui->model->setText(device.model);
		ui->serial->setText(device.serial);
		ui->firmware->setText(device.firmware);
		ui->size->setText(Def::FormatSize(device.size));
		ui->mountpoint->setText(device.mountpoint);
		ui->fstype->setText(device.fstype);
		ui->fsoptions->setText(device.fsoptions);
		ui->kernel->setText(device.kernel);
	}
	else if(dataset == TestWidget::REFERENCE)
	{
		// update info tab - reference devices
		ui->reference_model->setText(refDevice.model);
		ui->reference_serial->setText(refDevice.serial);
		ui->reference_firmware->setText(refDevice.firmware);
		ui->reference_size->setText(Def::FormatSize(refDevice.size));
		ui->reference_mountpoint->setText(refDevice.mountpoint);
		ui->reference_fstype->setText(refDevice.fstype);
		ui->reference_fsoptions->setText(refDevice.fsoptions);
		ui->reference_kernel->setText(refDevice.kernel);
	}
	else
	{
		std::cerr << "WARNING: Cannot update info for unknown dataset." << std::endl;
	}
}

void HDDTestWidget::ReloadTests(bool loaded)
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

void HDDTestWidget::EraseResults(TestWidget::DataSet dataset)
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

void HDDTestWidget::on_save_clicked()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save results"), "", tr("Results (*.hddtest)"));
	if(filename.length() > 0)
	{
		// Create base document for tests results
		QDomDocument doc("HddTest");

		// Create base element
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
		QFile file(filename);
		file.open(QIODevice::WriteOnly);
		QTextStream stream(&file);
		stream << doc.toString();
		file.close();
	}
}

void HDDTestWidget::OpenResultFile(QString filename, TestWidget::DataSet dataset)
{
	if(filename.length() == 0)
	{
		std::cerr << "WARNING: no filename given for results file" << std::endl;
		return;
	}

	// open file
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly))
	{
		std::cerr << "Cannot open results file" << std::endl;
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
		std::cerr << "Cannot set XML document context to file" << std::endl;
		return;
	}
	file.close();
	QDomElement root = doc.documentElement();

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

void HDDTestWidget::closeEvent(QCloseEvent *ev)
{
	// Check whenever something is in progress
	bool running = false;
	if(ui->filerwwidget->testState == TestWidget::STARTED)
		running = true;
	if(ui->filestructurewidget->testState == TestWidget::STARTED)
		running = true;
	if(ui->readblockwidget->testState == TestWidget::STARTED)
		running = true;
	if(ui->readcontwidget->testState == TestWidget::STARTED)
		running = true;
	if(ui->readrndwidget->testState == TestWidget::STARTED)
		running = true;
	if(ui->seekwidget->testState == TestWidget::STARTED)
		running = true;
	if(ui->smallfileswidget->testState == TestWidget::STARTED)
		running = true;

	if(running)
	{
		// Refuse to close and report to user
		ev->ignore();
		QMessageBox box;
		box.setText("Cannot close application since test is running.");
		box.exec();
	}
	else
		ev->accept();	// close
}

void HDDTestWidget::device_list_refresh()
{
	// Improves performance and sychromization
	ui->drive->blockSignals(true);
	ui->reference->blockSignals(true);

	// Clear device lists
	ui->drive->clear();
	ui->reference->clear();

	// Add drive selection to results combo box
	QList<Device::Item> devList = device.GetDevices();
	for(int i = 0; i < devList.size(); ++i)
		ui->drive->addItem(QIcon(":/icon/icon/hdd.svg"), devList[i].label, QVariant::fromValue(devList[i]));

	// Add nothing operation to reference combo box
	ui->drive->insertSeparator(ui->drive->count());
	ui->reference->addItem(
				QIcon::fromTheme("process-stop"),
				"Nothing",
				QVariant::fromValue(Device::Item::None()));
	ui->reference->insertSeparator(ui->reference->count());

	// Add saved resutls for both combo boxes
	QFileInfoList savedList = QDir("reference").entryInfoList(QStringList("*.hddtest"), QDir::Files);
	for(int i = 0; i < savedList.size(); ++i)
	{
		QString label = savedList[i].fileName();
		QString path = savedList[i].absoluteFilePath();
		Device::Item item = Device::Item::Saved(path);

		ui->drive->addItem(QIcon::fromTheme("document-open"), label, QVariant::fromValue(item));
		ui->reference->addItem(QIcon::fromTheme("document-open"), label, QVariant::fromValue(item));
	}

	// Add file open dialog for both combo boxes
	ui->drive->insertSeparator(ui->drive->count());
	ui->reference->insertSeparator(ui->reference->count());
	ui->drive->addItem(QIcon::fromTheme("document-open"), "Open another file", QVariant::fromValue(Device::Item::Open()));
	ui->reference->addItem(QIcon::fromTheme("document-open"), "Open another file", QVariant::fromValue(Device::Item::Open()));

	ui->drive->blockSignals(false);
	ui->reference->blockSignals(false);
}

void HDDTestWidget::on_about_clicked()
{
	About about;
	about.exec();
}

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

#ifndef HDDTEST_H
#define HDDTEST_H

#include <QObject>
#include <QDialog>
#include <QDir>
#include <QTimer>
#include <QGraphicsLineItem>
#include <QFont>
#include <QFileDialog>
#include <QCloseEvent>
#include <QMutex>

#include "about.h"
#include "seeker.h"
#include "readrnd.h"
#include "readcont.h"
#include "readblock.h"
#include "filerw.h"
#include "filestructure.h"
#include "smallfiles.h"

/// User interface classes
/** This namespace contains user interface classses **/
namespace Ui {
	class HDDTestWidget;
}

/// Main application window class
/** The HDDTestWidget class provides handles main application window.
  The benchmarks placed as tabs on this window are managed by separate classes **/
class HDDTestWidget : public QDialog {
	Q_OBJECT
public:
	HDDTestWidget(QWidget *parent = 0);
	~HDDTestWidget();

	/** Enables or disables tests depending on capabilities of selected device
	  @param loaded whenever the device is faked and resutls were loaded **/
	void ReloadTests(bool loaded);

	/** Erases results elected by dataset from all benchmarks
	  @param dataset selected results type **/
	void EraseResults(TestWidget::DataSet dataset);

	/** Updates information about slected device
	  @param dataset which device's information should be updated **/
	void UpdateInfo(TestWidget::DataSet dataset);

	/** Open results stored in file
	  @param filename of the file with results (can be full path)
	  @param dataset target dataset for stored results **/
	void OpenResultFile(QString filename, TestWidget::DataSet = TestWidget::RESULTS);

private:
	Ui::HDDTestWidget *ui;

	Device device;
	Device refDevice;

	QMutex driveRefreshMutex;

protected:
	void closeEvent(QCloseEvent *);

private slots:
	void on_save_clicked();
	void on_drive_currentIndexChanged(QString);
	void on_reference_currentIndexChanged(QString);
	void device_accessWarning();
	void device_operationError();
	void refDevice_accessWarning();
	void device_list_refresh();
	void on_about_clicked();
};

#endif // HDDTEST_H

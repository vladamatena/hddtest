/**
* \class HDDTest
*
* This class implements callbacks to main app GUI.
*
* \author Vladimír Matěna
*
* Contact vlada.matena@gmail.com
*
*/


#ifndef HDDTEST_H
#define HDDTEST_H

#include <QObject>
#include <QDialog>
#include <QDir>
#include <QTimer>
#include <QGraphicsLineItem>
#include <QFont>
#include <QFileDialog>

#include "seeker.h"
#include "readrnd.h"
#include "readcont.h"
#include "readblock.h"
#include "filerw.h"
#include "filestructure.h"
#include "smallfiles.h"

namespace Ui {
	class HDDTestWidget;
}


class HDDTestWidget : public QDialog {
	Q_OBJECT
public:
	HDDTestWidget(QWidget *parent = 0);
	~HDDTestWidget();

	void ReloadTests(bool loaded);
	void EraseResults(TestWidget::DataSet dataset);
	void UpdateInfo(TestWidget::DataSet dataset);
	void OpenResultFile(QString filename, TestWidget::DataSet = TestWidget::RESULTS);

private:
	Ui::HDDTestWidget *ui;

	Device device;
	Device refDevice;

private slots:
	void on_save_clicked();
	void on_drive_currentIndexChanged(QString);
	void on_reference_currentIndexChanged(QString);
	void device_accessWarning();
	void refDevice_accessWarning();
};

#endif // HDDTEST_H

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
    class HDDTest;
}


class HDDTest : public QDialog {
	Q_OBJECT
public:
	struct DeviceItem
	{
		enum Type	{ HDD_ITEM_DEVICE, HDD_ITEM_OPEN, HDD_ITEM_SAVED, HDD_ITEM_NONE };

		DeviceItem():
			type(HDD_ITEM_NONE), path("") {}
		DeviceItem(Type type, QString path):
			type(type), path(path) {}

		Type type;
		QString path;
	};

    HDDTest(QWidget *parent = 0);
    ~HDDTest();

	void ReloadTests(bool loaded);
	void EraseResults(TestWidget::DataSet dataset);
	void UpdateInfo(TestWidget::DataSet dataset);
	void OpenResultFile(QString filename, TestWidget::DataSet = TestWidget::RESULTS);

private:
    Ui::HDDTest *ui;

	Device device;
	Device refDevice;

private slots:
	void on_save_clicked();
	void on_drive_currentIndexChanged(QString);
	void on_reference_currentIndexChanged(QString);
	void device_accessWarning();
	void refDevice_accessWarning();
};

Q_DECLARE_METATYPE(HDDTest::DeviceItem)

#endif // HDDTEST_H

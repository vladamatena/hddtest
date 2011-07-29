/**
* \author Vladimír Matěna vlada.matena@gmail.com
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
#include <QCloseEvent>

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

protected:
	void closeEvent(QCloseEvent *);

private slots:
	void on_save_clicked();
	void on_drive_currentIndexChanged(QString);
	void on_reference_currentIndexChanged(QString);
	void device_accessWarning();
	void device_operationError();
	void refDevice_accessWarning();
};

#endif // HDDTEST_H

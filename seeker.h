/**
* \class Seeker
*
* Seeker test class. Implements Seeker test. The test test device for ramdom position access.
* Attempts to access different random positions on drive are made. Results are shown as dots.
* Dots shows dependency of seek time on seek length.
*
* \author Vladimír Matěna
*
* Contact vlada.matena@gmail.com
*
*/

#ifndef SEEKER_H
#define SEEKER_H

#include <QThread>
#include <QFile>
#include <QPointF>
#include <QtCore>
#include <QList>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QtXml>

#include "testwidget.h"
#include "device.h"
#include "randomgenerator.h"

/// Seeker
/// class implemeting function for running seek test
class Seeker : public TestWidget
{
private:
	/// Keeps information abou running or pased seek test
	class SeekResult
	{
	public:
		SeekResult():
				progress(0) {}

		void erase();
		void AddSeek(QPointF seek);		// add seek to this test
		qreal avg();	// get overall average seek time

		QList<QPointF> seeks;		// list of seeks
		QStack<QPointF> newseeks;	// list of new seeks (not yet displayed)
		unsigned int progress;		// percentage progress of the test		
	};
public:
	explicit Seeker(QWidget *parent = 0);
	~Seeker();

	static const hddsize SEEKER_BLOCKSIZE = 512 * Device::B;	/// seek read size
	static const hddsize SEEKER_SEEKCOUNT = 1000;

	SeekResult result;		// seek results
	SeekResult reference;	// seek reference results

	void TestLoop();
	void InitScene();
	void UpdateScene();

	int GetProgress();

	QDomElement WriteResults(QDomDocument &doc);	// writes results of test to XML
	void RestoreResults(QDomElement &root, DataSet dataset);			// reads results from XML document
	void EraseResults(DataSet dataset);

private:
	Line *dataAvgLine;
	Line *referenceAvgLine;

	Ticks *dataTicks;
	Ticks *referenceTicks;

	Net* net;
};

#endif // SEEKER_H

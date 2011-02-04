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

#define BLOCKSIZE 512
#define SEEKCOUNT 1000

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

#define SEEK_MAX_AVG 15	// how many max values are used to calculate final max
#define SEEK_MIN_AVG 10	// how many min values are used to calculate final min

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
		qreal max();	// get max seek time (average of slowest seeks)
		qreal min();	// get min seek time (average of fastest seeks)
		qreal avg();	// get overall average seek time

		QList<QPointF> seeks;		// list of seeks
		QStack<QPointF> newseeks;	// list of new seeks (not yet displayed)
		unsigned int progress;		// percentage progress of the test
		QList<qreal> maxims;		// list of longest seek times
		QList<qreal> mins;			// list of shortest seek times
	};
public:
	explicit Seeker(QWidget *parent = 0);
	~Seeker();

	SeekResult result;		// seek results
	SeekResult reference;	// seek reference results

	void TestLoop();
	void InitScene();
	void UpdateScene();

	qreal GetProgress();

	QDomElement WriteResults(QDomDocument &doc);	// writes results of test to XML
	void RestoreResults(QDomElement &root, bool reference = false);			// reads results from XML document

private:
	Line *dataAvgLine;
	Line *referenceAvgLine;

	Ticks *dataTicks;
	Ticks *referenceTicks;
};

#endif // SEEKER_H

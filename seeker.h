/**
*
* \author Vladimír Matěna vlada.matena@gmail.com
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

/// Seeker benchmark main class
/** Seeker test class. Implements Seeker test. The test test device for ramdom position access.
Attempts to access different random positions on drive are made. Results are shown as dots.
Dots shows dependency of seek time on seek length. **/
class Seeker : public TestWidget
{
private:
	/// Keeps information about running or pased seek test
	class SeekResult
	{
	public:
		SeekResult():
				progress(0) {}

		void erase();					/// Erase all seeks
		void AddSeek(QPointF seek);		/// Add seek to this test
		qreal avg();					/// Get overall average seek time

		QList<QPointF> seeks;		/// List of seeks in results
		QStack<QPointF> newseeks;	/// List of new seeks (not yet displayed)
		unsigned int progress;		/// Percentage progress of the test
	};
public:
	explicit Seeker(QWidget *parent = 0);
	~Seeker();

	static const hddsize SEEKER_BLOCKSIZE = 512 * B;	/// seek read size
	static const hddsize SEEKER_SEEKCOUNT = 1000;				/// number of seeke per test
	static const int SEEKER_IMPORTANT = 2;						/// seeks slower than N * average are not important

	SeekResult result;		/// Seek results
	SeekResult reference;	/// Seek reference results

	void TestLoop();	/// Main benchmark code
	void InitScene();	/// Initializes scene before benchmark begins
	void UpdateScene();	/// Updates scene
	int GetProgress();	/// Returns benchmark progress

	QDomElement WriteResults(QDomDocument &doc);				/// Writes results of test to XML
	void RestoreResults(QDomElement &root, DataSet dataset);	/// Reads results from XML document
	void EraseResults(DataSet dataset);							/// Erases selected results

private:
	Line *dataAvgLine;
	Line *referenceAvgLine;

	Ticks *dataTicks;
	Ticks *referenceTicks;

	Net* net;
};

#endif // SEEKER_H

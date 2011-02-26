/**
* \class ReadRnd
*
* Read random test. The test reads blocks of differsent sizes from random positions on the device.
* Bar graphs for every block size aredrawn to the graph.
*
* \author Vladimír Matěna
*
* Contact vlada.matena@gmail.com
*
*/

#ifndef READRND_H
#define READRND_H

#include "testwidget.h"
#include "randomgenerator.h"
#include "device.h"

#define READ_RND_SIZE 104857600

// Class for keepeg subtest results and progress
class ReadRndResult
{
public:
	ReadRndResult(qint32 block_size);

	qint64 __bytes_read;
	qint64 __time_elapsed;
	qint32 __block_size;

	qreal max;

	void erase();
};

// Class for running random read test
class ReadRnd : public TestWidget
{
public:
	// ReadRnd class constructor
	ReadRnd(QWidget *parent = 0);

	void TestLoop();
	void InitScene();
	void UpdateScene();

	// list of subtest results
	QList<ReadRndResult> results;
	QList<ReadRndResult> reference;

	// test progress
	qreal GetProgress();

	QDomElement WriteResults(QDomDocument &doc);	// writes results of test to XML
	void RestoreResults(QDomElement &root, bool reference = false);			// reads results from XML document
	void EraseResults();

private:
	QList<Bar*> bars;
	QList<Bar*> reference_bars;
};

#endif // READRND_H

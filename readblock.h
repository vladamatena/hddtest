/**
* \class ReadBlock
*
* Read Block test. The test reads blocks of differsent sizes from the device.
* Bar graphs for every block size aredrawn to the graph.
*
* \author Vladimír Matěna
*
* Contact vlada.matena@gmail.com
*
*/

#ifndef READBLOCK_H
#define READBLOCK_H

#include "testwidget.h"
#include "randomgenerator.h"
#include "device.h"

#define READ_BLOCK_SIZE 104857600

// Class for keepeg subtest results and progress
class ReadBlockResult
{
public:
	ReadBlockResult(qint32 block_size);

	qint64 __bytes_read;
	qint64 __time_elapsed;
	qint32 __block_size;

	qreal max;

	void erase();
};

// Class for running random read test
class ReadBlock : public TestWidget
{
public:
	// ReadRnd class constructor
	ReadBlock(QWidget *parent = 0);

	void TestLoop();
	void InitScene();
	void UpdateScene();

	// list of subtest results
	QList<ReadBlockResult> results;
	QList<ReadBlockResult> reference;

	// test progress
	int GetProgress();

	QDomElement WriteResults(QDomDocument &doc);	// writes results of test to XML
	void RestoreResults(QDomElement &root, DataSet = RESULTS);			// reads results from XML document
	void EraseResults();

private:
	QList<Bar*> bars;
	QList<Bar*> reference_bars;
};

#endif // READBLOCK_H

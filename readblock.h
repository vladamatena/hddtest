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

// Class for keepeg subtest results and progress
class ReadBlockResult
{
public:
	ReadBlockResult(hddsize block_size);

	hddsize __bytes_read;
	hddtime __time_elapsed;
	hddsize __block_size;

	void erase();
};

// Class for running random read test
class ReadBlock : public TestWidget
{
public:
	// ReadRnd class constructor
	ReadBlock(QWidget *parent = 0);

	static const hddsize READ_BLOCK_SIZE = 100 * M;
	static const hddsize READ_BLOCK_BASE_BLOCK_SIZE = 1 * M;
	static const int READ_BLOCK_BLOCK_SIZE_COUNT = 12;
	static const int READ_BLOCK_BLOCK_SIZE_STEP = 2;

	void TestLoop();
	void InitScene();
	void UpdateScene();

	// list of subtest results
	QList<ReadBlockResult> results;
	QList<ReadBlockResult> reference;

	// test progress
	int GetProgress();

	QDomElement WriteResults(QDomDocument &doc);	// writes results of test to XML
	void RestoreResults(QDomElement &root, DataSet dataset);			// reads results from XML document
	void EraseResults(DataSet dataset);

private:
	QList<Bar*> bars;
	QList<Bar*> reference_bars;
};

#endif // READBLOCK_H

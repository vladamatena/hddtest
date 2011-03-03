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

// Class for keepeg subtest results and progress
class ReadRndResult
{
public:
	ReadRndResult(hddsize block_size);

	hddsize __bytes_read;
	hddtime __time_elapsed;
	hddsize __block_size;
	hddsize __blocks_done;

	void erase();
};

// Class for running random read test
class ReadRnd : public TestWidget
{
public:
	// ReadRnd class constructor
	ReadRnd(QWidget *parent = 0);

	static const hddsize READ_RND_SIZE = 100;

	void TestLoop();
	void InitScene();
	void UpdateScene();

	// list of subtest results
	QList<ReadRndResult> results;
	QList<ReadRndResult> reference;

	// test progress
	int GetProgress();

	QDomElement WriteResults(QDomDocument &doc);	// writes results of test to XML
	void RestoreResults(QDomElement &root, DataSet dataset);			// reads results from XML document
	void EraseResults(DataSet dataset);

private:
	QList<Bar*> bars;
	QList<Bar*> reference_bars;
};

#endif // READRND_H

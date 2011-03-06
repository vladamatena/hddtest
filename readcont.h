/**
* \class ReadCont
*
* Implenets read continuos test. The test reads blocks from different size from the device.
* Blocks are read continuosly (one by one) for every size from the beginning of the device.
* Bar graph for wvery block size are shown.
*
* \author Vladimír Matěna
*
* Contact vlada.matena@gmail.com
*
*/


#ifndef READCONT_H
#define READCONT_H

#include <QThread>
#include <QList>
#include <QStack>
#include <QString>

#include "device.h"
#include "testwidget.h"

class ReadContResults
{
public:
	ReadContResults();

	QList<qreal> results;
	QQueue<qreal> new_results;
	int blocks;
	qreal avg;
	int blocks_done;

	void AddResult(qreal result);
	void erase();
};

class ReadCont : public TestWidget
{
public:
	ReadCont(QWidget *parent = 0);
	~ReadCont();

	static const hddsize READ_CONT_SIZE = 4096 * Device::M;
	static const hddsize READ_CONT_BLOCK = 5 * Device::M;

	void TestLoop();
	void InitScene();
	void UpdateScene();
	int GetProgress();

	ReadContResults results;
	ReadContResults reference;

	QDomElement WriteResults(QDomDocument &doc);					// writes results of test to XML
	void RestoreResults(QDomElement &root, DataSet dataset);		// reads results from XML document
	void EraseResults(DataSet dataset);

private:
	LineGraph *graph;
	LineGraph *refGraph;

	Net *net;

	Line *averageLine;
	Line *refAverageLine;
};


#endif // READCONT_H

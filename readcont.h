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

#define READ_CONT_SIZE 4294967296
#define READ_CONT_BLOCK 52428800

class ReadContResults
{
public:
	ReadContResults();

	QList<qreal> results;
	QStack<qreal> new_results;
	int blocks;
	qreal avg;
	qreal max;
	qreal min;
	int blocks_done;
	int progress;

	void AddResult(qreal result);
	void erase();
};

class ReadCont : public TestWidget
{
public:
	ReadCont(QWidget *parent = 0);
	~ReadCont();

	void TestLoop();
	void InitScene();
	void UpdateScene();
	int GetProgress();

	ReadContResults results;
	ReadContResults reference;

	QDomElement WriteResults(QDomDocument &doc);						// writes results of test to XML
	void RestoreResults(QDomElement &root, bool reference = false);		// reads results from XML document
	void EraseResults();

private:
//	bool first;
//	qreal last;

	LineGraph *graph;
	LineGraph *refGraph;

	Net *net;

	Line *averageLine;
	Line *refAverageLine;
	//Line *min_line;
	//Line *max_line;
};


#endif // READCONT_H

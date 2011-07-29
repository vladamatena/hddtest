/**
* \author Vladimír Matěna vlada.matena@gmail.com
*/

#ifndef READCONT_H
#define READCONT_H

#include <QThread>
#include <QList>
#include <QStack>
#include <QString>

#include "device.h"
#include "testwidget.h"

/// Stores Read Continuous benchmark results
/** ReadContResults class encapsulates read Continuous benchmark results
  @see ReadCont class **/
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

/// Read Continuous benchmark main class
/** Implenets read continuos test. The test reads blocks from different size from the device.
Blocks are read continuosly (one by one) for every size from the beginning of the device.
Bar graph for wvery block size are shown.
@see ReadContResults **/
class ReadCont : public TestWidget
{
public:
	ReadCont(QWidget *parent = 0);	/// The constructor
	~ReadCont(); /// The destructor

	static const hddsize READ_CONT_SIZE = 4096 * M;	/// Size of data read from device
	static const hddsize READ_CONT_BLOCK = 4 * M;	/// Block size by which data are read

	void TestLoop();	/// Main benchmark code
	void InitScene();	/// Initializes scene before benchmark begins
	void UpdateScene();	/// Updates scene
	int GetProgress();	/// Returns benchmark progress

	ReadContResults results;	/// Primary results
	ReadContResults reference;	/// Reference results

	QDomElement WriteResults(QDomDocument &doc);				/// Writes results of test to XML
	void RestoreResults(QDomElement &root, DataSet dataset);	/// Reads results from XML document
	void EraseResults(DataSet dataset);							/// Erase elected results

private:
	LineGraph *graph;
	LineGraph *refGraph;

	Net *net;

	Line *averageLine;
	Line *refAverageLine;
};


#endif // READCONT_H

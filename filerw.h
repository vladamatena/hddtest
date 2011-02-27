/**
* \class FileRW
*
* This class implemets file read - write test.
* The test writes file to safe temp an then reads it again.
* Both operations are visualised by line graph showing speed
*
* \author Vladimír Matěna
* Contact vlada.matena@gmail.com
*/

#ifndef FILERW_H
#define FILERW_H

#include "testwidget.h"

#define FILERW_SIZE 1073741824
#define FILERW_BLOCK 10485760

class FileRWResults
{
public:
	FileRWResults();

	QList<qreal> results;		/// all colected results
	QStack<qreal> new_results;	/// new results to be drawn to graph
	int blocks;					/// total blocks count
	qreal avg;					/// average speed
	qreal max;					/// max speed
	int blocks_done;			/// blocks already done

	void AddResult(qreal result);
	void erase();
};

class FileRW : public TestWidget
{
public:
	FileRW(QWidget *parent = 0);
	~FileRW();

	// members from Test
	void TestLoop();
	void InitScene();
	void UpdateScene();
	int GetProgress();

	FileRWResults results_write;	// results of write test
	FileRWResults results_read;		// results of read test
	FileRWResults reference_write;	// reference results for write test
	FileRWResults reference_read;	// reference results for read test

	QDomElement WriteResults(QDomDocument &doc);	// writes results of test to XML
	void RestoreResults(QDomElement &root, bool reference = false);			// reads results from XML document
	void EraseResults();

private:
	bool __first;
	qreal __last;

	LineGraph *__read_graph;
	LineGraph *__write_graph;
	LineGraph *__read_reference_graph;
	LineGraph *__write_reference_graph;

	Net *__net;

	Line *__avg_line;
	Line *__max_line;

	Legend *__legend;
};

#endif // FILERW_H

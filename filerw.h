/*******************************************************************************
*
*	HDDTest the graphical drive benchmarking tool.
*	Copyright (C) 2011  Vladimír Matěna <vlada.matena@gmail.com>
*
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
********************************************************************************/

#ifndef FILERW_H
#define FILERW_H

#include "definitions.h"
#include "testwidget.h"
#include "file.h"

/// Stores FileRW benchmark resutls
/** The FileRWResults class encapsules FileRW benchmark results.
  @see FileRW class **/
class FileRWResults {
public:
	FileRWResults();	/// The constructor

	QList<qreal> results;		/// all colected results
	QQueue<qreal> new_results;	/// new results to be drawn to graph
	int blocks;					/// total blocks count
	qreal avg;					/// average speed
	int blocks_done;			/// blocks already done

	/** Add new results
	  @param result value to be added **/
	void AddResult(qreal result);
	void erase();
};

/// FileRW benchmark main class
/** This class implemets file read - write test. The test writes
file to safe temp an then reads it again. Both operations are
visualised by line graph showing the speed.
@see FileRWResults **/
class FileRW : public TestWidget {
public:
	FileRW(QWidget *parent = 0);	/// The constructor
	~FileRW();	/// The destructor

	/// Count of bytes written and read to/from file.
	static const hddsize FILERW_SIZE = 1024 * M;

	/// Block size by which file is written/read
	static const hddsize FILERW_BLOCK = 4 * M;

	// members from Test
	void TestLoop();	/// Main benchmark code
	void InitScene();	/// Initializes scene before benchmark begins
	void UpdateScene();	/// Updates scene
	int GetProgress();	/// Returns benchmark progress

	FileRWResults results_write;	/// Results of write test
	FileRWResults results_read;		/// Results of read test
	FileRWResults reference_write;	/// Reference results for write test
	FileRWResults reference_read;	/// Reference results for read test

	QDomElement WriteResults(QDomDocument &doc); /// Writes results of test to XML
	void RestoreResults(QDomElement &root, DataSet dataset); /// Reads results from XML document
	void EraseResults(DataSet dataset);	/// Erases selected results

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

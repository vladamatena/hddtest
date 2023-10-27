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

#pragma once

#include "testwidget.h"
#include "randomgenerator.h"
#include "device.h"

/// Stores ReadBlock benchmark results
/** Class for keepeg ReadBlock subtest results and progress
@see ReadBlock class **/
class ReadBlockResult {
public:
	ReadBlockResult(hddsize block_size);

	hddsize __bytes_read;	/// Count of bytes read by selected block size
	hddtime __time_elapsed;	/// Time elased while reading
	hddsize __block_size;	/// Size of the block for this subtest

	void erase(); /// Erase all values
};

/// Read Block benchmark main class
/** Read Block test. The test reads blocks of differsent sizes from the device.
Bar graphs for every block size are drawn to the graph.
@see ReadBlockResults class **/
class ReadBlock : public TestWidget {
public:
	// ReadRnd class constructor
	ReadBlock(QWidget *parent = 0);	/// The constructor

	static const hddsize READ_BLOCK_SIZE = 100 * M;				/// Data to be read by every block size
	static const hddsize READ_BLOCK_BASE_BLOCK_SIZE = 1 * M;	/// base block size for first substes
	static const int READ_BLOCK_BLOCK_SIZE_COUNT = 12;			/// Subtest count
	static const int READ_BLOCK_BLOCK_SIZE_STEP = 2;			/// Divisior for next subtest

	void TestLoop();	/// Main benchmark code
	void InitScene();	/// Initializes scene before benchmark begins
	void UpdateScene();	/// Updates scene
	int GetProgress();	/// Returns benchmark progress

	// list of subtest results
	QList<ReadBlockResult> results;		/// Primary results
	QList<ReadBlockResult> reference;	/// Reference results

	QDomElement WriteResults(QDomDocument &doc);				/// Writes results of test to XML
	void RestoreResults(QDomElement &root, DataSet dataset);	/// Reads results from XML document
	void EraseResults(DataSet dataset);							/// Erases selected results

private:
	QList<Bar*> bars;
	QList<Bar*> reference_bars;
};

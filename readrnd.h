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

/// Stores Read Random benchmark results
/** ReadRndResults class for keepeg subtest results and progress
@see ReadRnd class **/
class ReadRndResult {
public:
	ReadRndResult(hddsize block_size);	/// The constructor

	hddsize __bytes_read;		/// Number of bytes read in this subtest
	hddtime __time_elapsed;		/// time spend reading in this subtest
	hddsize __block_size;		/// Block size in this subtest
	hddsize __blocks_done;		/// Count of blocks done in this subtest

	void erase();	/// Erase results
};

/// ReadRandom benchmark main class
/** Read random test. The test reads blocks of differsent sizes from random positions on the device.
Bar graphs for every block size aredrawn to the graph.
@see ReadRndResults **/
class ReadRnd : public TestWidget {
public:
	ReadRnd(QWidget *parent = 0); ///ReadRnd class constructor

	static const hddsize READ_RND_SIZE = 100;				/// Read random benchmark subtest data size
	static const hddsize READ_RND_BASE_BLOCK_SIZE = 1 * M;	/// Base block size (first subtest block size)
	static const int READ_RND_BLOCK_SIZE_COUNT = 12;		/// Subtest count
	static const int READ_RND_BLOCK_SIZE_STEP = 2;			/// next subtest divisior

	void TestLoop();	/// Main benchmark code
	void InitScene();	/// Initializes scene before benchmark begins
	void UpdateScene();	/// Updates scene
	int GetProgress();	/// Returns benchmark progress

	// list of subtest results
	QList<ReadRndResult> results;	/// Primary results
	QList<ReadRndResult> reference;	/// Reference results

	QDomElement WriteResults(QDomDocument &doc);				/// Writes results of test to XML
	void RestoreResults(QDomElement &root, DataSet dataset);	/// Reads results from XML document
	void EraseResults(DataSet dataset);							/// Erases selected resutls

private:
	QList<Bar*> bars;
	QList<Bar*> reference_bars;
};

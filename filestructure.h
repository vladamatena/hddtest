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

#ifndef FILESTRUCTURE_H
#define FILESTRUCTURE_H

#include "testwidget.h"
#include "randomgenerator.h"

/// Stores FileRW benchmark results
/** FileStructureResults class encapsulates Structure benchmark results
  @see FileStructure **/
class FileStructureResults {
public:
	/// benchmark phase
	enum Phase { PHASE_NONE, PHASE_BUILD, PHASE_DESTROY, PHASE_DONE };

	FileStructureResults();	/// The Constructore
	void erase();			/// Erase results

	int build_files;	/// Count of created files
	int build_dirs;		/// Count of created directories
	int destroyed;		/// Count of deleted files and directories
	Phase phase;		/// Phase of the benchmark
	bool done;			/// Whenever the benchmark has finished

	hddtime build;		/// Time needed to create files and directories
	hddtime destroy;	/// Time
};

/// FileRW benchmark main class
/** This class implements File Structure test. The test build dirs
and files in a huge structure and draw bar graphs with operation times.
@see FileStructureResults class **/
class FileStructure : public TestWidget {
public:
	FileStructure(QWidget *parent = 0);	/// The constructore

	/// Size of the structure used for benchmarking
	static const hddsize FILESTRUCTURE_SIZE = 1000;

	void TestLoop();	/// Main benchmark code
	void InitScene();	/// Initializes scene before benchmark begins
	void UpdateScene();	/// Updates scene
	int GetProgress();	/// Returns benchmark progress

	FileStructureResults results;	/// Primary results
	FileStructureResults reference;	/// reference results

	QDomElement WriteResults(QDomDocument &doc);				/// Writes results of test to XML
	void RestoreResults(QDomElement &root, DataSet dataset);	/// Reads results from XML document
	void EraseResults(DataSet dataset);							/// Erases selected results

private:
	Bar *build_bar;
	Bar *destroy_bar;
	Bar *build_reference_bar;
	Bar *destroy_reference_bar;
};

#endif // FILESTRUCTURE_H

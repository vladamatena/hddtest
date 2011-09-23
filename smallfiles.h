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

#ifndef SMALLFILES_H
#define SMALLFILES_H

#include "testwidget.h"
#include "randomgenerator.h"

/// Stores Small Files benchmark results
/** SmallFilesResults class encapsulates resutls of SmallFiles benchmark
  @see SmallFiles class **/
class SmallFilesResults
{
public:
	/** Enumerates different phases of the smallfiles benchmark **/
	enum Phase { PHASE_NONE, PHASE_DIR_BUILD, PHASE_FILE_BUILD, PHASE_FILE_READ, PHASE_DESTROY, PHASE_DONE };

	SmallFilesResults(); /// The SmallFilesresults constructor

	hddtime dir_build_time;		/// Directory build time
	hddtime file_build_time;	/// File build time
	hddtime file_read_time;		/// File read time
	hddtime destroy_time;		/// Files and directories erase time

	int dirs_build;		/// Count of dirs build
	int files_build;	/// Count of files build
	int files_read;		/// Count of files read
	int destroyed;		/// Count of files and direories deleted
	bool done;		/// Whenever the benchmark has finished

	/** The phase of the benchmark. This is needed when realtime graph is constructed
	 int order to add sync operation time to preciously measured operation time. **/
	Phase phase;

	void erase();	/// Erases all results
};

/// Small Files benchmark main class
/** Small files test class. Small files test is focuse on working with mixure of small files and dirs.
A huge structure of files and dirs is build then files are read and the whole structure is destroyed again.
Times of all kinds of operations are measured and whown in the bar graph.
@see SmallFilesResults class **/
class SmallFiles : public TestWidget
{
public:
	SmallFiles(QWidget *parent = 0);	/// The SmallFiles constructor

	/** Count of the files and directories used in the benchmark **/
	static const int SMALLFILES_SIZE = 1000;

	void InitScene();	/// Initializes the graph scene
	void TestLoop();	/// The main benchmark code
	void UpdateScene(); /// Updates graph
	int GetProgress();	/// Returns benchmark progress

	SmallFilesResults results;		/// Primary resutls
	SmallFilesResults reference;	/// Reference results

	QDomElement WriteResults(QDomDocument &doc);				/// Writes results of test to XML
	void RestoreResults(QDomElement &root, DataSet dataset);	/// Reads results from XML document
	void EraseResults(DataSet dataset);							/// Erases results

private:
	Bar *build_dir_bar;
	Bar *build_files_bar;
	Bar *read_files_bar;
	Bar *destroy_bar;

	Bar *build_dir_reference_bar;
	Bar *build_files_reference_bar;
	Bar *read_files_reference_bar;
	Bar *destroy_reference_bar;

};

#endif // SMALLFILES_H

/**
* \author Vladimír Matěna vlada.matena@gmail.com
*/

#ifndef FILESTRUCTURE_H
#define FILESTRUCTURE_H

#include "testwidget.h"
#include "randomgenerator.h"

/// Stores FileRW benchmark results
/** FileStructureResults class encapsulates Structure benchmark results
  @see FileStructure **/
class FileStructureResults
{
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
class FileStructure : public TestWidget
{
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

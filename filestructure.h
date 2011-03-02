/**
* \class FileStructure
*
* This class implements File Structure test.
* The test build dirs and files in a huge structure and draw bar graphs with operation times.
*
* \author Vladimír Matěna
*
* Contact vlada.matena@gmail.com
*
*/

#ifndef FILESTRUCTURE_H
#define FILESTRUCTURE_H

//#include "test.h"
#include "testwidget.h"
#include "randomgenerator.h"

class FileStructureResults
{
public:
	FileStructureResults();
	void erase();

	void AddBuild(hddtime time);
	void AddDestroy(hddtime time);

	int build_files;
	int build_dirs;
	int destroyed;

	hddtime build;
	hddtime destroy;
	hddtime max;

	int progress;
};

class FileStructure : public TestWidget
{
public:
	FileStructure(QWidget *parent = 0);

	static const hddsize FILESTRUCTURE_SIZE = 1000;

	void InitScene();
	void TestLoop();
	void UpdateScene();
	int GetProgress();

	FileStructureResults results;
	FileStructureResults reference;

	QDomElement WriteResults(QDomDocument &doc);					// writes results of test to XML
	void RestoreResults(QDomElement &root, DataSet dataset);	// reads results from XML document
	void EraseResults(DataSet dataset);

private:
	Bar *build_bar;
	Bar *destroy_bar;
	Bar *build_reference_bar;
	Bar *destroy_reference_bar;
};

#endif // FILESTRUCTURE_H

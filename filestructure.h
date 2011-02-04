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

#define FILESTRUCTURE_SIZE 1000

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
};

class FileStructure : public TestWidget
{
public:
	FileStructure(QWidget *parent = 0);

	void InitScene();
	void TestLoop();
	void UpdateScene();
	qreal GetProgress();

	FileStructureResults results;

	QDomElement WriteResults(QDomDocument &doc);	// writes results of test to XML
	void RestoreResults(QDomElement &root, bool reference = false);			// reads results from XML document

	int progress;

private:
	Bar *build_bar;
	Bar *destroy_bar;
};

#endif // FILESTRUCTURE_H

/**
* \class Small files
*
* Small files test class. Small files test is focuse on working with mixure of small files and dirs.
* A huge structure of files and dirs is build then files are read and the whole structure is destroyed again.
* Times of all kinds of operations are measured and whown in the bar graph.
*
* \author Vladimír Matěna
*
* Contact vlada.matena@gmail.com
*
*/

#ifndef SMALLFILES_H
#define SMALLFILES_H

#include "testwidget.h"
#include "randomgenerator.h"

class SmallFilesResults
{
public:
	SmallFilesResults();

	hddtime dir_build_time;
	hddtime file_build_time;
	hddtime file_read_time;
	hddtime destroy_time;

	int dirs_build;
	int files_build;
	int files_read;
	int destroyed;
	bool done;

	void erase();
};

class SmallFiles : public TestWidget
{
public:
	SmallFiles(QWidget *parent = 0);

	static const int SMALLFILES_SIZE = 1000;

	void InitScene();
	void TestLoop();
	void UpdateScene();
	int GetProgress();

	SmallFilesResults results;
	SmallFilesResults reference;

	QDomElement WriteResults(QDomDocument &doc);					// writes results of test to XML
	void RestoreResults(QDomElement &root, DataSet dataset);	// reads results from XML document
	void EraseResults(DataSet dataset);

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

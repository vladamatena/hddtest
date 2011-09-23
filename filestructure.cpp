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

#include "filestructure.h"

FileStructure::FileStructure(QWidget *parent):
	TestWidget(parent)
{
	build_bar = this->addBar(				"s", "Structure build",		QColor(255,	0,	0),		0.1, 0.16);
	build_reference_bar = this->addBar(		"s", "Structure build",		QColor(0,	0,	255),	0.31, 0.16);
	destroy_bar = this->addBar(				"s", "Structure destroy",	QColor(255,	64,	0),		0.53, 0.16);
	destroy_reference_bar = this->addBar(	"s", "Structure destroy",	QColor(0,	64,	255),	0.74, 0.16);

	testName = "File structure";
	testDescription = "File structure test creates random directory structure containing " +
			QString::number(FILESTRUCTURE_SIZE) + " files and " + QString::number(FILESTRUCTURE_SIZE) +
			" directories. Then this structure si deleted." +
			" Construct and delete oprations times are shown in bar graph." +
			" This test can only be performed on mounted filesystem.";
}

void FileStructure::TestLoop()
{
	// init random
	RandomGenerator random;

	// list of nodes in test
	int counter = 1;
	QList<QString> files;
	QList<QString> nodes;
	nodes.push_back(device->GetSafeTemp());	//	add initial node

	// create structure
	device->DropCaches();
	results.phase = FileStructureResults::PHASE_BUILD;
	while((results.build_files < FILESTRUCTURE_SIZE) || (results.build_dirs < FILESTRUCTURE_SIZE))
	{
		if((!(results.build_files < FILESTRUCTURE_SIZE)) || (random.Get32() % 2 == 0))
		{
			// construct new node name and path
			QString name = QString::number(counter++);
			QString path = nodes.at(random.Get64() % nodes.size());
			nodes.push_back(path + "/" + name);

			// create node
			results.build += device->MkDir(path + "/" + name);

			++results.build_dirs;
		}
		else
		{
			// construct file name and path
			QString name = QString::number(counter++);
			QString path = nodes.at(random.Get64() % nodes.size());
			files.push_back(path + "/" + name);

			// create file
			results.build += device->MkFile(path + "/" + name, 0);

			++results.build_files;
		}

		if(testState == STOPPING)
			break;
	}
	results.build += device->Sync();


	// make sure structure is not cached
	device->DropCaches();
	results.phase = FileStructureResults::PHASE_DESTROY;

	// delete files
	for(int i = 0; i < files.size(); ++i)
	{
		results.destroy += device->DelFile(files[i]);
		++results.destroyed;
	}
	results.destroy += device->Sync();

	// delete dirs
	for(int i = nodes.size() - 1; i > 0 ; --i)	// node 0 is temp directory in which is test running
	{
		results.destroy += device->DelDir(nodes[i]);
		++results.destroyed;
	}
	results.destroy += device->Sync();
	results.done = true;
	results.phase = FileStructureResults::PHASE_DONE;

	device->ClearSafeTemp();
}

void FileStructure::InitScene()
{
	results.erase();
}

void FileStructure::UpdateScene()
{
	// get result progress
	hddtime build = results.build;
	hddtime destroy = results.destroy;

	// add current operation progress
	if(device)
	{
		if(results.phase == FileStructureResults::PHASE_BUILD)
			build += device->timer.GetCurrentOffset();
		else if (results.phase == FileStructureResults::PHASE_DESTROY)
			destroy += device->timer.GetCurrentOffset();
	}

	// update result bars
	build_bar->Set(
			(100 * (results.build_dirs + results.build_files)) / (2 * FILESTRUCTURE_SIZE),
			(qreal) build / s);

	destroy_bar->Set(
			100 * results.destroyed / (2 * FILESTRUCTURE_SIZE),
			(qreal)destroy / s) ;

	// update reference bars
	build_reference_bar->Set(
			(100 * (reference.build_dirs + reference.build_files)) / (2 * FILESTRUCTURE_SIZE),
			(qreal)reference.build / s);

	destroy_reference_bar->Set(
			100 * reference.destroyed / (2 * FILESTRUCTURE_SIZE),
			(qreal)reference.destroy / s) ;

	// rescale
	Rescale();
}

int FileStructure::GetProgress()
{
	// do not show 100% until done
	int state = results.build_files + results.build_dirs + results.destroyed + results.done;
	int target = 4 * FILESTRUCTURE_SIZE;
	return (100 * state) / target;
}

FileStructureResults::FileStructureResults()
{
	erase();
}

void FileStructureResults::erase()
{
	build_files = 0;
	build_dirs = 0;
	destroyed = 0;

	destroy = 0.0f;
	build = 0.0f;

	done = false;
	phase = PHASE_NONE;
}

QDomElement FileStructure::WriteResults(QDomDocument &doc)
{
	// create main seek element
	QDomElement master = doc.createElement("File_Structure");
	master.setAttribute("valid", (GetProgress() == 100)?"yes":"no");
	doc.appendChild(master);

	// add build element
	QDomElement build = doc.createElement("Build");
	build.setAttribute("time", results.build);
	master.appendChild(build);

	// add destroy element
	QDomElement destroy = doc.createElement("Destroy");
	destroy.setAttribute("time", results.destroy);
	master.appendChild(destroy);

	return master;
}

void FileStructure::RestoreResults(QDomElement &results, DataSet dataset)
{
	FileStructureResults *res = (dataset == REFERENCE)?&this->reference:&this->results;

	// Locate main seek element
	QDomElement main = results.firstChildElement("File_Structure");
	if(!main.attribute("valid", "no").compare("no"))
		return;

	// init scene and remove results
	res->erase();

	//// get Build
	QDomElement build = main.firstChildElement("Build");
	if(build.isNull())
		return;
	res->build = build.attribute("time", "0").toDouble();

	//// get Destroy
	QDomElement destroy = main.firstChildElement("Destroy");
	if(destroy.isNull())
		return;
	res->destroy = destroy.attribute("time", "0").toDouble();

	// set progress and update scene
	res->build_dirs = FILESTRUCTURE_SIZE;
	res->build_files = FILESTRUCTURE_SIZE;
	res->destroyed = FILESTRUCTURE_SIZE * 2;

	UpdateScene();
}

void FileStructure::EraseResults(DataSet dataset)
{
	if(dataset == RESULTS)
		results.erase();
	else
		reference.erase();

	UpdateScene();
}

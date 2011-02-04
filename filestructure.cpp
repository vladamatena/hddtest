/**
*
* \author Vladimír Matěna
*
* Contact vlada.matena@gmail.com
*
*/

#include "filestructure.h"

FileStructure::FileStructure(QWidget *parent):
	TestWidget(parent)
{
	build_bar = this->addBar("s", "Structure build", QColor(0, 255, 0), 0.2, 0.2);
	destroy_bar = this->addBar("s", "Structure destroy", QColor(255, 0, 0), 0.7, 0.2);
}

void FileStructure::TestLoop()
{
	// init random
	RandomGenerator random;

	// list of nodes in test
	QList<QString> files;
	QList<QString> nodes;
	nodes.push_back(device->GetSafeTemp());	//	add initial node
	progress = 0;

	// create structure
	while(
			(results.build_files < FILESTRUCTURE_SIZE) ||
			(results.build_dirs < FILESTRUCTURE_SIZE))
	{
		if((!(results.build_files < FILESTRUCTURE_SIZE)) || (random.Get32() % 2 == 0))
		{
			// construct new node name and path
			QString name = QString::number(nodes.size());
			QString path = nodes.at(random.Get64() % nodes.size());
			nodes.push_back(path + "/" + name);

			// create node
			results.AddBuild(device->MkDir(path + "/" + name));

			++results.build_dirs;
		}
		else
		{
			// construct file name and path
			QString name = QString::number(files.size());
			QString path = nodes.at(random.Get64() % nodes.size());
			files.push_back(path + "/" + name);

			// create file
			results.AddBuild(device->MkFile(path + "/" + name, 0));

			++results.build_files;
		}

		progress = 100 * (results.build_files + results.build_dirs + results.destroyed) / (4 * FILESTRUCTURE_SIZE);

		if(go == false)
			return;
	}

	// sleep 1 sec
	sleep(1);

	// del files
	for(int i = 0; i < files.size(); ++i)
	{
		results.AddDestroy(device->DelFile(files[i]));
		++results.destroyed;

		progress = 100 * (results.build_files + results.build_dirs + results.destroyed) / (4 * FILESTRUCTURE_SIZE);

		if(go == false)
			return;
	}

	// del dirs
	for(int i = nodes.size() - 1; i > 0 ; --i)
	{
		results.AddDestroy(device->DelDir(nodes[i]));
		++results.destroyed;

		progress = 100 * (results.build_files + results.build_dirs + results.destroyed) / (4 * FILESTRUCTURE_SIZE);

		if(go == false)
			return;
	}
}

void FileStructure::InitScene()
{
	results.erase();
}


void FileStructure::UpdateScene()
{
	// update progress
//	SetProgress(progress);

	// rescale
	Rescale((qreal)results.max / 1000000, true);

	// update bars
	build_bar->Set(
			(100 * (results.build_dirs + results.build_files)) / (2 * FILESTRUCTURE_SIZE),
			(qreal)results.build / 1000000);

	destroy_bar->Set(
			100 * results.destroyed / (2 * FILESTRUCTURE_SIZE),
			(qreal)results.destroy / 1000000) ;
	/*
	build_bar->SetProgress((100 * (results.build_dirs + results.build_files)) / (2 * FILESTRUCTURE_SIZE));
	destroy_bar->SetProgress(100 * results.destroyed / (2 * FILESTRUCTURE_SIZE));

	build_bar->SetValue((qreal)results.build / 1000000);
	destroy_bar->SetValue((qreal)results.destroy / 1000000);
	*/
}

qreal FileStructure::GetProgress()
{
	return progress;
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

	destroy = 0.0;
	build = 0.0;
	max = 0.0;
}

void FileStructureResults::AddBuild(hddtime time)
{
	build += time;
	max = std::max(build, destroy);
}

void FileStructureResults::AddDestroy(hddtime time)
{
	destroy += time;
	max = std::max(build, destroy);
}

QDomElement FileStructure::WriteResults(QDomDocument &doc)
{
	// create main seek element
	QDomElement master = doc.createElement("File_Structure");
	master.setAttribute("valid", (100 == progress)?"yes":"no");
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

void FileStructure::RestoreResults(QDomElement &results, bool reference)
{
	// Locate main seek element
	QDomElement main = results.firstChildElement("File_Structure");
	if(!main.attribute("valid", "no").compare("no"))
		return;

	// init scene and remove results
	InitScene();

	//// get Build
	QDomElement build = main.firstChildElement("Build");
	if(build.isNull())
		return;
	this->results.AddBuild(build.attribute("time", "0").toDouble());

	//// get Destroy
	QDomElement destroy = main.firstChildElement("Destroy");
	if(destroy.isNull())
		return;
	this->results.AddDestroy(destroy.attribute("time", "0").toDouble());

	// set progress and update scene
	progress = 100;
	this->results.build_dirs = FILESTRUCTURE_SIZE;
	this->results.build_files = FILESTRUCTURE_SIZE;
	this->results.destroyed = FILESTRUCTURE_SIZE * 2;

	UpdateScene();
}


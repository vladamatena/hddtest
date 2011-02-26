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
	QList<QString> files;
	QList<QString> nodes;
	nodes.push_back(device->GetSafeTemp());	//	add initial node
	results.progress = 0;

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

		results.progress = 100 * (results.build_files + results.build_dirs + results.destroyed) / (4 * FILESTRUCTURE_SIZE);

		if(go == false)
			return;
	}

	// small delay for system to return normal
	sleep(1);

	// delete files
	for(int i = 0; i < files.size(); ++i)
	{
		results.AddDestroy(device->DelFile(files[i]));
		++results.destroyed;

		results.progress = 100 * (results.build_files + results.build_dirs + results.destroyed) / (4 * FILESTRUCTURE_SIZE);

		if(go == false)
			return;
	}

	// delete dirs
	for(int i = nodes.size() - 1; i > 0 ; --i)
	{
		results.AddDestroy(device->DelDir(nodes[i]));
		++results.destroyed;

		results.progress = 100 * (results.build_files + results.build_dirs + results.destroyed) / (4 * FILESTRUCTURE_SIZE);

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
	// update result bars
	build_bar->Set(
			(100 * (results.build_dirs + results.build_files)) / (2 * FILESTRUCTURE_SIZE),
			(qreal)results.build / 1000000);

	destroy_bar->Set(
			100 * results.destroyed / (2 * FILESTRUCTURE_SIZE),
			(qreal)results.destroy / 1000000) ;

	// update reference bars
	build_reference_bar->Set(
			(100 * (reference.build_dirs + reference.build_files)) / (2 * FILESTRUCTURE_SIZE),
			(qreal)reference.build / 1000000);

	destroy_reference_bar->Set(
			100 * reference.destroyed / (2 * FILESTRUCTURE_SIZE),
			(qreal)reference.destroy / 1000000) ;

	// rescale
	Rescale();
}

qreal FileStructure::GetProgress()
{
	return (qreal)(results.build_files + results.build_dirs + results.destroyed) / (qreal)(4 * FILESTRUCTURE_SIZE);
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
	max = 0.0f;
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
	master.setAttribute("valid", (100 == results.progress)?"yes":"no");
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
	FileStructureResults *res = reference?&this->reference:&this->results;

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
	res->AddBuild(build.attribute("time", "0").toDouble());

	//// get Destroy
	QDomElement destroy = main.firstChildElement("Destroy");
	if(destroy.isNull())
		return;
	res->AddDestroy(destroy.attribute("time", "0").toDouble());

	// set progress and update scene
	res->progress = 100;
	res->build_dirs = FILESTRUCTURE_SIZE;
	res->build_files = FILESTRUCTURE_SIZE;
	res->destroyed = FILESTRUCTURE_SIZE * 2;

	UpdateScene();
}

void FileStructure::EraseResults()
{
	results.erase();
}

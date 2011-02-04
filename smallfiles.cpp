/**
*
* \author Vladimír Matěna
*
* Contact vlada.matena@gmail.com
*
*/

#include "smallfiles.h"

SmallFiles::SmallFiles(QWidget *parent):
	TestWidget(parent)
{
	build_dir_bar = this->addBar("s", "Directory structure", QColor(255, 0 , 0), 0.1, 0.1);
	build_files_bar = this->addBar("s", "Files 1-10K", QColor(0, 0, 255), 0.3, 0.1);
	read_files_bar = this->addBar("s", "Read files", QColor(255, 0 , 255), 0.5, 0.1);
	destroy_bar = this->addBar("s", "Structure destroy", QColor(0, 255 , 0), 0.7, 0.1);
}

void SmallFiles::InitScene()
{
	results.erase();
}

void SmallFiles::TestLoop()
{
	// init random
	RandomGenerator random;

	// list of nodes in test
	QList<QString> files;
	QList<QString> nodes;
	nodes.push_back(device->GetSafeTemp());	//	add initial node

	// build dirs
	for(int i = 0; i < SMALLFILES_SIZE; ++i)
	{
		// construct new node name and path
		QString file = nodes.at(random.Get64() % nodes.size()) + "/" + QString::number(nodes.size());
		nodes.push_back(file);

		// create node
		hddtime time = device->MkDir(file);
		results.CheckMax(results.dir_build_time += time);

		++results.dirs_build;

		if(go == false)
			return;
	}

	// build files
	for(int i = 0; i < SMALLFILES_SIZE; ++i)
	{
		// construct new node name and path
		QString file = nodes.at(random.Get64() % nodes.size()) + "/" + QString::number(nodes.size());
		files.push_back(file);

		// create file
		hddtime time = device->MkFile(file, Device::K + random.Get64() % (9 * Device::K));
		results.CheckMax(results.file_build_time += time);

		++results.files_build;

		if(go == false)
			return;
	}

	// read files in random order
	QList<QString> files_to_read(files);
	while(!files_to_read.empty())
	{
		// generate random index
		int index = random.Get64() % files_to_read.size();
		hddtime time = device->ReadFile(files_to_read[index]);
		files_to_read.removeAt(index);
		results.CheckMax(results.file_read_time += time);

		++results.files_read;

		if(go == false)
			return;
	}

	// del files
	for(int i = 0; i < files.size(); ++i)
	{
		results.CheckMax(results.destroy_time += device->DelFile(files[i]));
		++results.destroyed;

		if(go == false)
			return;
	}

	// del dirs
	for(int i = nodes.size() - 1; i > 0 ; --i)
	{
		results.CheckMax(results.destroy_time += device->DelDir(nodes[i]));
		++results.destroyed;

		if(go == false)
			return;
	}
}

void SmallFiles::UpdateScene()
{
	// update progress
//	SetProgress(100 * (results.dirs_build + results.files_build + results.files_read + results.destroyed) / (5 * SMALLFILES_SIZE));

	// rescale
	Rescale((qreal)results.max / Device::s, true);

	// update bars
	this->build_dir_bar->Set(
			100 * results.dirs_build / SMALLFILES_SIZE,
			(qreal)results.dir_build_time / Device::s);
	this->build_files_bar->Set(
			100 * results.files_build / SMALLFILES_SIZE,
			(qreal)results.file_build_time / Device::s);
	this->read_files_bar->Set(
			100 * results.files_read / SMALLFILES_SIZE,
			(qreal)results.file_read_time / Device::s);
	this->destroy_bar->Set(
			100 * results.destroyed / (SMALLFILES_SIZE * 2),
			(qreal)results.destroy_time / Device::s);
}

qreal SmallFiles::GetProgress()
{
	return progress;
}

SmallFilesResults::SmallFilesResults()
{
	erase();
}

void SmallFilesResults::erase()
{
	dir_build_time = 0;
	file_build_time = 0;
	file_read_time = 0;
	destroy_time = 0;

	max = 0;

	dirs_build = 0;
	files_build = 0;
	files_read = 0;
	destroyed = 0;
}

void SmallFilesResults::CheckMax(hddtime time)
{
	max = std::max(max, time);
}

QDomElement SmallFiles::WriteResults(QDomDocument &doc)
{
	// create main seek element
	QDomElement master = doc.createElement("Small_Files");
	master.setAttribute("valid", (1 ==((results.dirs_build + results.files_build + results.files_read + results.destroyed) / (5 * SMALLFILES_SIZE)))?"yes":"no");
	doc.appendChild(master);


	// add build dirs element
	QDomElement build_dirs = doc.createElement("Build_dirs");
	build_dirs.setAttribute("time", results.dir_build_time);
	master.appendChild(build_dirs);

	// add build files element
	QDomElement build_files = doc.createElement("Build_files");
	build_files.setAttribute("time", results.file_build_time);
	master.appendChild(build_files);

	// add read files element
	QDomElement read_files = doc.createElement("Read_files");
	read_files.setAttribute("time", results.file_read_time);
	master.appendChild(read_files);

	// add destroy element
	QDomElement destroy = doc.createElement("Destroy");
	destroy.setAttribute("time", results.destroy_time);
	master.appendChild(destroy);

	return master;
}

void SmallFiles::RestoreResults(QDomElement &results, bool reference)
{
	// Locate main seek element
	QDomElement main = results.firstChildElement("Small_Files");
	if(!main.attribute("valid", "no").compare("no"))
		return;

	// init scene and remove results
	InitScene();

	//// get dirs build
	QDomElement dir_build = main.firstChildElement("Build_dirs");
	if(dir_build.isNull())
		return;
	this->results.dir_build_time = dir_build.attribute("time", "0").toDouble();

	//// get file build
	QDomElement files_build = main.firstChildElement("Build_files");
	if(files_build.isNull())
		return;
	this->results.file_build_time = files_build.attribute("time", "0").toDouble();

	//// get read files
	QDomElement files_read = main.firstChildElement("Read_files");
	if(files_read.isNull())
		return;
	this->results.file_read_time = files_read.attribute("time", "0").toDouble();

	//// get destroy
	QDomElement destroy = main.firstChildElement("Destroy");
	if(destroy.isNull())
		return;
	this->results.destroy_time = destroy.attribute("time", "0").toDouble();

	// check max
	this->results.CheckMax(this->results.dir_build_time);
	this->results.CheckMax(this->results.file_build_time);
	this->results.CheckMax(this->results.file_read_time);
	this->results.CheckMax(this->results.destroy_time);

	// set progress and update scene
	this->results.dirs_build = SMALLFILES_SIZE;
	this->results.files_build = SMALLFILES_SIZE;
	this->results.files_read = SMALLFILES_SIZE;
	this->results.destroyed = 2 * SMALLFILES_SIZE;

	UpdateScene();
}

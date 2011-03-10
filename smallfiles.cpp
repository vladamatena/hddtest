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
	build_dir_bar = this->addBar(	"s", "Dirs",		QColor(255,	0,		0),	0.03, 0.1);
	build_files_bar = this->addBar(	"s", "Files 1-10K",	QColor(255,	64,		0),	0.27, 0.1);
	read_files_bar = this->addBar(	"s", "Read files",	QColor(255,	128,	0),	0.51, 0.1);
	destroy_bar = this->addBar(		"s", "Delete",		QColor(255,	192,	0),	0.75, 0.1);

	build_dir_reference_bar = this->addBar(		"s", "Dirs",		QColor(0,	0,		255),	0.14, 0.1);
	build_files_reference_bar = this->addBar(	"s", "Files 1-10K",	QColor(0,	64,		255),	0.38, 0.1);
	read_files_reference_bar = this->addBar(	"s", "Read files",	QColor(0,	128,	255),	0.62, 0.1);
	destroy_reference_bar = this->addBar(		"s", "Delete",		QColor(0,	192,	255),	0.86, 0.1);

	testName = "Small files";
	testDescription = "This test creates random directory struture containing " + QString::number(SMALLFILES_SIZE) +
			" then " + QString::number(SMALLFILES_SIZE) +
			" files in size 1K to 10K are randomly distributed across this strucure." +
			" after this files are read again in random order. Finally whole structure is deleted including files." +
			" Every operation has it`s own bar that shows operation time." +
			" This test is only aviable for devices containing mounted filesystem.";
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
	results.phase = SmallFilesResults::PHASE_DIR_BUILD;
	device->DropCaches();
	device->Sync();
	for(int i = 0; (i < SMALLFILES_SIZE) && (testState != STOPPING); ++i)
	{
		// construct new node name and path
		QString file = nodes.at(random.Get64() % nodes.size()) + "/" + QString::number(nodes.size());
		nodes.push_back(file);

		// create node
		hddtime time = device->MkDir(file);
		results.dir_build_time += time;

		++results.dirs_build;
	}
	results.dir_build_time += device->Sync();

	// build files
	results.phase = SmallFilesResults::PHASE_FILE_BUILD;
	device->DropCaches();
	for(int i = 0; (i < SMALLFILES_SIZE) && (testState != STOPPING); ++i)
	{
		// construct new node name and path
		QString file = nodes.at(random.Get64() % nodes.size()) + "/" + QString::number(nodes.size());
		files.push_back(file);

		// create file
		hddtime time = device->MkFile(file, K + random.Get64() % (9 * K));
		results.file_build_time += time;

		++results.files_build;
	}
	results.file_build_time += device->Sync();

	// read files in random order
	results.phase = SmallFilesResults::PHASE_FILE_READ;
	device->DropCaches();
	QList<QString> files_to_read(files);
	while(!files_to_read.empty() && (testState != STOPPING))
	{
		// generate random index
		int index = random.Get64() % files_to_read.size();
		hddtime time = device->ReadFile(files_to_read[index]);
		files_to_read.removeAt(index);
		results.file_read_time += time;

		++results.files_read;
	}
	results.file_read_time += device->Sync();

	// make sure caches are empty
	device->DropCaches();

	// del files
	results.phase = SmallFilesResults::PHASE_DESTROY;
	for(int i = 0; i < files.size(); ++i)
	{
		results.destroy_time += device->DelFile(files[i]);
		++results.destroyed;
	}
	results.destroy_time += device->Sync();

	// del dirs
	for(int i = nodes.size() - 1; i > 0 ; --i)
	{
		results.destroy_time += device->DelDir(nodes[i]);
		++results.destroyed;
	}
	results.destroy_time += device->Sync();

	results.done = true;
}

void SmallFiles::UpdateScene()
{
	// get progress
	hddtime dir_build = results.dir_build_time;
	hddtime file_build = results.file_build_time;
	hddtime file_read =  results.file_read_time;
	hddtime destroy = results.destroy_time;

	// add current operation progress
	if(device)
	{
		switch(results.phase)
		{
		case SmallFilesResults::PHASE_DIR_BUILD:
			dir_build += device->timer.GetCurrentOffset();
			break;
		case SmallFilesResults::PHASE_FILE_BUILD:
			file_build += device->timer.GetCurrentOffset();
			break;
		case SmallFilesResults::PHASE_FILE_READ:
			file_read += device->timer.GetCurrentOffset();
			break;
		case SmallFilesResults::PHASE_DESTROY:
			destroy += device->timer.GetCurrentOffset();
		default:
			break;
		}
	}

	// update bars
	build_dir_bar->Set(
			100 * results.dirs_build / SMALLFILES_SIZE,
			(qreal)dir_build / s);
	build_files_bar->Set(
			100 * results.files_build / SMALLFILES_SIZE,
			(qreal)file_build / s);
	read_files_bar->Set(
			100 * results.files_read / SMALLFILES_SIZE,
			(qreal)file_read / s);
	destroy_bar->Set(
			100 * results.destroyed / (SMALLFILES_SIZE * 2),
			(qreal)(destroy) / s);

	// update reference bars
	build_dir_reference_bar->Set(
			100 * reference.dirs_build / SMALLFILES_SIZE,
			(qreal)reference.dir_build_time / s);
	build_files_reference_bar->Set(
			100 * reference.files_build / SMALLFILES_SIZE,
			(qreal)reference.file_build_time / s);
	read_files_reference_bar->Set(
			100 * reference.files_read / SMALLFILES_SIZE,
			(qreal)reference.file_read_time / s);
	destroy_reference_bar->Set(
			100 * reference.destroyed / (SMALLFILES_SIZE * 2),
			(qreal)reference.destroy_time / s);

	// rescale
	Rescale();
}

int SmallFiles::GetProgress()
{
	// "+ results.done" and  "+ 1" forces 99% until filan sync is done
	int state = results.dirs_build + results.files_build + results.files_read + results.destroyed + results.done;
	int target = 5 * SMALLFILES_SIZE + 1;
	return (100 * state) / target;
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

	dirs_build = 0;
	files_build = 0;
	files_read = 0;
	destroyed = 0;

	done = false;
	phase = PHASE_NONE;
}

QDomElement SmallFiles::WriteResults(QDomDocument &doc)
{
	// create main seek element
	QDomElement master = doc.createElement("Small_Files");
	master.setAttribute("valid", (this->GetProgress() == 100)?"yes":"no");
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

void SmallFiles::RestoreResults(QDomElement &results, DataSet dataset)
{
	SmallFilesResults &res = (dataset == REFERENCE)?this->reference:this->results;

	// Locate main seek element
	QDomElement main = results.firstChildElement("Small_Files");
	if(!main.attribute("valid", "no").compare("no"))
		return;

	// init scene and remove results
	res.erase();

	//// get dirs build
	QDomElement dir_build = main.firstChildElement("Build_dirs");
	if(dir_build.isNull())
		return;
	res.dir_build_time = dir_build.attribute("time", "0").toDouble();

	//// get file build
	QDomElement files_build = main.firstChildElement("Build_files");
	if(files_build.isNull())
		return;
	res.file_build_time = files_build.attribute("time", "0").toDouble();

	//// get read files
	QDomElement files_read = main.firstChildElement("Read_files");
	if(files_read.isNull())
		return;
	res.file_read_time = files_read.attribute("time", "0").toDouble();

	//// get destroy
	QDomElement destroy = main.firstChildElement("Destroy");
	if(destroy.isNull())
		return;
	res.destroy_time = destroy.attribute("time", "0").toDouble();

	// set progress and update scene
	res.dirs_build = SMALLFILES_SIZE;
	res.files_build = SMALLFILES_SIZE;
	res.files_read = SMALLFILES_SIZE;
	res.destroyed = 2 * SMALLFILES_SIZE;

	UpdateScene();
}

void SmallFiles::EraseResults(DataSet dataset)
{
	if(dataset == RESULTS)
		results.erase();
	else
		reference.erase();

	UpdateScene();
}

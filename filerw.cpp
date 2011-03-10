/**
*
* \author Vladimír Matěna
*
* Contact vlada.matena@gmail.com
*
*/

#include "filerw.h"

FileRW::FileRW(QWidget *parent) :
	TestWidget(parent)
{
	// Add two line graph components (reading + writting)
	__read_graph = addLineGraph("MB/s", QColor(255, 192, 192));
	__write_graph = addLineGraph("MB/s", QColor(255, 0, 0));

	// Add two line graph components (reference reading + reference writting)
	__read_reference_graph = addLineGraph("MB/s", QColor(192, 192, 255));
	__write_reference_graph = addLineGraph("MB/s", QColor(0, 0, 255));

	// Add background net
	__net = addNet("MB/s", "File position", "Speed");

	// Add legend
	__legend = addLegend();
	__legend->AddItem("Read", QColor(255, 192, 192));
	__legend->AddItem("Write", QColor(255, 0, 0));
	__legend->AddItem("Read", QColor(192, 192, 255));
	__legend->AddItem("Write", QColor(0, 0, 255));

	testName = "File write and read";
	testDescription = "R/W File test writes " + Def::FormatSize(FILERW_SIZE) +
			" to file on mounted device. Then whole file is read again." +
			" Both reading and writting are performed in blocks of " +
			Def::FormatSize(FILERW_BLOCK) + "." +
			" Process is shown in graph where darker color shows write speed" +
			" and lighter read speed depending on file position." +
			" This test is not aviable(grayed start button) when device is not mounted.";
}

FileRW::~FileRW()
{
}

void FileRW::TestLoop()
{
	// erase old results
	results_write.erase();
	results_read.erase();

	// prepare test file
	QString filename = device->GetSafeTemp() + "/" + "hddtestfile";

	File file(filename);

	// get block count	
	results_write.blocks = FILERW_SIZE / FILERW_BLOCK;
	results_read.blocks = FILERW_SIZE / FILERW_BLOCK;
	results_write.blocks_done = 0;
	results_read.blocks_done = 0;

	// write blocks until enough data is written
	file.SetPos(0);
	for(results_write.blocks_done = 1; results_write.blocks_done <= results_write.blocks; ++results_write.blocks_done)
	{
		hddtime time = file.Write(FILERW_BLOCK);
		results_write.AddResult((qreal)FILERW_BLOCK / time);
		if(testState == STOPPING)
			break;
	}

	// sync filesystem and drop caches
	file.Reopen();
	device->Sync();
	device->DropCaches();

	// read block until enough data is read
	file.SetPos(0);
	for(results_read.blocks_done = 1; results_read.blocks_done <= results_read.blocks; ++results_read.blocks_done)
	{
		hddtime time = file.Read(FILERW_BLOCK);
		results_read.AddResult((qreal)FILERW_BLOCK / time);
		if(testState == STOPPING)
			break;
	}	

	// close and delete file
	file.Close();
	device->DelFile(filename);
}

void FileRW::InitScene()
{
	// reset first value
	__first = true;

	// erase graphs and results
	__read_graph->erase();
	__write_graph->erase();
	results_read.erase();
	results_write.erase();
}

void FileRW::UpdateScene()
{
	// set graph size
	__write_graph->SetSize(results_write.blocks);
	__read_graph->SetSize(results_read.blocks);
	__write_reference_graph->SetSize(reference_write.blocks);
	__read_reference_graph->SetSize(reference_read.blocks);

	// add new values to write graph
	while(!results_write.new_results.empty())
	{
		qreal data = results_write.new_results.dequeue();
		__write_graph->AddValue(data);
	}

	// add new values to read graph
	while(!results_read.new_results.empty())
	{
		qreal data = results_read.new_results.dequeue();
		__read_graph->AddValue(data);
	}

	// add new values to reference write graph
	while(!reference_write.new_results.empty())
	{
		qreal data = reference_write.new_results.dequeue();
		__write_reference_graph->AddValue(data);
	}

	// add new values to reference read graph
	while(!reference_read.new_results.empty())
	{
		qreal data = reference_read.new_results.dequeue();
		__read_reference_graph->AddValue(data);
	}

	// rescale graph
	Rescale();
}

int FileRW::GetProgress()
{
	int done = results_read.blocks_done + results_write.blocks_done;
	int blocks = results_write.blocks + results_read.blocks;
	if(blocks)
		return (100 * done) / blocks;
	else
		return 0;
}

FileRWResults::FileRWResults()
{
	avg = 0;

	// zero block count
	blocks = 0;
	blocks_done = 0;
}

void FileRWResults::AddResult(qreal result)
{
	results.push_back(result);		// add to results
	new_results.enqueue(result);	// add to results to draw

	// calc sum
	qreal sum = 0;
	for(int i = 0; i < results.size(); ++i)
		sum += results[i];
	avg = sum / results.size();
}

void FileRWResults::erase()
{
	// celar results
	results.clear();
	new_results.clear();

	// reset statistics
	avg = 0;

	// zero block count
	blocks = 0;
	blocks_done = 0;
}


QDomElement FileRW::WriteResults(QDomDocument &doc)
{
	// create main seek element
	QDomElement master = doc.createElement("File_Read_Write");
	master.setAttribute("valid", (GetProgress() == 100)?"yes":"no");
	doc.appendChild(master);

	//// add write element
	QDomElement write = doc.createElement("Write_data");
	master.appendChild(write);

	// add values to write element
	if(GetProgress() == 100) for(int i = 0; i < results_write.results.size(); ++i)
	{
		QDomElement value = doc.createElement("Write");
		value.setAttribute("speed", results_write.results[i]);
		write.appendChild(value);
	}

	// add read element
	QDomElement read = doc.createElement("Read_data");
	master.appendChild(read);

	// add values to read element
	if(GetProgress() == 100) for(int i = 0; i < results_read.results.size(); ++i)
	{
		QDomElement value = doc.createElement("Read");
		value.setAttribute("speed", results_read.results[i]);
		read.appendChild(value);
	}

	return master;
}

void FileRW::RestoreResults(QDomElement &results, DataSet dataset)
{
	FileRWResults *res_write = (dataset == REFERENCE)?&reference_write:&results_write;
	FileRWResults *res_read = (dataset == REFERENCE)?&reference_read:&results_read;

	// Locate main fileRW element
	QDomElement main = results.firstChildElement("File_Read_Write");
	if(!main.attribute("valid", "no").compare("no"))
		return;

	// erase old results
	res_write->erase();
	res_read->erase();
	(dataset == REFERENCE)?__write_reference_graph->erase():__write_graph->erase();
	(dataset == REFERENCE)?__read_reference_graph->erase():__read_graph->erase();

	//// get Write
	QDomElement write = main.firstChildElement("Write_data");
	if(write.isNull())
		return;
	// get list of writes
	QDomNodeList writes = write.elementsByTagName("Write");
	res_write->blocks = writes.size();
	// read write result data
	for(int i = 0; i < writes.size(); ++i)
		res_write->AddResult(writes.at(i).toElement().attribute("speed", "0").toDouble());

	// get Read
	QDomElement read = main.firstChildElement("Read_data");
	if(write.isNull())
		return;
	// get list of reads
	QDomNodeList reads = read.elementsByTagName("Read");
	res_read->blocks = reads.size();
	// read result data
	for(int i = 0; i < reads.size(); ++i)
		res_read->AddResult(reads.item(i).toElement().attribute("speed", "0").toDouble());

	// set progress and update scene
	res_write->blocks_done = res_write->blocks;
	res_read->blocks_done = res_read->blocks;
	UpdateScene();
}

void FileRW::EraseResults(DataSet dataset)
{
	// erase data
	if(dataset == RESULTS)
	{
		results_read.erase();
		results_write.erase();

		__write_graph->erase();
		__read_graph->erase();
	}
	else
	{
		reference_read.erase();
		reference_write.erase();

		__write_reference_graph->erase();
		__read_reference_graph->erase();
	}

	// resfresh view
	UpdateScene();
}

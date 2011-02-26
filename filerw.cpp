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
	testDescription = "R/W File test writes " + Device::Format(FILERW_SIZE) +
			" to file on mounted device. Then whole file is read again." +
			" Process is shown in graph where darker color shows write speed" +
			"and lighter read speed depending on file position." +
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
	QString temp = device->GetSafeTemp();
	Device file_write;
	file_write.Open(temp + "/file.1G", false, true);

	// get block count	
	results_write.blocks = FILERW_SIZE / FILERW_BLOCK;
	results_read.blocks = FILERW_SIZE / FILERW_BLOCK;
	results_write.blocks_done = 0;
	results_read.blocks_done = 0;

	// write blocks until enough data is written
	file_write.SetPos(0);
	for(results_write.blocks_done = 1; results_write.blocks_done <= results_write.blocks; ++results_write.blocks_done)
	{
		hddtime time = file_write.Write(FILERW_BLOCK);
		results_write.AddResult((qreal)FILERW_BLOCK / time);
		if(go == false)
			break;
	}

	Device file_read;
	file_read.Open(temp + "/file.1G", false);

	// read block until enough data is read
	file_read.SetPos(0);
	for(results_read.blocks_done = 1; results_read.blocks_done <= results_read.blocks; ++results_read.blocks_done)
	{
		hddtime time = file_read.Read(FILERW_BLOCK);
		results_read.AddResult((qreal)FILERW_BLOCK / time);
		if(go == false)
			break;
	}
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
	int nrws = results_write.new_results.size();
	for(int i = 0; i < nrws; ++i)
	{
		qreal data = results_write.new_results.pop();
		__write_graph->AddValue(data);
	}

	// add new values to read graph
	int nrrs = results_read.new_results.size();
	for(int i = 0; i < nrrs; ++i)
	{
		qreal data = results_read.new_results.pop();
		__read_graph->AddValue(data);
	}

	// add new values to reference write graph
	int nrwsr = reference_write.new_results.size();
	for(int i = 0; i < nrwsr; ++i)
	{
		qreal data = reference_write.new_results.pop();
		__write_reference_graph->AddValue(data);
	}

	// add new values to reference read graph
	int nrrsr = reference_read.new_results.size();
	for(int i = 0; i < nrrsr; ++i)
	{
		qreal data = reference_read.new_results.pop();
		__read_reference_graph->AddValue(data);
	}

	// rescale graph
	Rescale();
}

qreal FileRW::GetProgress()
{
	if((results_read.blocks_done == 0) && (results_write.blocks_done == 0))
		return 0.0f;
	qreal done = results_read.blocks_done + results_write.blocks_done;
	qreal blocks = results_write.blocks + results_read.blocks;
	return done / blocks;
}

FileRWResults::FileRWResults()
{
	max = 0;
	avg = 0;

	// zero block count
	blocks = 0;
	blocks_done = 0;
}

void FileRWResults::AddResult(qreal result)
{
	results.push_back(result);		// add to results
	new_results.push_back(result);	// add to results to draw

	// update max
	if(result > max || max == 0)
		max = result;

	// calc sum
	qreal sum = 0;
	for(int i = 0; i < results.size(); ++i)
		sum += results[i];
	avg = sum / results.size();
}

void FileRWResults::erase()
{
	results.erase(results.begin(), results.end());						// erase results
	this->new_results.erase(new_results.begin(), new_results.end());	// clear results to draw

	// reset statistics
	max = 0;
	avg = 0;

	// zero block count
	blocks = 0;
	blocks_done = 0;
}


QDomElement FileRW::WriteResults(QDomDocument &doc)
{
	// create main seek element
	QDomElement master = doc.createElement("File_Read_Write");
	master.setAttribute("valid", (GetProgress()==100)?"yes":"no");
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

void FileRW::RestoreResults(QDomElement &results, bool reference)
{
	FileRWResults *res_write = reference?&reference_write:&results_write;
	FileRWResults *res_read = reference?&reference_read:&results_read;

	// Locate main fileRW element
	QDomElement main = results.firstChildElement("File_Read_Write");
	if(!main.attribute("valid", "no").compare("no"))
		return;

	// erase old results
	res_write->erase();
	res_read->erase();
	reference?__write_reference_graph->erase():__write_graph->erase();
	reference?__read_reference_graph->erase():__read_graph->erase();

	//// get Write
	QDomElement write = main.firstChildElement("Write_data");
	if(write.isNull())
		return;
	// get list of writes
	QDomNodeList writes = write.elementsByTagName("Write");
	res_write->blocks = writes.size();
	// read write result data
	for(int i = writes.size() - 1; i >= 0; --i)
		res_write->AddResult(writes.at(i).toElement().attribute("speed", "0").toDouble());

	// get Read
	QDomElement read = main.firstChildElement("Read_data");
	if(write.isNull())
		return;
	// get list of reads
	QDomNodeList reads = read.elementsByTagName("Read");
	res_read->blocks = reads.size();
	// read result data
	for(int i = reads.size() - 1; i >= 0; --i)
		res_read->AddResult(reads.at(i).toElement().attribute("speed", "0").toDouble());

	// set progress and update scene
	res_write->blocks_done = res_write->blocks;
	res_read->blocks_done = res_read->blocks;
	UpdateScene();
}

void FileRW::EraseResults()
{
	// erase test results
	results_read.erase();
	results_write.erase();

	// erase line graphs
	__write_graph->erase();
	__read_graph->erase();

	// resfresh view
	UpdateScene();
}

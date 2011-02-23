/**
*
* \author Vladimír Matěna
*
* Contact vlada.matena@gmail.com
*
*/

#include "readcont.h"

ReadCont::ReadCont(QWidget *parent):
	TestWidget(parent)
{
	// add line components to graph
	averageLine = addLine("MB/s", "", QColor(255, 0, 0));
	refAverageLine = addLine("MB/s", "", QColor(0, 0, 255));

	// add line graph component to graph
	graph = addLineGraph("MB/s", QColor(255, 128, 128));
	refGraph = addLineGraph("MB/s", QColor(128, 128, 255));

	// add background net
	net = addNet("MB/s");
}

ReadCont::~ReadCont()
{
}

void ReadCont::TestLoop()
{
	// erase old results
	results.erase();

	// get test size
	hddsize bytes_to_read = READ_CONT_SIZE;
	if(bytes_to_read > device->GetSize())
		bytes_to_read = device->GetSize();

	results.progress = 0;

	// get block count
	results.blocks = bytes_to_read / READ_CONT_BLOCK;

	// read block until enough data is read
	for(results.blocks_done = 0; results.blocks_done < results.blocks; ++results.blocks_done)
	{
		hddtime time = device->Read(READ_CONT_BLOCK);
		results.AddResult((qreal)READ_CONT_BLOCK / time);
		results.progress = 100 * results.blocks_done / (results.blocks - 1);

		if(go == false)
			break;
	}
}

void ReadCont::InitScene()
{
	graph->erase();
	results.erase();
}

void ReadCont::UpdateScene()
{
	// set line graph line count
	graph->SetSize(results.blocks);
	refGraph->SetSize(reference.blocks);

	// add all new values to line graph
	int resSize = results.new_results.size();
	for(int i = 0; i < resSize; ++i)
	{
		qreal data = results.new_results.pop();
		graph->AddValue(data);
	}

	// add all new values to reference line graph
	int refSize = reference.new_results.size();
	for(int i = 0; i < refSize; ++i)
	{
		qreal data = reference.new_results.pop();
		refGraph->AddValue(data);
	}


	// update horizontal lines
	averageLine->SetValue(results.avg);
	refAverageLine->SetValue(reference.avg);

	// rescale scene to reflect possible new max
	Rescale();
}

qreal ReadCont::GetProgress()
{
	return results.progress / 100.0f;
}

ReadContResults::ReadContResults()
{
	min = 0;
	max = 0;
	avg = 0;
}

void ReadContResults::AddResult(qreal result)
{
	results.push_back(result);		// add result
	new_results.push_back(result);	// add results to new results (not yet drawn)

	// update min / max
	if(result > max || max == 0)
		max = result;
	if(result < min || min == 0)
		min = result;

	// update sum
	qreal sum = 0;
	for(int i = 0; i < results.size(); ++i)
		sum += results[i];
	avg = sum / results.size();
}

void ReadContResults::erase()
{
	results.erase(results.begin(), results.end());
	min = 0;
	max = 0;
	avg = 0;
}

QDomElement ReadCont::WriteResults(QDomDocument &doc)
{
	// create main seek element
	QDomElement master = doc.createElement("Read_Continuous");
	master.setAttribute("valid", (results.progress == 100)?"yes":"no");
	doc.appendChild(master);

	// write subresults
	for(int i = 0; i < results.results.size(); ++i)
	{
		// add speed element
		QDomElement speed = doc.createElement("Speed");
		speed.setAttribute("value", results.results[i]);
		master.appendChild(speed);
	}

	return master;
}

void ReadCont::RestoreResults(QDomElement &root, bool reference)
{
	ReadContResults &results = reference?this->reference:this->results;

	// Locate main readcont element
	QDomElement main = root.firstChildElement("Read_Continuous");
	if(!main.attribute("valid", "no").compare("no"))
		return;

	// init scene and remove results
	reference?refGraph->erase():graph->erase();
	results.erase();

	// get list of read continuous values
	QDomNodeList res = main.elementsByTagName("Speed");
	results.blocks = res.size();

	// read result data
	for(int i = 0; i < res.size(); ++i)
		results.AddResult(res.at(i).toElement().attribute("value", "0").toDouble());

	// set progress
	results.progress = 100;

	// refresh view
	UpdateScene();
}

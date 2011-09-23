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
	net = addNet("MB/s", "Device position", "Read speed");

	testName = "Read Continuous";
	testDescription = "Read Continuous test reads " + Def::FormatSize(READ_CONT_SIZE) + " from device." +
			" Read operation is divided into blocks of " + Def::FormatSize(READ_CONT_BLOCK) + " in order to draw graph." +
			" Horizontal axis is device position and vertical is read speed.";
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

	// get block count
	results.blocks = bytes_to_read / READ_CONT_BLOCK;

	// read block until enough data is read
	for(results.blocks_done = 1; results.blocks_done <= results.blocks; ++results.blocks_done)
	{
		hddtime time = device->Read(READ_CONT_BLOCK);
		results.AddResult((qreal)READ_CONT_BLOCK / time);

		if(testState == STOPPING)
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
	while(!results.new_results.empty())
	{
		qreal data = results.new_results.dequeue();
		graph->AddValue(data);
	}

	// add all new values to reference line graph
	while(!reference.new_results.empty())
	{
		qreal data = reference.new_results.dequeue();
		refGraph->AddValue(data);
	}


	// update horizontal lines
	averageLine->SetValue(results.avg);
	refAverageLine->SetValue(reference.avg);

	// rescale scene to reflect possible new max
	Rescale();
}

int ReadCont::GetProgress()
{
	if(results.blocks == 0)
		return 0;

	return 100 * results.blocks_done / results.blocks;
}

ReadContResults::ReadContResults()
{
	erase();
}

void ReadContResults::AddResult(qreal result)
{
	results.push_back(result);		// add result
	new_results.enqueue(result);	// add results to new results (not yet drawn)

	// update average
	qreal sum = 0;
	for(int i = 0; i < results.size(); ++i)
		sum += results[i];
	avg = sum / results.size();
}

void ReadContResults::erase()
{
	this->blocks = 0;
	this->blocks_done = 0;
	results.clear();
	avg = 0;
}

QDomElement ReadCont::WriteResults(QDomDocument &doc)
{
	// create main seek element
	QDomElement master = doc.createElement("Read_Continuous");
	master.setAttribute("valid", (GetProgress() == 100)?"yes":"no");
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

void ReadCont::RestoreResults(QDomElement &root, DataSet dataset)
{
	ReadContResults &results = (dataset == REFERENCE)?this->reference:this->results;

	// Locate main readcont element
	QDomElement main = root.firstChildElement("Read_Continuous");
	if(!main.attribute("valid", "no").compare("no"))
		return;

	// init scene and remove results
	(dataset == REFERENCE)?refGraph->erase():graph->erase();
	results.erase();

	// get list of read continuous values
	QDomNodeList res = main.elementsByTagName("Speed");
	results.blocks = res.size();

	// read result data
	for(int i = 0; i < res.size(); ++i)
		results.AddResult(res.at(i).toElement().attribute("value", "0").toDouble());

	// set progress
	results.blocks_done = results.blocks = res.size();

	// refresh view
	UpdateScene();
}

void ReadCont::EraseResults(DataSet dataset)
{
	// erase data
	if(dataset == RESULTS)
	{
		results.erase();
		graph->erase();
	}
	else
	{
		reference.erase();
		refGraph->erase();
	}

	// refresh view
	UpdateScene();
}

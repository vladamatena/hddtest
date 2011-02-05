/**
*
* \author Vladimír Matěna
*
* Contact vlada.matena@gmail.com
*
*/

#include "readblock.h"

ReadBlock::ReadBlock(QWidget *parent):
	TestWidget(parent)
{
	// add subtests to subtest list
	int base = 1048576;
	results.push_back(ReadBlockResult(base));
	results.push_back(ReadBlockResult(base /= 2));
	results.push_back(ReadBlockResult(base /= 2));
	results.push_back(ReadBlockResult(base /= 2));
	results.push_back(ReadBlockResult(base /= 2));
	results.push_back(ReadBlockResult(base /= 2));
	results.push_back(ReadBlockResult(base /= 2));
	results.push_back(ReadBlockResult(base /= 2));
	results.push_back(ReadBlockResult(base /= 2));
	results.push_back(ReadBlockResult(base /= 2));
	results.push_back(ReadBlockResult(base /= 2));
	results.push_back(ReadBlockResult(base /= 2));

	progress = 0;

	// add bars to scene
	for(int i = 0; i < results.size(); ++i)
	{
		Bar *bar = this->addBar(
				"MB/s",
				Device::Format(results[i].__block_size),
				QColor(0xff, 0xa0 * (i+1) / results.size(), 0),
				i * 1.0f / results.size(),
				1.0f / (results.size() + 1));

		this->bars.push_back(bar);
	}
}

void ReadBlock::TestLoop()
{
	// erase prevoius results
	for(int i = 0;i < results.size(); ++i)
		results[i].erase();

	// run subtests
	for(int i = 0; i < results.size(); ++i)
	{
		ReadBlockResult *result = &results[i];

		// run subtest
		while(result->__bytes_read < READ_BLOCK_SIZE)
		{
			result->__time_elapsed += device->Read(result->__block_size);
			result->__bytes_read += result->__block_size;

			qreal speed = (qreal)result->__bytes_read / result->__time_elapsed;
			if(result->max < speed)
				result->max = speed;

			if(go == false)
				return;
		}

		// update progress
		progress = ((i + 1) * 100) / results.size();

		if(go == false)
			return;
	}
}


void ReadBlock::InitScene()
{

}

void ReadBlock::UpdateScene()
{
	// update progress
//	SetProgress(progress);

	// update subtest results
	for(int i = 0; i < results.size(); ++i)
	{
		//// update subresult
		const ReadBlockResult &result = results.at(i);

		// calculate MB/s
		qreal MBps = (qreal)result.__bytes_read / (qreal)result.__time_elapsed;

		// get global max
		qreal max = 0;
		for(int j = 0; j < results.size(); ++j)
		{
			qreal speed = (qreal)results[j].__bytes_read / results[j].__time_elapsed;
			if(max < speed)
				max = speed;
		}

		// update scene
		Rescale(max, true);
		bars[i]->Set(
				(qreal)(100 * result.__bytes_read) / READ_BLOCK_SIZE,
				MBps);
	}
}


qreal ReadBlock::GetProgress()
{
	return progress;
}

void ReadBlockResult::erase()
{
	// reset bytes read and time elapsed
	this->__bytes_read = 0;
	this->__time_elapsed = 0;
}

QDomElement ReadBlock::WriteResults(QDomDocument &doc)
{
	// create main seek element
	QDomElement master = doc.createElement("Read_Block");
	master.setAttribute("valid", (progress == 100)?"yes":"no");
	doc.appendChild(master);

	// write subresults
	for(int i = 0; i < results.size(); ++i)
	{
		// add build element
		QDomElement build = doc.createElement("Result");
		build.setAttribute("size", results[i].__block_size);
		build.setAttribute("time", results[i].__time_elapsed);
		master.appendChild(build);
	}

	return master;
}

void ReadBlock::RestoreResults(QDomElement &results, bool reference)
{
	// Locate main readblock element
	QDomElement seek = results.firstChildElement("Read_Block");
	if(!seek.attribute("valid", "no").compare("no"))
		return;

	// init scene and remove results
	InitScene();

	// get list of readblock subresults
	QDomNodeList xmlresults = seek.elementsByTagName("Result");

	// read subresults
	for(int i = 0; i < this->results.size(); ++i)
	{
		this->results[i].__block_size = xmlresults.at(i).toElement().attribute("size").toLongLong();
		this->results[i].__time_elapsed = xmlresults.at(i).toElement().attribute("time").toLongLong();
		this->results[i].__bytes_read = READ_BLOCK_SIZE;
	}

	// set progress
	progress = 100;

	// refresh view
	UpdateScene();
}

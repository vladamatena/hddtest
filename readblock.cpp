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
	for(int i = 0; i < 12; ++i)
	{
		results.push_back(ReadBlockResult(base));
		reference.push_back(ReadBlockResult(base));
		base /= 2;
	}

	// add bars to scene
	for(int i = 0; i < results.size(); ++i)
	{
		Bar *bar = this->addBar(
				"MB/s",
				Device::Format(results[i].__block_size),
				QColor(0xff, 0xa0 * (i+1) / results.size(), 0),
				2*i * 1.0f / (results.size() + reference.size()),
				1.0f / (results.size() + reference.size() + 1));
		bars.push_back(bar);
	}

	// add reference bars to scene
	for(int i = 0; i < reference.size(); ++i)
	{
		Bar *bar = this->addBar(
				"MB/s",
				Device::Format(reference[i].__block_size),
				QColor(0, 0xc0 * (i+1) / reference.size(), 0xff),
				(2*i + 1) * 1.0f / (reference.size() + reference.size()),
				1.0f / (reference.size() + reference.size() + 1));
		reference_bars.push_back(bar);
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

		if(go == false)
			return;
	}
}


void ReadBlock::InitScene()
{

}

void ReadBlock::UpdateScene()
{
	// update subtest results
	for(int i = 0; i < results.size(); ++i)
	{
		//// update subresult
		const ReadBlockResult &result = results.at(i);
		const ReadBlockResult &refer = reference.at(i);

		// get global max
		qreal max = 0;
		for(int j = 0; j < results.size(); ++j)
		{
			qreal speed = 0.0f;
			speed = (qreal)results[j].__bytes_read / results[j].__time_elapsed;
			if(max < speed)
				max = speed;
			speed = (qreal)reference[j].__bytes_read / reference[j].__time_elapsed;
			if(max < speed)
				max = speed;
		}

		// rescale and update grephics
		Rescale();
		bars[i]->Set(
				(qreal)(100 * result.__bytes_read) / READ_BLOCK_SIZE,
				(qreal)result.__bytes_read / (qreal)result.__time_elapsed);
		reference_bars[i]->Set(
				(qreal)(100 * refer.__bytes_read) / READ_BLOCK_SIZE,
				(qreal)refer.__bytes_read / (qreal)refer.__time_elapsed);
	}
}


qreal ReadBlock::GetProgress()
{
	qreal progress = 0.0f;

	for(unsigned int i = 0; i < results.size(); ++i)
		progress += (float)results[i].__bytes_read / READ_BLOCK_SIZE;
	return progress / results.size();
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
	master.setAttribute("valid", (GetProgress() == 100)?"yes":"no");
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
	QList<ReadBlockResult> *res = reference?&this->reference:&this->results;

	// Locate main readblock element
	QDomElement seek = results.firstChildElement("Read_Block");
	if(!seek.attribute("valid", "no").compare("no"))
		return;

	// get list of readblock subresults
	QDomNodeList xmlresults = seek.elementsByTagName("Result");

	// read subresults
	for(int i = 0; i < this->results.size(); ++i)
	{
		(*res)[i].__block_size = xmlresults.at(i).toElement().attribute("size").toLongLong();
		(*res)[i].__time_elapsed = xmlresults.at(i).toElement().attribute("time").toLongLong();
		(*res)[i].__bytes_read = READ_BLOCK_SIZE;
	}

	// refresh view
	UpdateScene();
}

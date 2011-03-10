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

	// test name and description
	testName = "Read block";
	testDescription = "Read Block test reads " + Def::FormatSize(READ_BLOCK_SIZE) +
			" with different block sizes. Blocks of specified size are place next to each other." +
			" No seekeing is required to access next block. Block sizes are: ";
	for(int i = 0; i < results.size(); ++i)
	{
		if(i > 0)
			testDescription += ", ";
		testDescription += Def::FormatSize(results[i].__block_size);
	}

	// add bars to scene
	for(int i = 0; i < results.size(); ++i)
	{
		Bar *bar = this->addBar(
				"MB/s",
				Def::FormatSize(results[i].__block_size),
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
				Def::FormatSize(reference[i].__block_size),
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

			if(testState == STOPPING)
				return;
		}

		if(testState == STOPPING)
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

		// rescale and update graphics
		bars[i]->Set(
				(qreal)(100 * result.__bytes_read) / READ_BLOCK_SIZE,
				(result.__time_elapsed > 0)?(qreal)result.__bytes_read / (qreal)result.__time_elapsed:0);
		reference_bars[i]->Set(
				(qreal)(100 * refer.__bytes_read) / READ_BLOCK_SIZE,
				(refer.__time_elapsed > 0)?(qreal)refer.__bytes_read / (qreal)refer.__time_elapsed:0);
		Rescale();
	}
}

int ReadBlock::GetProgress()
{
	hddsize read = 0;

	for(int i = 0; i < results.size(); ++i)
		read += results[i].__bytes_read;
	return (100 * read) / (results.size() * READ_BLOCK_SIZE);
}

ReadBlockResult::ReadBlockResult(hddsize block_size):
	__bytes_read(0), __time_elapsed(0), __block_size(block_size)
{
	erase();
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

void ReadBlock::RestoreResults(QDomElement &results, DataSet dataset)
{
	QList<ReadBlockResult> &res = (dataset == REFERENCE)?this->reference:this->results;

	// Locate main readblock element
	QDomElement seek = results.firstChildElement("Read_Block");
	if(!seek.attribute("valid", "no").compare("no"))
		return;

	// remove old results
	for(int i = 0; i < res.size(); ++i)
		res[i].erase();

	// get list of readblock subresults
	QDomNodeList xmlresults = seek.elementsByTagName("Result");

	// read subresults
	for(int i = 0; i < this->results.size(); ++i)
	{
		res[i].__block_size = xmlresults.at(i).toElement().attribute("size").toLongLong();
		res[i].__time_elapsed = xmlresults.at(i).toElement().attribute("time").toLongLong();
		res[i].__bytes_read = READ_BLOCK_SIZE;
	}

	// refresh view
	UpdateScene();
}

void ReadBlock::EraseResults(DataSet dataset)
{
	// erase data
	if(dataset == RESULTS)
		for(int i = 0; i < results.size(); ++i)
			results[i].erase();
	else
		for(int i = 0; i < reference.size(); ++i)
			reference[i].erase();

	// refresh view
	UpdateScene();
}

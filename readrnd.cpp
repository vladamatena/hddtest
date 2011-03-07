/**
*
* \author Vladimír Matěna
*
* Contact vlada.matena@gmail.com
*
*/

#include "readrnd.h"

ReadRnd::ReadRnd(QWidget *parent):
	TestWidget(parent)
{
	// add subtests to subtest list
	int base = 1048576;
	for(int i = 0; i < 12; ++i)
	{
		results.push_back(ReadRndResult(base));
		reference.push_back(ReadRndResult(base));
		base /= 2;
	}

	// test name and description
	testName = "Read random";
	testDescription = "Read random test reads " + QString::number(READ_RND_SIZE) +
			" blocks for each block size. Blocks are distributed randomly across the device." +
			" Therefore seeking is required to access next block. Block sizes are: ";
	for(int i = 0; i < results.size(); ++i)
	{
		if(i > 0)
			testDescription += ", ";
		testDescription += Def::Format(results[i].__block_size);
	}

	// add bars to scene
	for(int i = 0; i < results.size(); ++i)
	{
		Bar *bar = this->addBar(
				"MB/s",
				Def::Format(results[i].__block_size),
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
				Def::Format(reference[i].__block_size),
				QColor(0, 0xc0 * (i+1) / reference.size(), 0xff),
				(2*i + 1) * 1.0f / (reference.size() + reference.size()),
				1.0f / (reference.size() + reference.size() + 1));
		reference_bars.push_back(bar);
	}
}

void ReadRnd::TestLoop()
{
	// erase prevoius results
	for(int i = 0;i < results.size(); ++i)
		results[i].erase();

	// initialize random number generator
	RandomGenerator gen;

	// run subtests
	for(int i = 0; i < results.size(); ++i)
	{
		ReadRndResult &result = results[i];

		// run subtest
		while(result.__blocks_done < READ_RND_SIZE)
		{
			// get new position
			hddsize newpos = gen.Get64() % (device->GetSize() - result.__block_size);

			result.__time_elapsed += device->ReadAt(result.__block_size, newpos);
			result.__bytes_read += result.__block_size;
			result.__blocks_done++;

			if(!go)
				return;
		}

		if(!go)
			return;
	}
}


void ReadRnd::InitScene()
{

}

void ReadRnd::UpdateScene()
{
	// update subtest results
	for(int i = 0; i < results.size(); ++i)
	{
		//// update subresult
		const ReadRndResult &result = results.at(i);
		const ReadRndResult &refer = reference.at(i);

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

		// rescale and update graphics
		bars[i]->Set(
				(qreal)(100 * result.__blocks_done) / READ_RND_SIZE,
				(result.__time_elapsed > 0)?(qreal)result.__bytes_read / (qreal)result.__time_elapsed:0);
		reference_bars[i]->Set(
				(qreal)(100 * refer.__blocks_done) / READ_RND_SIZE,
				(refer.__time_elapsed > 0)?(qreal)refer.__bytes_read / (qreal)refer.__time_elapsed:0);
		Rescale();
	}
}

ReadRndResult::ReadRndResult(hddsize block_size):
	__bytes_read(0), __time_elapsed(0), __block_size(block_size)
{
	erase();
}

void ReadRndResult::erase()
{
	// reset bytes read and time elapsed
	__bytes_read = 0;
	__time_elapsed = 0;
	__blocks_done = 0;
}

QDomElement ReadRnd::WriteResults(QDomDocument &doc)
{
	// create main seek element
	QDomElement master = doc.createElement("Read_Random");
	master.setAttribute("valid", (GetProgress() == 100)?"yes":"no");
	doc.appendChild(master);

	// write subresults
	for(int i = 0; i < results.size(); ++i)
	{
		// add build element
		QDomElement build = doc.createElement("Result");
		build.setAttribute("size", results[i].__block_size);
		build.setAttribute("time", results[i].__time_elapsed);
		build.setAttribute("read", results[i].__bytes_read);
		master.appendChild(build);
	}

	return master;
}

void ReadRnd::RestoreResults(QDomElement &results, DataSet dataset)
{
	QList<ReadRndResult> &res = (dataset == REFERENCE)?this->reference:this->results;

	// Locate main readrnd element
	QDomElement seek = results.firstChildElement("Read_Random");
	if(!seek.attribute("valid", "no").compare("no"))
		return;

	// remove old results
	for(int i = 0; i < res.size(); ++i)
		res[i].erase();

	// get list of reads
	QDomNodeList xmlresults = seek.elementsByTagName("Result");

	// read subresults
	for(int i = 0; i < res.size(); ++i)
	{
		res[i].__block_size = xmlresults.at(i).toElement().attribute("size").toLongLong();
		res[i].__time_elapsed = xmlresults.at(i).toElement().attribute("time").toLongLong();
		res[i].__bytes_read = xmlresults.at(i).toElement().attribute("read").toLongLong();
		res[i].__blocks_done = READ_RND_SIZE;
	}

	// refresh view
	UpdateScene();
}

int ReadRnd::GetProgress()
{
	hddsize progress = 0;

	for(int i = 0; i < results.size(); ++i)
		progress += results[i].__blocks_done;
	return (100 * progress) / (results.size() * READ_RND_SIZE);
}

void ReadRnd::EraseResults(DataSet dataset)
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


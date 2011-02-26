/**
*
* \author Vladimír Matěna
*
* Contact vlada.matena@gmail.com
*
*/

#include "seeker.h"

Seeker::Seeker(QWidget *parent) :
	TestWidget(parent)
{
	dataAvgLine = addLine("ms", "Avg", QColor(255, 0, 0));
	referenceAvgLine = addLine("ms", "Avg", QColor(0, 0, 255));

	dataTicks = addTicks(QColor(255, 0, 0));
	referenceTicks = addTicks(QColor(0, 0, 255));

	net = addNet("ms", "Seek length", "Seek time");

	testName = "Seek";
	testDescription = "Seek test performs " + QString::number(SEEKCOUNT) + " seeks to random positions on device. Seek lenght is marked on horizontal axis and seek duration is on vertical axis.";
}

Seeker::~Seeker()
{
	delete dataAvgLine;
	delete referenceAvgLine;

	delete dataTicks;
	delete referenceTicks;
}

void Seeker::TestLoop()
{
	// erease previous results
	result.erase();

	// initialize random number generator
	RandomGenerator gen;

	// seek to drive end
	hddpos last = device->GetSize();
	device->SeekTo(last);

	// test SEEKCOUNT seeks
	for(int i = 0; i < SEEKCOUNT; ++i)
	{
		hddpos next = gen.Get64() % device->GetSize();	// get next position

		// make timed seek
		hddtime timediff = device->SeekTo(next);

		// count seek length
		uint64_t posdiff;
		if(last > next)
			posdiff = last - next;
		else
			posdiff = next - last;

		// set last position for next test
		last = next;

		qreal pos = (qreal)posdiff / (qreal)device->GetSize();	// calculate pos relative to drivesize
		qreal time = (qreal)timediff / 1000.0;					// evaluate seek time in miliseconds

		result.AddSeek(QPointF(pos, time));	// add seek to results

		// count test progress
		result.progress = i * 100 / (SEEKCOUNT - 1);

		if(go == false)
			return;
	}
}

void Seeker::InitScene()
{
	// clear ticks
	dataTicks->erase();
}

void Seeker::UpdateScene()
{
	// draw new seeks
	for(int i = 0; i < result.newseeks.count(); ++i)
	{
		QPointF seek = result.newseeks.pop();
		dataTicks->AddTick(seek.y(), seek.x());
	}

	// draw new reference seeks
	for(int i = 0; i < reference.newseeks.count(); ++i)
	{
		QPointF seek = reference.newseeks.pop();
		referenceTicks->AddTick(seek.y(), seek.x());
	}

	// update lines
	dataAvgLine->SetValue(result.avg());
	referenceAvgLine->SetValue(reference.avg());

	// rescale view
	Rescale();
}

qreal Seeker::GetProgress()
{
	return result.progress / 100.0f;
}

void Seeker::SeekResult::AddSeek(QPointF seek)
{
	seeks.push_back(seek);		// add seek to result seek list
	newseeks.push_back(seek);	// add sekk to stack used for drawing new results

	// if seek lasted long add it to maxims
	if(maxims.count() < SEEK_MAX_AVG)	// not enough maxims - add any seek
	{
		maxims.push_front(seek.y());
		qSort(maxims);
	}
	else	// enough maxims - add slower seeks
	{
		if(seek.y() > maxims[0])
		{
			maxims[0] = seek.y();
			qSort(maxims);
		}
	}

	// is seek was extraordinary fast - add to mins
	if(mins.count() < SEEK_MIN_AVG)	// too few minims - add any seek
	{
		mins.push_front(seek.y());
		qSort(mins);
	}
	else	// min pool is full add only fastest seeks
	{
		if(seek.y() < mins[SEEK_MIN_AVG - 1])
		{
			mins[SEEK_MIN_AVG - 1] = seek.y();
			qSort(mins);
		}
	}
}

qreal Seeker::SeekResult::avg()
{
	// count arithmetic average from seek times
	qreal sum = 0;
	for(int i = 0;i < seeks.count(); ++i)
		sum += seeks[i].y();

	return sum / seeks.count();
}

qreal Seeker::SeekResult::max()
{
	// count arithmetic average from MAX_AVG slowest seeks
	if(maxims.count() > 0)
	{
		qreal sum = 0;
		for(int i = 0; i < maxims.count(); ++i)
			sum += maxims[i];

		return sum / maxims.count();
	}
	else
	{
		return 0;
	}
}

qreal Seeker::SeekResult::min()
{
	// count arithmetic average from MIN_AVG fastest seeks
	if(mins.count() > 0)
	{
		qreal sum = 0;
		for(int i = 0; i < mins.count(); ++i)
			sum += mins[i];

		return sum / mins.count();
	}
	else
	{
		return 0;
	}
}

void Seeker::SeekResult::erase()
{
	seeks.erase(seeks.begin(), seeks.end());
	newseeks.erase(newseeks.begin(), newseeks.end());
	mins.erase(mins.begin(), mins.end());
	maxims.erase(maxims.begin(), maxims.end());
	progress = 0.0f;
}

QDomElement Seeker::WriteResults(QDomDocument &doc)
{
	// create main seek element
	QDomElement master = doc.createElement("Seek");
	master.setAttribute("valid", (result.progress==100)?"yes":"no");
	doc.appendChild(master);

	// add values to main element
	if(result.progress == 100) for(int i = 0; i < result.seeks.size(); ++i)
	{
		QDomElement value = doc.createElement("Seek");
		value.setAttribute("length", result.seeks[i].x());
		value.setAttribute("time", result.seeks[i].y());
		master.appendChild(value);
	}

	return master;
}

void Seeker::RestoreResults(QDomElement &results, bool reference)
{
	SeekResult &result = reference?this->reference:this->result;

	// Locate main seek element
	QDomElement seek = results.firstChildElement("Seek");
	if(!seek.attribute("valid", "no").compare("no"))
		return;

	// clear results and initialize scene
	result.erase();
	reference?referenceTicks->erase():dataTicks->erase();

	// get list of seeks
	QDomNodeList seeks = seek.elementsByTagName("Seek");

	// read result data
	for(int i = 0; i < seeks.size(); ++i)
			result.AddSeek(
				QPointF(
						seeks.at(i).toElement().attribute("length", "0").toDouble(),
						seeks.at(i).toElement().attribute("time", "0").toDouble()));

	// set progress
	result.progress = 100;

	// refresh view
	UpdateScene();
}

void Seeker::EraseResults()
{
	result.erase();
}

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
	testDescription = "Seek test performs " + QString::number(SEEKER_SEEKCOUNT) +
			" seeks to random positions on device." +
			" Seek lenght is marked on horizontal axis and seek duration is on vertical axis.";
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
	hddsize last = device->GetSize();
	device->SeekTo(last);

	// test SEEKER_SEEKCOUNT seeks
	for(int i = 0; i < SEEKER_SEEKCOUNT; ++i)
	{
		hddsize next = gen.Get64() % device->GetSize();	// get next position

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
		result.progress = i * 100 / (SEEKER_SEEKCOUNT - 1);

		if(!go)
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
	while(!result.newseeks.empty())
	{
		QPointF seek = result.newseeks.pop();

		// mark seek as important if it is close to average
		if(seek.ry() < SEEKER_IMPORTANT * result.avg())
			dataTicks->AddTick(seek.y(), seek.x(), true);
		else
			dataTicks->AddTick(seek.y(), seek.x(), false);
	}

	// draw new reference seeks
	while(!reference.newseeks.empty())
	{
		QPointF seek = reference.newseeks.pop();

		// mark seek as important if it is close to average
		if(seek.ry() < SEEKER_IMPORTANT * reference.avg())
			referenceTicks->AddTick(seek.y(), seek.x(), true);
		else
			referenceTicks->AddTick(seek.y(), seek.x(), false);
	}

	// update lines
	dataAvgLine->SetValue(result.avg());
	referenceAvgLine->SetValue(reference.avg());

	// rescale view
	Rescale();
}

int Seeker::GetProgress()
{
	return result.progress;
}

void Seeker::SeekResult::AddSeek(QPointF seek)
{
	seeks.push_back(seek);		// add seek to result seek list
	newseeks.push_back(seek);	// add sekk to stack used for drawing new results
}

qreal Seeker::SeekResult::avg()
{
	// count arithmetic average from seek times
	qreal sum = 0;
	for(int i = 0;i < seeks.count(); ++i)
		sum += seeks[i].y();

	return sum / seeks.count();
}

void Seeker::SeekResult::erase()
{
	seeks.erase(seeks.begin(), seeks.end());
	newseeks.erase(newseeks.begin(), newseeks.end());
	progress = 0.0f;
}

QDomElement Seeker::WriteResults(QDomDocument &doc)
{
	// create main seek element
	QDomElement master = doc.createElement("Seeker");
	master.setAttribute("valid", (GetProgress() == 100)?"yes":"no");
	doc.appendChild(master);

	// add values to main element
	if(GetProgress() == 100) for(int i = 0; i < result.seeks.size(); ++i)
	{
		QDomElement value = doc.createElement("Seek");
		value.setAttribute("length", result.seeks[i].x());
		value.setAttribute("time", result.seeks[i].y());
		master.appendChild(value);
	}

	return master;
}

void Seeker::RestoreResults(QDomElement &results, DataSet dataset)
{
	SeekResult &result = (dataset == REFERENCE)?this->reference:this->result;

	// Locate main seek element
	QDomElement seek = results.firstChildElement("Seeker");
	if(!seek.attribute("valid", "no").compare("no"))
		return;

	// clear results and initialize scene
	result.erase();
	(dataset == REFERENCE)?referenceTicks->erase():dataTicks->erase();

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

void Seeker::EraseResults(DataSet dataset)
{
	if(dataset == RESULTS)
	{
		result.erase();
		dataTicks->erase();
	}
	else
	{
		reference.erase();
		referenceTicks->erase();
	}
	UpdateScene();
}

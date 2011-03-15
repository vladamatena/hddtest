#include "testwidget.h"
#include "ui_testwidget.h"

// TestThread holds pointer to TestWidget and vice versa
// so extra including in cpp file is needed
#include "testthread.h"

TestWidget::TestWidget(QWidget *parent) :
	QWidget(parent), ui(new Ui::TestWidget)
{
    ui->setupUi(this);

	device = NULL;

	test_thread = new TestThread(this);
	testState = STOPPED;

	connect(&refresh_timer, SIGNAL(timeout()), this, SLOT(refresh_timer_timeout()));
	connect(test_thread, SIGNAL(test_started()), this, SLOT(test_started()));
	connect(test_thread, SIGNAL(test_stopped()), this, SLOT(test_stopped()));

	Yscale = 1;

	ui->graph->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
	ui->graph->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->graph->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	// set graph scene
	scene = new QGraphicsScene(ui->graph->rect(), ui->graph);
	ui->graph->setScene(scene);
}

TestWidget::~TestWidget()
{
	for(int i = 0; i < markers.size(); ++i)
		delete markers[i];

	scene->clear();
	delete scene;

    delete ui;
	delete test_thread;
}

void TestWidget::SetDevice(Device *device)
{
	this->device = device;
}

void TestWidget::refresh_timer_timeout()
{
	int progress = GetProgress();
	ui->progress->setValue(progress);
	UpdateScene();
}

void TestWidget::on_startstop_clicked()
{
	if(testState == STOPPED)
		StartTest();
	else if(testState == STARTED)
		StopTest();
	else
		std::cerr << "Start/stop test clicked but should be disabled" << std::endl;
}

void TestWidget::StartTest()
{
	if(!device)
	{
		std::cerr << "WARNING: Start test without valid device pointer - ignoring" << std::endl;
		return;
	}

	// prepare ui for test
	testState = STARTING;
	ui->startstop->setText("Starting");
	ui->startstop->setEnabled(false);
	InitScene();

	// start test in another thread
	test_thread->start();
}

void TestWidget::StopTest()
{
	testState = STOPPING;
	ui->startstop->setText("Stopping");
	ui->startstop->setEnabled(false);
}

void TestWidget::test_started()
{
	// start ui refresh
	refresh_timer.start(100);

	testState = STARTED;
	ui->startstop->setText("Stop");
	ui->startstop->setEnabled(true);
}

void TestWidget::test_stopped()
{
	// stop scene refresh but make sure it is up-to-date
	refresh_timer_timeout();
	refresh_timer.stop();

	testState = STOPPED;
	ui->startstop->setText("Start");
	ui->startstop->setEnabled(true);
}

void TestWidget::SetStartEnabled(bool enabled)
{
	ui->startstop->setEnabled(enabled);
}

void TestWidget::on_info_clicked()
{
	// Show test description
	QMessageBox box;
	box.setText(testName);
	box.setInformativeText(testDescription);
	box.exec();
}

void TestWidget::on_image_clicked()
{
	// save test result as image
	QString filename = QFileDialog::getSaveFileName(this, tr("Save result image"), "", tr("Images (*.png)"));
	if(filename.length() > 0)
	{
		QImage image(scene->width(), scene->height(), QImage::Format_RGB32);
		image.fill(QColor(Qt::white).rgb());
		QPainter painter(&image);
		scene->render(&painter);
		image.save(filename);
	}
}

void TestWidget::Rescale(bool force)
{
	// get absolute min and max
	qreal max = 0.0f;
	qreal min = 0.0f;
	for(int i = 0; i < markers.size(); ++i)
	{
		if(markers[i]->max > max)
			max = markers[i]->max;
		if(markers[i]->min < min)
			min = markers[i]->min;
	}

	// calculate Y axis scale
	if((max * Yscale > graph.height() * 0.9) || (max * Yscale < graph.height() / 2) || force)
	{
		// calculate new Yscale to fit data in view
		if(max != min)
			Yscale = (qreal)graph.height() / (max * 1.5);

		// reposition markers according new Yscale
		for(int i = 0; i < markers.size(); ++i)
			markers[i]->Reposition();
	}

	// redraw scene
	scene->update();
}

void TestWidget::resizeEvent(QResizeEvent*)
{
	// resize scene to new window dimensions
	QRect rect = ui->graph->rect();
	scene->setSceneRect(rect);

	// set graph rect according to new window size
	graph = QRect(
				rect.width() * 0.05f,
				rect.height() * 0.05f,
				rect.width() * 0.9f,
				rect.height() * 0.9f);

	// reposition scene items according to new window dimensions
	Rescale(true);
}

///////////////////////////////////////////////////////////////////////////////
/////// Marker add functions //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TestWidget::Marker::Marker(TestWidget *test):
	max(0.0f), min(0.0f)
{
	this->test = test;
}

TestWidget::Marker::~Marker()
{

}

TestWidget::Line* TestWidget::addLine(QString unit, QString name, QColor color)
{
	Line * line = new Line(this, unit, name, color);
	markers.push_back(line);

	return line;
}

TestWidget::Ticks* TestWidget::addTicks(QColor color)
{
	Ticks *ticks = new Ticks(this, color);
	markers.push_back(ticks);

	return ticks;
}

TestWidget::Bar* TestWidget::addBar(QString unit, QString name, QColor color, qreal position, qreal width)
{
	Bar *bar = new Bar(this, unit, name, color, position, width);
	markers.push_back(bar);

	return bar;
}

TestWidget::LineGraph* TestWidget::addLineGraph(QString unit, QColor color)
{
	LineGraph *linegraph = new LineGraph(this, unit, color);
	markers.push_back(linegraph);

	return linegraph;
}

TestWidget::Net* TestWidget::addNet(QString unit, QString xAxis, QString yAxis)
{
	Net *net = new Net(this, unit, xAxis, yAxis);
	markers.push_back(net);

	return net;
}

TestWidget::Legend* TestWidget::addLegend()
{
	Legend *legend = new Legend(this);
	markers.push_back(legend);

	return legend;
}






///////////////////////////////////////////////////////////////////////////////
/////// Marker management functions ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TestWidget::Line::Line(TestWidget *test, QString unit, QString name, QColor color):
	Marker(test), unit(unit), name(name), color(color), line(NULL), text(NULL), value(0) {}

TestWidget::Line::~Line()
{
	delete line;
	delete text;
}

void TestWidget::Line::SetValue(qreal value)
{
	// update min and max for Line
	max = min = value;
	this->value = value;

	// add line
	if(line == NULL)
	{
		line = test->scene->addLine(0, 0, 0, 0, QPen(color));
		line->setZValue(100);
	}

	// add text
	if(text == NULL)
	{
		text = test->scene->addText("NO DATA");
		text->setDefaultTextColor(color);

		QFont font = text->font();
		font.setBold(true);
		text->setFont(font);
	}

	// shide line if value is zero
	text->setVisible(value != 0);
	line->setVisible(value != 0);

	// set new line position
	line->setLine(
			test->graph.left(),
			test->graph.top() + test->graph.height() - value * test->Yscale,
			test->graph.left() + test->graph.width() - text->boundingRect().width(),
			test->graph.top() + test->graph.height() - value * test->Yscale);

	// set new text position and content
	text->setPos(
			test->graph.left() + test->graph.width() - text->boundingRect().width(),
			test->graph.top() + test->graph.height() - value * test->Yscale - text->boundingRect().height() / 2);

	if(name.length() > 0)
		text->setPlainText(name + ": " + QString::number(value, 'f', 2) + " " + unit);
	else
		text->setPlainText(QString::number(value, 'f', 2) + " " + unit);
}

void TestWidget::Line::Reposition()
{
	SetValue(value);
}

TestWidget::Ticks::Ticks(TestWidget *test, QColor color):
	 Marker(test), color(color) {}

TestWidget::Ticks::~Ticks()
{
	for(int i = 0; i < ticks.size(); ++i)
		delete ticks[i].tick;
}

void TestWidget::Ticks::AddTick(qreal value, qreal position, bool important)
{
	// update max and min for ticks
	if(important)
	{
		if(value > max)
			max = value;
		if(value < min)
			min = value;
	}

	// add tick
	Tick tick;

	tick.tick = test->scene->addRect(
			test->graph.left() + position *  test->graph.width() * NET_WIDTH,
			test->graph.top() + test->graph.height() - value * test->Yscale,
			1,
			1,
			QPen(color));
	tick.position = position;
	tick.value = value;

	ticks.push_back(tick);	
}

void TestWidget::Ticks::Reposition()
{
	// reposition ticks
	for(int i = 0; i < ticks.size(); ++i)
		ticks[i].tick->setRect(
					test->graph.left() + ticks[i].position *  test->graph.width() * NET_WIDTH,
					test->graph.top() + test->graph.height() - ticks[i].value * test->Yscale,
					1,
					1);
}

void TestWidget::Ticks::erase()
{
	// erase all ticks
	for(int i = 0; i < ticks.size(); ++i)
		test->scene->removeItem(ticks[i].tick);
	ticks.clear();

	// reset min and max
	min = max = 0.0f;
}

TestWidget::Bar::Bar(TestWidget *test, QString unit, QString name, QColor color, qreal position, qreal width) :
		Marker(test), unit(unit), name(name), color(color), rect(NULL), inner_rect(NULL),
		value_text(NULL), name_text(NULL), position(position), width(width), value(0), progress(0) {}

TestWidget::Bar::~Bar()
{
	delete rect;
	delete inner_rect;
	delete value_text;
	delete name_text;
}

void TestWidget::Bar::Set(qreal progress, qreal value)
{
	this->value = value;
	this->progress = progress;

	// update min and max for bars
	max = value;
	min = 0;

	//// Create bar items
	// bar value
	if(value_text == NULL)
	{
		value_text = test->scene->addText("NO DATA");
		value_text->setDefaultTextColor(color);
		value_text->setZValue(100);		
	}

	// bar name
	if(name_text == NULL)
	{
		name_text = test->scene->addText(name);
		name_text->setDefaultTextColor(color);
	}

	// outer rect (bar frame)
	if(rect == NULL)
		rect = test->scene->addRect(0, 0, 0, 0, QPen(color));

	// inner rect (progress)
	if(inner_rect == NULL)
		inner_rect = test->scene->addRect(0, 0, 0, 0, QPen(Qt::NoPen), QBrush(color.darker(70)));

	// set items positions
	int W = test->graph.width() * width;
	int H = value * test->Yscale;
	int X = test->graph.left() + test->graph.width() * position;
	int Y = test->graph.top() + test->graph.height() - H - name_text->boundingRect().height();

	// set items data
	value_text->setPlainText(QString::number(value, 'f', 2) + " " + unit);
	value_text->setVisible(progress > 0);

	//// set positions
	rect->setRect(X, Y, W, H);
	inner_rect->setRect(X, Y + H * (100 - progress) / 100, W, H * progress / 100);

	value_text->setPos(
			X + W / 2 - value_text->boundingRect().width() / 2,
			rect->boundingRect().y() - value_text->boundingRect().height());

	name_text->setPos(
			X + W / 2 - name_text->boundingRect().width() / 2,
			test->graph.top() + test->graph.height() - name_text->boundingRect().height());
}

void TestWidget::Bar::Reposition()
{
	Set(progress, value);
}

TestWidget::LineGraph::LineGraph(TestWidget *test, QString unit, QColor color):
	 Marker(test), unit(unit), color(color)
{
	// set to something else than 0 should be changed by test before first use
	size = 10;
}

TestWidget::LineGraph::~LineGraph()
{
	for(int i = 0; i < lines.size(); ++i)
		delete lines[i];
}

void TestWidget::LineGraph::SetSize(int count)
{
	size = count;
	Reposition();
}

void TestWidget::LineGraph::AddValue(qreal value)
{
	// update max and min for linegraph
	if(value > max)
		max = value;
	if(value < min)
		min = value;

	// add new line
	values.push_back(value);

	if(values.size() > 1)
	{
		QPen pen(color);
		pen.setWidth(2);
		pen.setCapStyle(Qt::RoundCap);
		pen.setJoinStyle(Qt::RoundJoin);
		lines.push_back(test->scene->addLine(0, 0, 0, 0, pen));
		Reposition();
	}
}

void TestWidget::LineGraph::erase()
{
	for(int i = 0; i < lines.size(); ++i)
	{
		test->scene->removeItem(lines[i]);
		delete lines[i];
	}

	lines.clear();
	values.clear();

	size = 10;

	// reset min and max
	min = max = 0.0f;
}

void TestWidget::LineGraph::Reposition()
{
	// reposition lines
	for(int i = 0; i < lines.size(); ++i)
	{
		lines[i]->setLine(
				test->graph.left() + (qreal)i  / (size - 1) * test->graph.width() * NET_WIDTH,
				test->graph.top() + test->graph.height() - values[i] * test->Yscale,
				test->graph.left() + (qreal)(i + 1)  / (size - 1) * test->graph.width() * NET_WIDTH,
				test->graph.top() + test->graph.height() - values[i + 1] * test->Yscale);
	}	
}

TestWidget::Net::Net(TestWidget *test, QString unit, QString xAxis, QString yAxis):
	Marker(test), unit(unit), xAxis(xAxis), yAxis(yAxis)
{
	// left vertical line of net
	left_line = test->scene->addLine(
				test->graph.left(),
				test->graph.top(),
				test->graph.left(),
				test->graph.top() + test->graph.height(),
				QPen(QColor(200,200,200)));

	// axis descriptions
	xAxisText = test->scene->addText(xAxis);
	yAxisText = test->scene->addText(yAxis);
	yAxisText->rotate(-90);
}

TestWidget::Net::~Net()
{
	for(int i = 0; i < net.size(); ++i)
		delete net[i];

	for(int i = 0; i < net_markups.size(); ++i)
		delete net_markups[i];

	delete left_line;
	delete xAxisText;
	delete yAxisText;
}

void TestWidget::Net::Reposition()
{
	// If Yscale is not set do not do anything
	if((test->Yscale == 0) || (test->Yscale == INFINITY))
		return;

	// reposition left line
	left_line->setLine(
				test->graph.left(),
				test->graph.top(),
				test->graph.left(),
				test->graph.top() + test->graph.height());

	// reposition axis descriptions
	xAxisText->setPos(
				test->graph.left() + xAxisText->boundingRect().height(),
				test->graph.bottom());
	yAxisText->setPos(
				test->graph.left() - yAxisText->boundingRect().height(),
				test->graph.bottom() - yAxisText->boundingRect().height());


	// get font height
	qreal fontHeight = xAxisText->boundingRect().height();

	// get distance
	qreal dist = (3 * fontHeight) / (10 * test->Yscale);
	dist = pow(10, round(log10(dist)));

	// get line count
	int count = test->graph.height() / (test->Yscale * dist);

	// construct lines
	while(net.size() != count)
	{
		if(net.size() > count) // lines are too many - remove one
		{
			test->scene->removeItem(net.back());
			delete net.back();
			net.pop_back();
		}
		else	// missing some lines - add one
		{
			net.push_back(test->scene->addLine(0,0,0,0,QPen(QColor(200,200,200))));
			net.back()->setZValue(-100);
		}
	}

	// construct markups
	while(net_markups.size() != (count / 10) + 1)
	{
		if(net_markups.size() > (count / 10) + 1) // markups are too many - remove one
		{
			test->scene->removeItem(net_markups.back());
			delete net_markups.back();
			net_markups.pop_back();
		}
		else	// missing some markups - add one
		{
			net_markups.push_back(test->scene->addText(""));
			net_markups.back()->setZValue(-100);
			net_markups.back()->setDefaultTextColor(QColor(230,230,230));
		}
	}

	// position lines and markups
	for(int i = 0; i < net.size(); ++i)
	{
		if(i % 10)	// default line
		{
			net[i]->setLine(
					test->graph.left(),
					test->graph.top() + test->graph.height() - test->Yscale * dist * i,
					test->graph.left() + test->graph.width() * NET_WIDTH,
					test->graph.top() + test->graph.height() - test->Yscale * dist * i);
			net[i]->setPen(QPen(QColor(230,230,230)));
		}
		else		// extra line with value
		{
			// position line
			net[i]->setLine(
					test->graph.left(),
					test->graph.top() + test->graph.height() - test->Yscale * dist * i,
					test->graph.left() + test->graph.width() * NET_HIGHLIGHT_WIDTH,
					test->graph.top() + test->graph.height() - test->Yscale * dist * i);
			net[i]->setPen(QPen(QColor(200,200,200)));

			// position and set text
			if(i / 10 < net_markups.size())
			{
				net_markups[i / 10]->setPlainText(QString::number(i * dist) + " " + unit);
				net_markups[i / 10]->setPos(
						test->graph.left() + test->graph.width() * NET_HIGHLIGHT_WIDTH,
						test->graph.top() + test->graph.height() - (i * test->Yscale * dist)
							- net_markups[i / 10]->boundingRect().height() / 2);
			}
		}
	}
}

TestWidget::Legend::Legend(TestWidget *test):
	Marker(test) {}

TestWidget::Legend::~Legend()
{
	for(int i = 0; i < items.size(); ++i)
	{
		delete items[i].rect;
		delete items[i].text;
	}
}

void TestWidget::Legend::AddItem(QString name, QColor color)
{
	Item item;
	item.rect = test->scene->addRect(0, 0, 0, 0, color, color);
	item.text = test->scene->addText(name);
	items.push_back(item);
}

void TestWidget::Legend::Reposition()
{
	// first free position
	int pos = test->graph.right();

	// position items
	for(int i = 0; i < items.size(); ++i)
	{
		int width = items[i].text->boundingRect().height();
		items[i].text->setPos(pos -= items[i].text->boundingRect().width(), 0);
		items[i].rect->setRect(
					pos -= width,
					width * 0.1f,
					width * 0.8f,
					width * 0.8f);
		pos -= width;
	}
}


#include "testwidget.h"
#include "ui_testwidget.h"

TestWidget::TestWidget(QWidget *parent) :
	QWidget(parent), ui(new Ui::TestWidget)
{
    ui->setupUi(this);

	device = NULL;

	test_thread = new TestThread(this);
	running = false;
	go = false;
	connect(&refresh_timer, SIGNAL(timeout()), this, SLOT(refresh_timer_timeout()));

	Yscale = 0;

	ui->graph->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
	ui->graph->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->graph->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	// set graph scene
	scene = new QGraphicsScene(ui->graph->rect(), ui->graph);
	ui->graph->setScene(scene);
}

TestWidget::~TestWidget()
{
    delete ui;
	delete test_thread;
}

void TestWidget::SetDevice(Device *device)
{
	this->device = device;
}

TestWidget::TestThread::TestThread(TestWidget *widget)
{
	this->widget = widget;
}

void TestWidget::TestThread::run()
{
	widget->go = true;
	widget->running = true;
	widget->TestLoop();
	widget->running = false;
}

void TestWidget::refresh_timer_timeout()
{
	qreal progress = GetProgress();
	ui->progress->setValue(progress * 100);
	if(progress == 1)
	{
		refresh_timer.stop();
		ui->startstop->setText("Start");
	}

	UpdateScene();
}

void TestWidget::StartTest()
{
	if(!device)
		return;
	ui->startstop->setText("Stop");
	InitScene();
	test_thread->start();
	refresh_timer.start(100);
}

void TestWidget::StopTest()
{
	refresh_timer.stop();
	ui->startstop->setText("Start");
	go = false;
	running = false;
}

void TestWidget::SetStartEnabled(bool enabled)
{
	ui->startstop->setEnabled(enabled);
}

void TestWidget::on_startstop_clicked()
{
	if(running == false)
		StartTest();
	else
		StopTest();
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
	if((max * Yscale > scene->height()) || (max * Yscale < scene->height() / 2) || (Yscale == 0) || force)
	{
		// calculate new Yscale to fit data in view
		Yscale = (qreal)scene->height() / (max * 1.5);

		// reposition markers according new Yscale
		for(int i = 0; i < markers.size(); ++i)
			markers[i]->Reposition();
	}
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

TestWidget::Marker::Marker():
	max(0.0f), min(0.0f) {}

TestWidget::Line* TestWidget::addLine(QString unit, QString name, QColor color)
{
	Line * line = new Line(this, unit, name, color);
	markers.push_back(line);

	return line;
}

TestWidget::Ticks* TestWidget::addTicks(QColor color)
{
	Ticks *ticks = new Ticks(this, color);
	markers.push_back(ticks); // TODO delete markers on exit

	return ticks;
}

TestWidget::Bar* TestWidget::addBar(QString unit, QString name, QColor color, qreal position, qreal width)
{
	Bar *bar = new Bar(this, unit, name, color, position, width);
	markers.push_back(bar);	// TODO delete markers on exit

	return bar;
}

TestWidget::LineGraph* TestWidget::addLineGraph(QString unit, QColor color)
{
	LineGraph *linegraph = new LineGraph(this, unit, color);
	markers.push_back(linegraph); // TODO delete markers on exit

	return linegraph;
}

TestWidget::Net* TestWidget::addNet(QString unit)
{
	Net *net = new Net(this, unit);
	markers.push_back(net);	// TODO delete merkers on exit

	return net;
}





///////////////////////////////////////////////////////////////////////////////
/////// Marker management functions ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TestWidget::Line::Line(TestWidget *test, QString unit, QString name, QColor color):
		unit(unit), name(name), color(color), line(NULL), text(NULL), value(0)
{
	this->test = test;
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
	color(color)
{
	this->test = test;
}

void TestWidget::Ticks::AddTick(qreal value, qreal position)
{
	// update max and min for ticks
	if(value > max)
		max = value;
	if(value < min)
		min = value;

	// add tick
	QGraphicsRectItem *tick;

	tick = test->scene->addRect(
			test->graph.left() + position *  test->graph.width() * LINEGRAPH_WIDTH,
			test->graph.top() + test->graph.height() - value * test->Yscale,
			1,
			1,
			QPen(color));

	ticks.push_back(tick);
	positions.push_back(QPointF(position, value));
}

void TestWidget::Ticks::Reposition()
{
	// reposition ticks
	for(int i = 0; i < ticks.size(); ++i)
		ticks[i]->setRect(
					test->graph.left() + positions[i].rx() *  test->graph.width() * LINEGRAPH_WIDTH,
					test->graph.top() + test->graph.height() - positions[i].ry() * test->Yscale,
					1,
					1);
}

void TestWidget::Ticks::erase()
{
	// erase all ticks
	for(int i = 0; i < ticks.size(); ++i)
		test->scene->removeItem(ticks[i]);
	ticks.clear();
	positions.clear();

	// reset min and max
	min = max = 0.0f;
}

TestWidget::Bar::Bar(TestWidget *test, QString unit, QString name, QColor color, qreal position, qreal width) :
		unit(unit), name(name), color(color), rect(NULL), inner_rect(NULL),
		value_text(NULL), name_text(NULL), position(position), width(width), value(0), progress(0)
{
	this->test = test;
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

TestWidget::LineGraph::LineGraph(TestWidget *test, QString unit, QColor color) :
		 unit(unit), color(color)
{
	this->test = test;
	size = 10;
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
		test->scene->removeItem(lines[i]);

	lines.erase(lines.begin(), lines.end());
	values.erase(values.begin(), values.end());

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
				test->graph.left() + (qreal)i  / (size - 1) * test->graph.width() * LINEGRAPH_WIDTH,
				test->graph.top() + test->graph.height() - values[i] * test->Yscale,
				test->graph.left() + (qreal)(i + 1)  / (size - 1) * test->graph.width() * LINEGRAPH_WIDTH,
				test->graph.top() + test->graph.height() - values[i + 1] * test->Yscale);
	}	
}

TestWidget::Net::Net(TestWidget *test, QString unit)
{
	this->test = test;
	this->unit = unit;

	left_line = test->scene->addLine(
				test->graph.left(),
				test->graph.top(),
				test->graph.left(),
				test->graph.top() + test->graph.height(),
				QPen(QColor(200,200,200)));
}

void TestWidget::Net::Reposition()
{
	// If Yscale is not set do not do anything
	if(test->Yscale == 0)
		return;

	// reposition left line
	left_line->setLine(
				test->graph.left(),
				test->graph.top(),
				test->graph.left(),
				test->graph.top() + test->graph.height());

	// get distances
	int dist = 1;		//TODO: handle distances in better way

	// get count
	int count = test->graph.height() / (test->Yscale * dist);

	// construct lines
	while(net.size() != count)
	{
		if(net.size() > count) // lines are too many - remove one
		{
			test->scene->removeItem(net.back());
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
		if((dist * i) % 10)	// default line
		{
			net[i]->setLine(
					test->graph.left(),
					test->graph.top() + test->graph.height() - test->Yscale * dist * i,
					test->graph.left() + test->graph.width() * LINEGRAPH_WIDTH,
					test->graph.top() + test->graph.height() - test->Yscale * dist * i);
			net[i]->setPen(QPen(QColor(230,230,230)));
		}
		else		// extra line with value
		{
			// position line
			net[i]->setLine(
					test->graph.left(),
					test->graph.top() + test->graph.height() - test->Yscale * dist * i,
					test->graph.left() + test->graph.width() * LINEGRAPH_NET_WIDTH,
					test->graph.top() + test->graph.height() - test->Yscale * dist * i);
			net[i]->setPen(QPen(QColor(200,200,200)));

			// position and set text
			if(i / 10 < net_markups.size())
			{
				net_markups[i / 10]->setPlainText(QString::number(i * dist) + " " + unit);
				net_markups[i / 10]->setPos(
						test->graph.left() + test->graph.width() * LINEGRAPH_NET_WIDTH,
						test->graph.top() + test->graph.height() - (i * test->Yscale * dist) - net_markups[i / 10]->boundingRect().height() / 2);
			}
		}
	}
}

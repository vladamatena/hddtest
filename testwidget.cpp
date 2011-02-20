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

	// set graph scene
	QRect content = ui->graph->rect();
	scene = new QGraphicsScene(0, 0, content.width(), content.height());

	ui->graph->setScene(scene);
	ui->graph->fitInView(0, 0, content.width(), content.height(), Qt::KeepAspectRatio);
	ui->graph->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
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
		StartTest(); // TODO add device here instead of NULL
	else
		StopTest();
}

void TestWidget::Rescale(bool force)
{
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
		Yscale = (qreal)scene->height() / (max * 1.5);	// calc new multipiler
		resizeEvent(NULL);
	}
}

void TestWidget::resizeEvent(QResizeEvent *event)
{
	QRect rect = ui->graph->rect();
	scene->setSceneRect(rect);
	ui->graph->fitInView(
				rect.width() * 0.05,
				rect.height() * 0.05,
				rect.width() * 1.05,
				rect.height() * 1.05,
				Qt::KeepAspectRatio);

	for(int i = 0; i < markers.size(); ++i)
		markers[i]->Reposition();
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

TestWidget::LineGraph* TestWidget::addLineGraph(QString unit, bool net, QColor color)
{
	LineGraph *linegraph = new LineGraph(this, unit, net, color);
	markers.push_back(linegraph); // TODO delete markers on exit

	return linegraph;
}





///////////////////////////////////////////////////////////////////////////////
/////// Marker management functions ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TestWidget::Line::Line(TestWidget *test, QString unit, QString name, QColor color):
		unit(unit), name(name), color(color), line(NULL), text(NULL)
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
			0,
			test->scene->height() - value * test->Yscale,
			test->scene->width() - text->boundingRect().width(),
			test->scene->height() - value * test->Yscale);

	// set new text position and content
	text->setPos(
			test->scene->width() - text->boundingRect().width(),
			test->scene->height() - value * test->Yscale - text->boundingRect().height() / 2);

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
	color(color), rect(NULL)
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

	// add graph bounding rect
	if(rect)
		rect->setRect(
				0,
				0,
				test->scene->width() * LINEGRAPH_WIDTH,
				test->scene->height());
	else
		rect = test->scene->addRect(
				0,
				0,
				test->scene->width() * LINEGRAPH_WIDTH,
				test->scene->height(),
				QPen(QColor(0,0,0)));

	// add tick
	QGraphicsRectItem *tick;

	tick = test->scene->addRect(
			position *  test->scene->width() * LINEGRAPH_WIDTH,
			test->scene->height() - value * test->Yscale,
			1,
			1,
			QPen(color));

	ticks.push_back(tick);
	positions.push_back(QPointF(position, value));
}

void TestWidget::Ticks::Reposition()
{
	for(int i = 0; i < ticks.size(); ++i)
		ticks[i]->setRect(
					positions[i].rx() *  test->scene->width() * LINEGRAPH_WIDTH,
					test->scene->height() - positions[i].ry() * test->Yscale,
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
		value_text(NULL), name_text(NULL), position(position), width(width)
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


	// set items data
	value_text->setPlainText(QString::number(value, 'f', 2) + " " + unit);

	// set items positions
	int W = test->scene->width() * width;
	int H = value * test->Yscale;
	int X = test->scene->width() * position;
	int Y = test->scene->height() - H - name_text->boundingRect().height();

	//// set positions
	rect->setRect(X, Y, W, H);
	inner_rect->setRect(X, Y + H * (100 - progress) / 100, W, H * progress / 100);

	value_text->setPos(
			X + W / 2 - value_text->boundingRect().width() / 2,
			rect->boundingRect().y() - value_text->boundingRect().height());

	name_text->setPos(
			X + W / 2 - name_text->boundingRect().width() / 2,
			test->scene->height() - name_text->boundingRect().height());
}

void TestWidget::Bar::Reposition()
{
	Set(progress, value);
}

TestWidget::LineGraph::LineGraph(TestWidget *test, QString unit, bool shownet, QColor color) :
		 shownet(shownet), unit(unit), color(color)
{
	this->test = test;
	size = 10;
	Yscale_cached = 0;
	rect = NULL;
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
	Yscale_cached = 0;
	rect = NULL;

	// reset min and max
	min = max = 0.0f;
}

void TestWidget::LineGraph::Reposition()
{
	// add graph bounding rect
	if(rect)
	{
		rect->setRect(0,
					  0,
					  test->scene->width() * LINEGRAPH_WIDTH,
					  test->scene->height());
	}
	else
	{
		rect = test->scene->addRect(0,
								   0,
								   test->scene->width() * LINEGRAPH_WIDTH,
								   test->scene->height(),
								   QPen(QColor(0,0,0)));
	}

	// reposition lines
	for(int i = 0; i < lines.size(); ++i)
	{
		lines[i]->setLine(
				(qreal)i  / (size - 1) * test->scene->width() * LINEGRAPH_WIDTH,
				test->scene->height() - values[i] * test->Yscale,
				(qreal)(i + 1)  / (size - 1) * test->scene->width() * LINEGRAPH_WIDTH,
				test->scene->height() - values[i + 1] * test->Yscale);
	}

	// draw net
	if(shownet && (Yscale_cached != test->Yscale))
	{
		Yscale_cached = test->Yscale;

		// get distances
		qreal dist = 1;

		// get count
		int count = test->scene->height() / (test->Yscale * dist);

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

		// construct makups
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

		// position lines and makups
		for(int i = 0; i < net.size(); ++i)
		{
			if(i % 10)	// default line
			{
				net[i]->setLine(
						0,
						test->scene->height() - test->Yscale * dist * i,
						test->scene->width() * LINEGRAPH_WIDTH,
						test->scene->height() - test->Yscale * dist * i);
				net[i]->setPen(QPen(QColor(230,230,230)));
			}
			else		// extra line with value
			{
				// position line
				net[i]->setLine(
						0,
						test->scene->height() - test->Yscale * dist * i,
						test->scene->width() * LINEGRAPH_NET_WIDTH,
						test->scene->height() - test->Yscale * dist * i);
				net[i]->setPen(QPen(QColor(200,200,200)));

				// position and set text
				if(i / 10 < net_markups.size())
				{
					net_markups[i / 10]->setPlainText(QString::number(i) + " " + unit);
					net_markups[i / 10]->setPos(
							test->scene->width() * LINEGRAPH_NET_WIDTH,
							test->scene->height() - (i*test->Yscale * dist) - net_markups[i/10]->boundingRect().height() / 2);
				}
			}
		}
	}
}

#ifndef TESTWIDGET_H
#define TESTWIDGET_H

#include <QWidget>
#include <QThread>
#include <QGraphicsScene>
#include <QTimer>
#include <QList>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>

#include "device.h"

#define LINEGRAPH_WIDTH 0.88
#define LINEGRAPH_NET_WIDTH 0.89

#define TICKS_WIDTH 0.88


namespace Ui {
    class TestWidget;
}

class TestWidget : public QWidget
{
    Q_OBJECT

public:
	class Marker
	{
	public:
		TestWidget *test;
	};
	class Ticks : public Marker
	{
	public:
		Ticks(TestWidget *test, QColor color);
		void AddTick(qreal value, qreal position);
		void erase();

	private:
		QColor color;
		QList<QGraphicsRectItem*> ticks;
		QGraphicsRectItem *rect;
	};
	class Line : public Marker
	{
	public:
		Line(TestWidget *test, QString unit, QString name, QColor color);
		void SetValue(qreal value);

	private:
		QString unit, name;
		QColor color;
		QGraphicsLineItem *line;
		QGraphicsTextItem *text;
	};
	class Bar : public Marker	{
	public:
		Bar(TestWidget *test, QString unit, QString name, QColor color, qreal position, qreal width);
		void Set(qreal progress, qreal value);

	private:
		QString unit, name;
		QColor color;
		QGraphicsRectItem *rect, *inner_rect;
		QGraphicsTextItem *value_text, *name_text;
		qreal position, width, value;
	};
	class LineGraph : public Marker
	{
	public:
		LineGraph(TestWidget *test, QString unit, bool net, QColor color);
		void SetSize(int count);	// set graph final value count
		void AddValue(qreal value);	// add new value to graph
		void Reposition();			// repositions lines in screen acordin to new scale and count
		void erase();				// erase all data in graph

	private:
		int size;
		bool shownet;
		QString unit;
		qreal Yscale_cached;
		QColor color;
		QList<QGraphicsLineItem*> lines;
		QList<qreal> values;
		QList<QGraphicsLineItem*> net;
		QList<QGraphicsTextItem*> net_markups;
		QGraphicsRectItem *rect;
	};

	explicit TestWidget(QWidget *parent = 0);
	~TestWidget();

	void SetDevice(Device *device);
	void SetStartEnabled(bool enabled);

	void StartTest();
	void StopTest();

	// test specific functions
	virtual void TestLoop() = 0;
	virtual void InitScene() = 0;
	virtual void UpdateScene() = 0;
	virtual qreal GetProgress() = 0;
	virtual QDomElement WriteResults(QDomDocument &doc) = 0;
	virtual void RestoreResults(QDomElement &root, bool reference = false) = 0;

	// Graph creation functions
	Line* addLine(QString unit, QString name, QColor color);
	Ticks* addTicks(QColor color);
	Bar* addBar(QString unit, QString name, QColor color, qreal position, qreal width);
	LineGraph* addLineGraph(QString unit, bool net, QColor color);

	void Rescale(qreal max, bool force = false);

	Device *device;
	QGraphicsScene *scene;

	qreal Yscale;

	bool running;
	bool go;

	// TODO: remove this
	//int progress;

protected:
	 void resizeEvent(QResizeEvent *event);

private:
	class TestThread : public QThread
	{
	public:
		TestThread(TestWidget *widget);
		void run();
	private:
		TestWidget *widget;
	};

    Ui::TestWidget *ui;

	QTimer refresh_timer;
	TestThread *test_thread;
	QList<Marker*> markers;

private slots:
	void refresh_timer_timeout();
	void on_startstop_clicked();
};

#endif // TESTWIDGET_H

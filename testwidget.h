#ifndef TESTWIDGET_H
#define TESTWIDGET_H

#include <math.h>
#include <QWidget>
#include <QThread>
#include <QGraphicsScene>
#include <QTimer>
#include <QList>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QFileDialog>

#include "device.h"

// Forward declaration od TestThread class
class TestThread;

namespace Ui {
    class TestWidget;
}

class TestWidget : public QWidget
{
    Q_OBJECT

public:
	enum DataSet { RESULTS, REFERENCE };
	enum TestState { STARTING, STARTED, STOPPING, STOPPED };

	//////////////////////////////////////////////////////////////////////////////
	//// Test markers ////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	class Marker
	{
	public:
		Marker(TestWidget *test);
		virtual ~Marker();

		static const qreal NET_WIDTH = 0.88;			// Width of net lines
		static const qreal NET_HIGHLIGHT_WIDTH = 0.89;	// Width of extended net lines

		TestWidget *test;				/// pointer to test containing this marker
		qreal max;						/// maximal value used by this marker
		qreal min;						/// minimal value used by this marker
		virtual void Reposition() = 0;	/// reposition marker on view change
	};

	class Ticks : public Marker
	{
	public:
		Ticks(TestWidget *test, QColor color);
		~Ticks();
		void AddTick(qreal value, qreal position, bool important = true);
		void erase();
		void Reposition();

	private:
		struct Tick
		{
			qreal value;
			qreal position;
			QGraphicsRectItem *tick;
		};

		QColor color;
		QList<Tick> ticks;
	};

	class Line : public Marker
	{
	public:
		Line(TestWidget *test, QString unit, QString name, QColor color);
		~Line();
		void SetValue(qreal value);
		void Reposition();

	private:
		QString unit, name;
		QColor color;
		QGraphicsLineItem *line;
		QGraphicsTextItem *text;
		qreal value;
	};

	class Bar : public Marker
	{
	public:
		Bar(TestWidget *test, QString unit, QString name, QColor color, qreal position, qreal width);
		~Bar();
		void Set(qreal progress, qreal value);
		void Reposition();

	private:
		QString unit, name;
		QColor color;
		QGraphicsRectItem *rect, *inner_rect;
		QGraphicsTextItem *value_text, *name_text;
		qreal position, width, value, progress;
	};

	class LineGraph : public Marker
	{
	public:
		LineGraph(TestWidget *test, QString unit, QColor color);
		~LineGraph();
		void SetSize(int count);	// set graph final value count
		void AddValue(qreal value);	// add new value to graph
		void Reposition();			// repositions lines in screen acording to new scale and count
		void erase();				// erase all data in graph

	private:
		int size;
		QString unit;
		QColor color;
		QList<QGraphicsLineItem*> lines;
		QList<qreal> values;
	};

	class Net : public Marker
	{
	public:
		Net(TestWidget *test, QString unit, QString xAxis, QString yAxis);
		~Net();
		void Reposition();

	private:
		QString unit, xAxis, yAxis;
		QList<QGraphicsLineItem*> net;
		QList<QGraphicsTextItem*> net_markups;
		QGraphicsLineItem *left_line;
		QGraphicsTextItem *xAxisText, *yAxisText;
	};

	class Legend : public Marker
	{
	public:
		Legend(TestWidget *test);
		~Legend();
		void AddItem(QString name, QColor color);
		void Reposition();
	private:
		struct Item
		{
		public:
			QGraphicsTextItem *text;
			QGraphicsRectItem *rect;
		};

		QList<Item> items;
	};

	/////////////////////////////////////////////////////////////////////////
	//// TestWidget class methods ///////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////

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
	virtual int GetProgress() = 0;	///	Report test progress in range from 0 to 100.
	virtual void EraseResults(DataSet dataset) = 0;
	virtual QDomElement WriteResults(QDomDocument &doc) = 0;
	virtual void RestoreResults(QDomElement &root, DataSet dataset) = 0;

	// Marker adding functions
	Line* addLine(QString unit, QString name, QColor color);
	Ticks* addTicks(QColor color);
	Bar* addBar(QString unit, QString name, QColor color, qreal position, qreal width);
	LineGraph* addLineGraph(QString unit, QColor color);
	Net* addNet(QString unit, QString xAxis, QString yAxis);
	Legend* addLegend();

	void Rescale(bool force = false);	/// Rescales scene according new marker values and windows dimensions

	Device *device;				/// pointer to device selecte dfor testing in GUI
	QGraphicsScene *scene;		/// pointer to current grephics scene

	qreal Yscale;				/// Y axis multipiler
	QRect graph;				/// rect in graphics scene occupied by graph

	QString testName;			/// test name
	QString testDescription;	/// test description - used by info box

	TestState testState;		/// Current test state

protected:
	 void resizeEvent(QResizeEvent*);

private:
	Ui::TestWidget *ui;

	QTimer refresh_timer;
	TestThread *test_thread;
	QList<Marker*> markers;		// List of markers used in scene

private slots:
	void refresh_timer_timeout();
	void on_startstop_clicked();
	void on_info_clicked();
	void on_image_clicked();
	void test_started();
	void test_stopped();
};

#endif // TESTWIDGET_H

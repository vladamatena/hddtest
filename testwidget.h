/**
* \author Vladimír Matěna vlada.matena@gmail.com
*/

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

/// Base for all benchmarks - common function and untilities
/** The TestWidget class is base for all specialized benchmark classes.
It handles widget callbacks, graph drawing and benchmark process itself.
It provides several methods for displaying markers and graph parts.
The benchmarks specific classes should use only these methods provided by
TestWidget class to build their graphs. When graph component is created
this way the ThetWidget class deinitializes it corectly in its destructor.
The benchmark classes are intended to extend this class and implement
pure virtual methods to provide benchmark specific functionality.
This class handles benchmark starting/stopping and peridical refreshing
the graph by calling UpdateScene method implemented in benchmark specific class.
The TestWidget class handles GUI parts as Progress bar, start/stop button,
export button, info button and graph area. **/
class TestWidget : public QWidget
{
    Q_OBJECT

public:
	enum DataSet { RESULTS, REFERENCE };
	enum TestState { STARTING, STARTED, STOPPING, STOPPED };

	//////////////////////////////////////////////////////////////////////////////
	//// Test markers
	//////////////////////////////////////////////////////////////////////////////
	/// Marker is common base of all graph parts
	/** Marker class is base to all markers and graph parts used by benchmarks
	to build their GUI. It also serves as common interface to to functions that
	are the same for all markers, such as Reposition calls. The TestWidget class
	rescales graph parts and uses information from min and max fields to calculate
	scale multipiler. The Reposition method reimplemented by markers is called when
	new scale multipiler is calculated. The NET_WIDTH and NET_HIGHLIGHT_WIDTH
	constants are used by all markers as borders of graph area.**/
	/// Marker is common base of all graph parts
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

	/// Dot graph
	/** Ticks class extends Marker class and provides dot graph which is used
	by Seek benchmark. The graph contains dots that can be added by AddTick method
	one by one. Dots are used to display discrete values in 2D graph.
	All dost can be erased by erase method.**/
	class Ticks : public Marker
	{
	public:
		Ticks(TestWidget *test, QColor color);
		~Ticks();

		/** Add one tick/dot to graph
		  @param value tick's vertical position
		  @param position tick's horizontal position **/
		void AddTick(qreal value, qreal position, bool important = true);

		void erase();		/// Erases all ticks in graph
		void Reposition();	/// Resposition ticks according to new scale

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

	/// Horizontal line marker
	/** The Line class extends marker class to simple horizontal line with
	value on the right.**/
	class Line : public Marker
	{
	public:
		Line(TestWidget *test, QString unit, QString name, QColor color);
		~Line();

		/** Set line value and moves
		it to new position according to value.
		@param value new value**/
		void SetValue(qreal value);

		void Reposition();	/// Reposition line to new scale

	private:
		QString unit, name;
		QColor color;
		QGraphicsLineItem *line;
		QGraphicsTextItem *text;
		qreal value;
	};

	/// Bar marker
	/** Bar class implements simple bar in graph.
	The progress and value can be set for the graph.
	The value is displayed on the top of the bar and also
	determines its height. The bar consist just of its border
	when progress is 0. When progress gets higher the bar
	fills with the color from bottom.**/
	class Bar : public Marker
	{
	public:
		Bar(TestWidget *test, QString unit, QString name, QColor color, qreal position, qreal width);
		~Bar();

		/** Set new value and progress to bar
		  @param progress new progress
		  @param value new value **/
		void Set(qreal progress, qreal value);

		void Reposition();	/// Reposition bar according to new scale

	private:
		QString unit, name;
		QColor color;
		QGraphicsRectItem *rect, *inner_rect;
		QGraphicsTextItem *value_text, *name_text;
		qreal position, width, value, progress;
	};

	/// Line graph
	/** Linegraph class extends Marker class to linegraph.
	The linegraph consists of lines connected to a polyline.
	Parts of polyline can be added one by one. The whole graph
	can be erased to initial empty state. A count of line segments
	in the whole graph needs to be set in order to display polyline
	in the correct width. **/
	class LineGraph : public Marker
	{
	public:
		LineGraph(TestWidget *test, QString unit, QColor color);
		~LineGraph();

		/**  Set graph final value count
		  @param count new final line segment count **/
		void SetSize(int count);

		/** Add new value to graph
		  @param value to be added **/
		void AddValue(qreal value);

		void Reposition();			/// Repositions lines in screen acording to new scale and count
		void erase();				/// Erase all data in graph

	private:
		int size;
		QString unit;
		QColor color;
		QList<QGraphicsLineItem*> lines;
		QList<qreal> values;
	};

	/// Net with measure graph
	/** Net class extends marker class to background net
	 with numbers on the side. The net recalculates step of its scale
	 on Yscale update.**/
	class Net : public Marker
	{
	public:
		Net(TestWidget *test, QString unit, QString xAxis, QString yAxis);
		~Net();
		void Reposition();		/// Recalculates and moves net according to new scale

	private:
		QString unit, xAxis, yAxis;
		QList<QGraphicsLineItem*> net;
		QList<QGraphicsTextItem*> net_markups;
		QGraphicsLineItem *left_line;
		QGraphicsTextItem *xAxisText, *yAxisText;
	};

	/// Legend marker
	/** Legend class bases on Marker class and provides
	Simple legend in the top righ corner of the graph area. The legen provides
	mapping from colour to text. The coloured rectangle with description is displayed
	for every item. Items can only be added as they are not expected to change
	while application is running.**/
	class Legend : public Marker
	{
	public:
		Legend(TestWidget *test);
		~Legend();

		/** Add new item to legend
		  @param name legend item description
		  @param color legend item colour **/
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

	/** TestWidget constructor. Called by Qt when GUI is initialized. **/
	explicit TestWidget(QWidget *parent = 0);
	~TestWidget();

	/** Set pointer to device class when new device is selected in GUI.
	  @param device pointer to new device **/
	void SetDevice(Device *device);

	/** Set whenever the benchmark can be started. Causes start button be disabled.
	  @param enabled whenever the benchmarks is enabled **/
	void SetStartEnabled(bool enabled);

	void StartTest();	/// Starts the benchmark
	void StopTest();	/// Cancels benchmark

	// test specific functions
	/** Benchmarking code run in separate thread.
	This method is implemntd by benchmark specific class. **/
	virtual void TestLoop() = 0;

	/** Code to initialze graph before benchmark starts supplied by
	benchmarking class. Maybe this will not be needed as benchmarks can initialize
	this in their constructors. **/
	virtual void InitScene() = 0;

	/** Method implementd by benchmarking class. Called when graph needs refresh.**/
	virtual void UpdateScene() = 0;

	/** Reports test progress in range from 0 to 100.
	This is implemented by benchmark class. The return value is used to
	update progress bar below the graph.**/
	virtual int GetProgress() = 0;

	/** This method is implemented by benchmark specific class.
	It should erase reference or measured resutls.
	  @param dataset which resutls should be erased
	  @see DataSet **/
	virtual void EraseResults(DataSet dataset) = 0;

	/** This method is called when resutls should be saved.
	 The class extending TestWidget should supply code neede to save resutls.
	 @param doc QDomDocument to which results should be saved. **/
	virtual QDomElement WriteResults(QDomDocument &doc) = 0;

	/** Method implemented by benchmark specific class. It should load results from
	XML element.
	 @param root resutls root element
	 @param dataset which results are to be replace **/
	virtual void RestoreResults(QDomElement &root, DataSet dataset) = 0;

	// Marker adding functions
	/** Adds line marker
	  @param unit units to be displayed
	  @param name value desrition
	  @param color line and value colour
	  @return pointer to line marker used to update it **/
	Line* addLine(QString unit, QString name, QColor color);

	/** Add ticks marker to graph
	  @param color colour of the ticks
	  @return pointer to the resulting marker **/
	Ticks* addTicks(QColor color);

	/** Add bar marker
	  @param unit units displyer with value
	  @param name label at the bar buttom
	  @param position of the bar as fraction of graph width
	  @param witdth of the amrker as fraction of graph width
	  @return pointer to marker **/
	Bar* addBar(QString unit, QString name, QColor color, qreal position, qreal width);

	/** Add line graph
	  @param unit name - not used
	  @param color linegraph colour
	  @return pointer to new marker **/
	LineGraph* addLineGraph(QString unit, QColor color);

	/** Add net to graph
	  @param unit descrittion for measure values
	  @param xAxis descrition text
	  @param yAxis descrition text
	  @return pointer to new marker **/
	Net* addNet(QString unit, QString xAxis, QString yAxis);

	/** Add legend to graph. **/
	Legend* addLegend();

	/** Rescales scene according new marker values and windows dimensions
	  @param force rescales even when all values can still be displayed **/
	void Rescale(bool force = false);

	Device *device;				/// Pointer to device selecte dfor testing in GUI
	QGraphicsScene *scene;		/// Pointer to current grephics scene

	qreal Yscale;				/// Y axis multipiler
	QRect graph;				/// Rect in graphics scene occupied by graph

	QString testName;			/// Test name
	QString testDescription;	/// Test description - used by info box

	TestState testState;		/// Current test state

protected:
	 void resizeEvent(QResizeEvent*); /// Rescales graph on resize event

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

/**
* \author Vladimír Matěna vlada.matena@gmail.com
*/

#ifndef TESTTHREAD_H
#define TESTTHREAD_H

#include <QThread>

// Forward declaration of TestWidget
class TestWidget;

/// Runs benchmark in separate thread
/** The TestThread class is used to run a benchmark code in new thread.
  The class in constructed with the TestWidget class pointer.
  The benchmarking method form TestWidget is called in new thread when
  Start is called on this class. The cache flush is done before benchmarking
  code is run. This class also emits the test_started and
  test_stopped signals. **/
class TestThread : public QThread
{
    Q_OBJECT
public:
	/** Construct TestThread with TestWidget instance which defines code to be run.
	  @param widget benchmark class providing the method to be run in the new thread **/
	explicit TestThread(TestWidget *widget);

	void run();	/// Starts the benchmark in new thread
private:
	TestWidget *widget;

signals:
	void test_started(); /// Emited when benchmark is started
	void test_stopped(); /// Emitted when benchmark has finished
public slots:
};

#endif // TESTTHREAD_H

#ifndef TESTTHREAD_H
#define TESTTHREAD_H

#include <QThread>

// Forward declaration of TestWidget
class TestWidget;

class TestThread : public QThread
{
    Q_OBJECT
public:
	explicit TestThread(TestWidget *widget);
	void run();
private:
	TestWidget *widget;

signals:
	void test_started();
	void test_stopped();
public slots:
};

#endif // TESTTHREAD_H

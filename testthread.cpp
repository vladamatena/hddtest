#include "testthread.h"
#include "testwidget.h"

TestThread::TestThread(TestWidget *widget) :
	QThread(widget)
{
	this->widget = widget;
}

void TestThread::run()
{
	// mark test started
	widget->go = true;
	widget->running = true;

	// prepare device for test
	widget->device->Warmup();
	widget->device->DropCaches();
	widget->device->Sync();

	// run test
	test_started();
	widget->TestLoop();
	test_stopped();

	// mark test finished
	widget->running = false;
}

/**
* \author Vladimír Matěna vlada.matena@gmail.com
*/

#include "testthread.h"
#include "testwidget.h"

TestThread::TestThread(TestWidget *widget) :
	QThread(widget)
{
	this->widget = widget;
}

void TestThread::run()
{
	// prepare device for test
	widget->device->Warmup();
	widget->device->DropCaches();
	widget->device->Sync();

	// run test
	test_started();
	widget->TestLoop();
	test_stopped();
}

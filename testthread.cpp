/*******************************************************************************
*
*	HDDTest the graphical drive benchmarking tool.
*	Copyright (C) 2011  Vladimír Matěna <vlada.matena@gmail.com>
*
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
********************************************************************************/

#include "testthread.h"
#include "testwidget.h"

TestThread::TestThread(TestWidget *widget) :
	QThread(widget) {
	this->widget = widget;
}

void TestThread::run() {
	// prepare device for test
	widget->device->Warmup();
	widget->device->DropCaches();
	widget->device->Sync();

	// run test
    emit test_started();
	widget->TestLoop();
    emit test_stopped();
}

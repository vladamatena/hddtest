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

#pragma once

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
class TestThread : public QThread {
    Q_OBJECT
public:
	/** Construct TestThread with TestWidget instance which defines code to be run.
	  @param widget benchmark class providing the method to be run in the new thread **/
	explicit TestThread(TestWidget *widget);

	void run();	/// Starts the benchmark in new thread
private:
	TestWidget *widget;

signals:
	/// Emited when benchmark is started
	void test_started();
	/// Emitted when benchmark has finished
	void test_stopped();
};

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

/** \mainpage HDDTest documentation main page
 *
 * \section basics Basic ideas and solutions
 * \subsection classes Classes
 * Classes in the project copy separated functional parts of the application.
 * There are simple classes for the main application window called HDDTestWidget
 * and random number generator called RandomGenerator. Basic primitives used by benchmarks were
 *  also moved to separated classes for file and device access called File and Device.
 *
 * Every benchmark has its own class which covers both benchmarks GUI and benchmark
 *  process itself. Benchmark classes are based on a class common to all benchmarks.
 *  The class is called TestWidget. It handles GUI elements that are common to all benchmarks,
 *  graph drawing and benchmark state changes.
 *
 * There are more helper classes described in generated Doxygen documentation that are not
 *  important to understand basic principles of the application.
 *
 * \subsection qt Qt
 * Qt framework is used to handle application GUI and everything that Qt libraries can handle
 *  in order to make application depend more on Qt than Linux specific things.
 *  Only raw device access used by benchmarks was done as low-level as possible because it
 *  is not sure how Qt handles it internally.
 *
 * The most interesting part of GUI is graph drawing. All the graphs and measures are drawn by
 *  HDDTest itself. Graph primitives are implemented in TestWidget class and provided to benchmark
 *  classes. Graph drawing is done via QGraphicsView object. QGraphicsView provides drawing
 *  geometrical primitives and fonts QGraphicsView. Graph primitives are based on those
 *  QGraphicsView provides. QGraphicsView also supports scaling and anti aliasing which is used to smooth graphs.
 *
 * \subsection fieleanddevice File and Device classes
 * These two classes provide basic functions needed by benchmarks. Provided functions are
 *  mostly wrappers to file read and write functions. Wrappers do not care about data being transferred.
 *  Instead of this only sizes and times are important. Methods of this classes are used exclusively by
 *  benchmarks and device enumeration in main application window. Even when moving code to benchmark
 *  functions would be possible, the code was left separated.
 *
 * Device and file access and enumeration code was moved to this separated classes in order to
 *  make application ready for supporting multiple platforms in future. Separating this code
 *  could also help when low-level access code needs to be changed because of bugs. Also the
 *  benchmark functions are much more simple when not including time measurement calls.
 *
 * \subsection testwidgetclass TestWidget class
 * TestWidget class implements common code from all benchmarks and graph drawing code.
 * This class has three basic functions. The first one is to provide callback to benchmark GUI elements.
 *  The second one is to start and stop the test in separate thread. The third is to provide graph drawing.
 *
 * The actual benchmarks are classes derived from TestWidget class which implement a few methods
 *  specific to them. When benchmark is started a benchmark specific method containing benchmarking
 *  code is run in separate thread and another benchmark specific method is called periodically
 *  to redraw the results in graph. The graph drawing method uses basic graph parts provided by base
 *  TestWiget class such as Bar graph, Line Graph and more support elements.
 *
 * \subsection benchmarkspecificclasses Benchmark specific classes
 * Every benchmark has its own class that contains its GUI handling and benchmark function.
 *  Benchmark class also defines class that holds results in benchmark specific way. This class is
 *  based on TestWidget class. Benchmark class implements several virtual methods defined by TestWidget.
 *  These methods define benchmark specific behaviour. Every benchmark defines TestLoop method which
 *  contains benchmark code. InitScene, UpdateScene and GetProgress methods that handles GUI specific things.
 *  And WriteResults, RestoreResults and EraseResults to handle results in benchmark specific way.
 *
 * \section building Building
 *
 * \subsection step1 Step 1: # make clean
 * \subsection step2 Step 2: # qmake -Wall CONFIG+=debug hddtest.pro
 * \subsection step3 Step 3: # make
 */

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include<QtCore>

namespace HDDTest {
	class Def;

	typedef qint64 hddtime; /// time interval in microseconds
	typedef qint64 hddsize; /// size on drive in bytes

	// size units
	static const hddsize B = 1;
	static const hddsize K = 1024 * B;
	static const hddsize M = 1024 * K;
	static const hddsize G = 1024 * M;

	// time units
	static const hddtime us = 1;
	static const hddtime ms = 1000 * us;
	static const hddtime s	= 1000 * ms;

	/// Contains human readable value formating functions
	class Def
	{
	public:
		static QString FormatSize(hddsize size);	/// Size to human readable format convertor
		static QString FormatSpeed(hddsize size);	/// Speed to human readable format convertor
	};
}

#endif // DEFINITIONS_H

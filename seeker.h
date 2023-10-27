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
#include <QFile>
#include <QPointF>
#include <QtCore>
#include <QList>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QtXml>

#include "testwidget.h"
#include "device.h"
#include "randomgenerator.h"

/// Seeker benchmark main class
/** Seeker test class. Implements Seeker test. The test test device for ramdom position access.
Attempts to access different random positions on drive are made. Results are shown as dots.
Dots shows dependency of seek time on seek length. **/
class Seeker : public TestWidget {
private:
	/// Keeps information about running or pased seek test
	class SeekResult {
	public:
		SeekResult():
				progress(0) {}

		void erase();					/// Erase all seeks
		void AddSeek(QPointF seek);		/// Add seek to this test
		qreal avg();					/// Get overall average seek time
		qreal average;

		QList<QPointF> seeks;		/// List of seeks in results
		QStack<QPointF> newseeks;	/// List of new seeks (not yet displayed)
		unsigned int progress;		/// Percentage progress of the test
	};
public:
	explicit Seeker(QWidget *parent = 0);
	~Seeker();

	static const hddsize SEEKER_BLOCKSIZE = 512 * B;	/// seek read size
	static const hddsize SEEKER_SEEKCOUNT = 1000;				/// number of seeke per test
	static const int SEEKER_IMPORTANT = 2;						/// seeks slower than N * average are not important

	SeekResult result;		/// Seek results
	SeekResult reference;	/// Seek reference results

	void TestLoop();	/// Main benchmark code
	void InitScene();	/// Initializes scene before benchmark begins
	void UpdateScene();	/// Updates scene
	int GetProgress();	/// Returns benchmark progress

	QDomElement WriteResults(QDomDocument &doc);				/// Writes results of test to XML
	void RestoreResults(QDomElement &root, DataSet dataset);	/// Reads results from XML document
	void EraseResults(DataSet dataset);							/// Erases selected results

private:
	Line *dataAvgLine;
	Line *referenceAvgLine;

	Ticks *dataTicks;
	Ticks *referenceTicks;

	Net* net;
};

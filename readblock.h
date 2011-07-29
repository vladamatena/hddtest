/**
* \author Vladimír Matěna vlada.matena@gmail.com
*/

#ifndef READBLOCK_H
#define READBLOCK_H

#include "testwidget.h"
#include "randomgenerator.h"
#include "device.h"

/// Stores ReadBlock benchmark results
/** Class for keepeg ReadBlock subtest results and progress
@see ReadBlock class **/
class ReadBlockResult
{
public:
	ReadBlockResult(hddsize block_size);

	hddsize __bytes_read;	/// Count of bytes read by selected block size
	hddtime __time_elapsed;	/// Time elased while reading
	hddsize __block_size;	/// Size of the block for this subtest

	void erase(); /// Erase all values
};

/// Read Block benchmark main class
/** Read Block test. The test reads blocks of differsent sizes from the device.
Bar graphs for every block size are drawn to the graph.
@see ReadBlockResults class **/
class ReadBlock : public TestWidget
{
public:
	// ReadRnd class constructor
	ReadBlock(QWidget *parent = 0);	/// The constructor

	static const hddsize READ_BLOCK_SIZE = 100 * M;				/// Data to be read by every block size
	static const hddsize READ_BLOCK_BASE_BLOCK_SIZE = 1 * M;	/// base block size for first substes
	static const int READ_BLOCK_BLOCK_SIZE_COUNT = 12;			/// Subtest count
	static const int READ_BLOCK_BLOCK_SIZE_STEP = 2;			/// Divisior for next subtest

	void TestLoop();	/// Main benchmark code
	void InitScene();	/// Initializes scene before benchmark begins
	void UpdateScene();	/// Updates scene
	int GetProgress();	/// Returns benchmark progress

	// list of subtest results
	QList<ReadBlockResult> results;		/// Primary results
	QList<ReadBlockResult> reference;	/// Reference results

	QDomElement WriteResults(QDomDocument &doc);				/// Writes results of test to XML
	void RestoreResults(QDomElement &root, DataSet dataset);	/// Reads results from XML document
	void EraseResults(DataSet dataset);							/// Erases selected results

private:
	QList<Bar*> bars;
	QList<Bar*> reference_bars;
};

#endif // READBLOCK_H

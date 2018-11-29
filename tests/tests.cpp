#include <QtTest>

#include "coord.h"
#include "field.h"

class CommonTest : public QObject
{
    Q_OBJECT

public:
    CommonTest();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();

    void Coord_initialization_tests();
    void Coord_getters_tests();
    void Coord_operation_tests();

    void Cell_test_candidates();
//    void Cell_test_removeCandidate();

    void benchmark();

private:
};

CommonTest::CommonTest()
{
}

void CommonTest::initTestCase()
{
}

void CommonTest::cleanupTestCase()
{
}

void CommonTest::init()
{
	Coord::init(9);
}

void CommonTest::Coord_initialization_tests()
{
	Coord coord(5,7);
	QVERIFY(coord.isValid());
	
	Coord coord_wrong_row(0,1);
	QVERIFY(!coord_wrong_row.isValid());
	
	Coord coord_wrong_col(1,0);
	QVERIFY(!coord_wrong_col.isValid());
}

void CommonTest::Coord_getters_tests()
{
	Coord coord(5,7);
	QVERIFY(coord.row() == 5);
	QVERIFY(coord.col() == 7);
	QVERIFY(coord.isValid());

	QVERIFY(coord.squareIdx() == 5);
	quint16 rawIdx = coord.rawIndex();
	QVERIFY(rawIdx == 42);
	
	coord.setRowCol(4,6);
	QVERIFY(coord.row() == 4);
	QVERIFY(coord.col() == 6);
	QVERIFY(coord.rawIndex() != rawIdx);
	
	coord.setRawIndex(24);
	QVERIFY(coord.col() == 7);
	QVERIFY(coord.row() == 3);

	QVERIFY(Coord::last().rawIndex() == Coord::maxRawIndex());
}

void CommonTest::Coord_operation_tests()
{
	Coord coord(1,2);
	coord++;
	QVERIFY(coord.row() == 1);
	QVERIFY(coord.col() == 3);
	
	coord.setRowCol(1,9);
	coord++;
	QVERIFY(coord.row() == 2);
	QVERIFY(coord.col() == 1);
	
	coord = Coord::last();
	QVERIFY(coord == Coord::last());
	
	coord++;
	QVERIFY(!coord.isValid());
	
	Coord coordA(1,2);
	Coord coordB(2,1);
	QVERIFY(coordA < coordB);
	QVERIFY(coordB > coordA);
	QVERIFY(coordA != coordB);
	

}

void CommonTest::Cell_test_candidates()
{
    Cell cell;
    QVERIFY(!cell.isValid());

    cell.resetCandidates(9);
    QVERIFY(cell.isValid());

    QVERIFY(cell.hasCandidate(1));
    QVERIFY(cell.hasCandidate(3));
    QVERIFY(cell.hasCandidate(5));
    QVERIFY(cell.hasCandidate(7));
    QVERIFY(cell.hasCandidate(9));
    QVERIFY_EXCEPTION_THROWN(cell.hasCandidate(0), std::out_of_range);
    QVERIFY_EXCEPTION_THROWN(cell.hasCandidate(10), std::out_of_range);

    QBitArray evenBits(9);
    evenBits.setBit(1);
    evenBits.setBit(3);
    evenBits.setBit(5);
    evenBits.setBit(7);
    QVERIFY(cell.hasAnyOfCandidates(evenBits));

    QVERIFY(!cell.isResolved());
}

void CommonTest::benchmark()
{
	Field array;
	array.readFromPlainTextFile("puzzle/learningcurve.sdm", 0);
	bool isResolved = false;
	bool isValid = false;
	QBENCHMARK{
		array.process();
	}
	isValid = array.isValid();
	isResolved = array.isResolved();
	QVERIFY(isValid);
	QVERIFY(isResolved);
}

QTEST_APPLESS_MAIN(CommonTest)

#include "tests.moc"

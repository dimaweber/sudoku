#include <QtTest>

#include "coord.h"
#include "field.h"
#include "resolver.h"
#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
#   include <QRandomGenerator>
#endif

class CommonTest : public QObject
{
    Q_OBJECT

public:
    CommonTest();

private slots:

    // Low-level api tests
    void initTestCase();
    void cleanupTestCase();
    void init();

    void Coord_initialization_tests();
    void Coord_getters_tests();
    void Coord_operation_tests();
    void Coord_same_house_tests();

    void Cell_test_candidates();
//    void Cell_test_removeCandidate();

    // Low-level technique tests (1 iteration)
    void naked_group_test();

    // Hi-level techniques tests (solve whole puzzle)
    void xyzwing_test();
    void ywing_test();
    void unique_rectangle_tests();
    void coloring_test();

    // Benchmarks
    void benchmark9x9();
    void benchmark16x16();

private:
    template<class TECH>
    void lowLevelTechniqueTest(QString filename, int num, const std::list<std::pair<Coord, std::list<CellValue>>>& list)
    {
        Field field;
        QVERIFY(field.readFromPlainTextFile(filename, num));
        TECH tech(field);

        for(auto c: list)
            for (auto a: c.second)
                QCOMPARE(field.cell(c.first)->hasCandidate(a), true);

        while (tech.perform());

        for(auto c: list)
            for (auto a: c.second)
                QCOMPARE(field.cell(c.first)->hasCandidate(a), false);
    }
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

void CommonTest::Coord_same_house_tests()
{
    Coord c;

    c.setRowCol(1,1);

    QVector<Coord> sameHouse = c.sameColumnCoordinates();
    for(quint8 i=1;i<=9;i++)
        if (i!=c.row())
            QVERIFY(sameHouse.contains(Coord(i,c.col())));
        else
            QVERIFY(!sameHouse.contains(Coord(i, c.col())));

    sameHouse = c.sameRowCoordinates();
    for(quint8 i=1;i<=9;i++)
        if (i!=c.col())
            QVERIFY(sameHouse.contains(Coord(c.row(), i)));
        else
            QVERIFY(!sameHouse.contains(Coord(c.row(), i)));

}

void CommonTest::Cell_test_candidates()
{
    std::unique_ptr<Cell> cell (new Cell());
    QVERIFY(!cell->isValid());

    cell->resetCandidates(9);
    QVERIFY(cell->isValid());

    QVERIFY(cell->hasCandidate(1));
    QVERIFY(cell->hasCandidate(3));
    QVERIFY(cell->hasCandidate(5));
    QVERIFY(cell->hasCandidate(7));
    QVERIFY(cell->hasCandidate(9));
    QVERIFY_EXCEPTION_THROWN(cell->hasCandidate(0),  std::out_of_range);
    QVERIFY_EXCEPTION_THROWN(cell->hasCandidate(10), std::out_of_range);

    QBitArray evenBits(9);
    evenBits.setBit(1);
    evenBits.setBit(3);
    evenBits.setBit(5);
    evenBits.setBit(7);
    QVERIFY(cell->hasAnyOfCandidates(evenBits));

    QVERIFY(!cell->isResolved());
}

void CommonTest::naked_group_test()
{
    std::list<std::pair<Coord, std::list<CellValue>>> checks;

    // Naked pair 1 test
    checks.push_back({{1,4},{1}});
    checks.push_back({{1,5},{1, 6}});
    checks.push_back({{1,6},{6}});
    checks.push_back({{3,1},{1, 7}});
    checks.push_back({{3,5},{7, 6}});
    lowLevelTechniqueTest<NakedGroupTechnique>("../puzzle/naked_group.sdm", 0, checks);

    // Naked pair 2 test
    checks.clear();
    checks.push_back({{1,4},{7}});
    checks.push_back({{2,4},{7}});
    checks.push_back({{2,6},{1,2}});
    checks.push_back({{3,4},{7}});
    checks.push_back({{8,3},{7}});
    checks.push_back({{9,3},{7}});
    checks.push_back({{8,5},{3,7}});
    lowLevelTechniqueTest<NakedGroupTechnique>("../puzzle/naked_group.sdm", 1, checks);

    // Naked triple 1 test
    checks.clear();
    checks.push_back({{5,1},{5, 9}});
    checks.push_back({{5,3},{5, 9}});
    checks.push_back({{5,7},{5, 8, 9}});
    checks.push_back({{5,8},{5, 8, 9}});
    lowLevelTechniqueTest<NakedGroupTechnique>("../puzzle/naked_group.sdm", 2, checks);

    // Naked triple 2 test
    checks.clear();
    checks.push_back({{4,2}, {1,8}});
    checks.push_back({{4,3}, {1,8}});
    checks.push_back({{4,7}, {8}});
    checks.push_back({{4,8}, {2,8}});
    checks.push_back({{5,3}, {1,5}});
    checks.push_back({{6,2}, {1,5,8}});
    checks.push_back({{6,3}, {1,5,8}});
    checks.push_back({{6,7}, {8}});
    checks.push_back({{6,8}, {2,8}});
    lowLevelTechniqueTest<NakedGroupTechnique>("../puzzle/naked_group.sdm", 3, checks);

    // Naked quad test
    checks.clear();
    checks.push_back({{1,2}, {1,5}});
    checks.push_back({{1,3}, {5}});
    checks.push_back({{2,3}, {5, 6, 8}});
    checks.push_back({{2,3}, {5, 6, 8}});
    checks.push_back({{3,3}, {6}});
    lowLevelTechniqueTest<NakedGroupTechnique>("../puzzle/naked_group.sdm", 4, checks);
}

void CommonTest::benchmark16x16()
{
    Field array16x16;
    QVERIFY(array16x16.readFromPlainTextFile("../puzzle/16x16.sdm", 1));

    Resolver resolver16x16(array16x16, nullptr);
    resolver16x16.registerTechnique<NakedSingleTechnique>();
    resolver16x16.registerTechnique<HiddenSingleTechnique>();
    resolver16x16.registerTechnique<NakedGroupTechnique>();
    resolver16x16.registerTechnique<HiddenGroupTechnique>();
    resolver16x16.registerTechnique<IntersectionsTechnique>();
    resolver16x16.registerTechnique<BiLocationColoringTechnique>();
    resolver16x16.registerTechnique<XWingTechnique>();
    resolver16x16.registerTechnique<YWingTechnique>();
    resolver16x16.registerTechnique<XYZWingTechnique>();
    resolver16x16.registerTechnique<UniqueRectangle>();


    QBENCHMARK {
        resolver16x16.process();
    }

    QVERIFY(array16x16.isValid());
    QVERIFY(array16x16.isResolved());

}

void CommonTest::xyzwing_test()
{
    Field array9x9;
    QVERIFY(array9x9.readFromPlainTextFile("../puzzle/xyz-wing.sdm", 1));

    Resolver resolver9x9(array9x9, nullptr);
    resolver9x9.registerTechnique<NakedSingleTechnique>();
    resolver9x9.registerTechnique<HiddenSingleTechnique>();
    resolver9x9.registerTechnique<NakedGroupTechnique>();
    resolver9x9.registerTechnique<HiddenGroupTechnique>();
    resolver9x9.registerTechnique<IntersectionsTechnique>()->setEnabled(false);
    resolver9x9.registerTechnique<BiLocationColoringTechnique>()->setEnabled(false);
    resolver9x9.registerTechnique<XWingTechnique>()->setEnabled(false);
    resolver9x9.registerTechnique<YWingTechnique>()->setEnabled(false);
    resolver9x9.registerTechnique<XYZWingTechnique>();

    resolver9x9.process();

    QVERIFY(array9x9.isValid());
    QVERIFY(array9x9.isResolved());
}

void CommonTest::ywing_test()
{
    Field array9x9;
    QVERIFY(array9x9.readFromPlainTextFile("../puzzle/ywing.sdm", 1));

    Resolver resolver9x9(array9x9, nullptr);
    resolver9x9.registerTechnique<NakedSingleTechnique>();
    resolver9x9.registerTechnique<HiddenSingleTechnique>();
    resolver9x9.registerTechnique<NakedGroupTechnique>();
    resolver9x9.registerTechnique<HiddenGroupTechnique>();
    resolver9x9.registerTechnique<IntersectionsTechnique>()     ->  setEnabled(false);
    resolver9x9.registerTechnique<BiLocationColoringTechnique>()->  setEnabled(false);
    resolver9x9.registerTechnique<XWingTechnique>()             ->  setEnabled(false);
    resolver9x9.registerTechnique<YWingTechnique>();
    resolver9x9.registerTechnique<XYZWingTechnique>()           ->  setEnabled(false);

    resolver9x9.process();

    QVERIFY(array9x9.isValid());
    QVERIFY(array9x9.isResolved());
}

void CommonTest::unique_rectangle_tests()
{
    Field array9x9;
    QVERIFY(array9x9.readFromPlainTextFile("../puzzle/unique_rectangle_type1.sdm", 1));

    Resolver resolver9x9(array9x9, nullptr);
    resolver9x9.registerTechnique<NakedSingleTechnique>();
    resolver9x9.registerTechnique<HiddenSingleTechnique>();
    resolver9x9.registerTechnique<NakedGroupTechnique>();
    resolver9x9.registerTechnique<HiddenGroupTechnique>();
    resolver9x9.registerTechnique<IntersectionsTechnique>();
    resolver9x9.registerTechnique<BiLocationColoringTechnique>()    ->  setEnabled(false);
    resolver9x9.registerTechnique<XWingTechnique>()                 ->  setEnabled(false);
    resolver9x9.registerTechnique<YWingTechnique>();
    resolver9x9.registerTechnique<XYZWingTechnique>()               ->  setEnabled(false);
    resolver9x9.registerTechnique<UniqueRectangle>();

    resolver9x9.process();
    QVERIFY(array9x9.isValid());
    QVERIFY(array9x9.isResolved());

    QVERIFY(array9x9.readFromPlainTextFile("../puzzle/unique_rectangle_type2.sdm", 1));
    resolver9x9.process();
    QVERIFY(array9x9.isValid());
    QVERIFY(array9x9.isResolved());

    QVERIFY(array9x9.readFromPlainTextFile("../puzzle/unique_rectangle_type3.sdm", 1));
    resolver9x9.process();
    QVERIFY(array9x9.isValid());
    QVERIFY(array9x9.isResolved());
}

void CommonTest::coloring_test()
{
    Field array9x9;
    QVERIFY(array9x9.readFromPlainTextFile("../puzzle/coloring.sdm", 0));

    Resolver resolver9x9(array9x9, nullptr);
    resolver9x9.registerTechnique<NakedSingleTechnique>();
    resolver9x9.registerTechnique<HiddenSingleTechnique>();
    resolver9x9.registerTechnique<NakedGroupTechnique>();
    resolver9x9.registerTechnique<HiddenGroupTechnique>();
    resolver9x9.registerTechnique<IntersectionsTechnique>();
    resolver9x9.registerTechnique<BiLocationColoringTechnique>();
    resolver9x9.registerTechnique<XWingTechnique>()->setEnabled(false);
    resolver9x9.registerTechnique<YWingTechnique>()->setEnabled(false);
    resolver9x9.registerTechnique<XYZWingTechnique>()->setEnabled(false);
    resolver9x9.registerTechnique<UniqueRectangle>()->setEnabled(false);

    resolver9x9.process();
    QVERIFY(array9x9.isValid());
    QVERIFY(array9x9.isResolved());
}

void CommonTest::benchmark9x9()
{
    Field array9x9;
    QVERIFY(array9x9.readFromPlainTextFile("../puzzle/learningcurve.sdm", 1));

    bool isResolved = false;
    bool isValid = false;

    Resolver resolver9x9(array9x9, nullptr);
    resolver9x9.registerTechnique<NakedSingleTechnique>();
    resolver9x9.registerTechnique<HiddenSingleTechnique>();
    resolver9x9.registerTechnique<NakedGroupTechnique>();
    resolver9x9.registerTechnique<HiddenGroupTechnique>();
    resolver9x9.registerTechnique<IntersectionsTechnique>();
    resolver9x9.registerTechnique<BiLocationColoringTechnique>();
    resolver9x9.registerTechnique<XWingTechnique>();
    resolver9x9.registerTechnique<YWingTechnique>();
    resolver9x9.registerTechnique<XYZWingTechnique>();
    resolver9x9.registerTechnique<UniqueRectangle>();

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    QRandomGenerator rng(25121981);
#else
    qsrand(25121981);
#endif

    QBENCHMARK{
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        int idx = rng() % 2000 + 1;
#else
        int idx = qrand() % 2000 + 1;
#endif
        idx = 1;
        QVERIFY(array9x9.readFromPlainTextFile("../puzzle/learningcurve.sdm", idx));
        resolver9x9.process();
    }

    isValid = array9x9.isValid();
    isResolved = array9x9.isResolved();
    QVERIFY(isValid);
    QVERIFY(isResolved);
}

QTEST_MAIN(CommonTest)

#include "tests.moc"

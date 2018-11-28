#include <QtTest>


class CellTest : public QObject
{
    Q_OBJECT

public:
    CellTest();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void hmac_sha512_test();
private:
};

CellTest::CellTest()
{
}

void CellTest::initTestCase()
{
}

void CellTest::cleanupTestCase()
{
}

void CellTest::init()
{
}

void CellTest::cleanup()
{
}

void CellTest::hmac_sha512_test()
{
	QVERIFY(1==1);
}


//QTEST_APPLESS_MAIN(CellTest)

#include "test_cell.moc"

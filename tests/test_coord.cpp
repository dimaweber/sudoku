#include <QtTest>


class CoordTest : public QObject
{
    Q_OBJECT

public:
    CoordTest();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void hmac_sha512_test();
private:
};

CoordTest::CoordTest()
{
}

void CoordTest::initTestCase()
{
}

void CoordTest::cleanupTestCase()
{
}

void CoordTest::init()
{
}

void CoordTest::cleanup()
{
}

void CoordTest::hmac_sha512_test()
{
	QVERIFY(1==1);
}


//QTEST_APPLESS_MAIN(CoordTest)

#include "test_coord.moc"

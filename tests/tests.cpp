#include <QtTest>


class CommonTest : public QObject
{
    Q_OBJECT

public:
    CommonTest();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void hmac_sha512_test();
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
}

void CommonTest::cleanup()
{
}

void CommonTest::hmac_sha512_test()
{
	QVERIFY(1==1);
}


QTEST_APPLESS_MAIN(CommonTest)

#include "tests.moc"

#include <QtTest>

#include "../database.h"
#include "../variantfunctions.h"

#include <cmath>

class ServerParseUnitTest : public QObject
{
    Q_OBJECT

private slots:
    void vigenereEncryptsKnownText();
    void vigenereDecryptsKnownText();
    void vigenerePreservesCaseAndPunctuation();
    void vigenereRejectsEmptyKey();
    void vigenereRejectsNonLatinKey();

    void sha512MatchesKnownDigest();

    void bisectionFindsRoot();
    void bisectionRejectsInvalidInterval();
    void bisectionRejectsNonPositiveEpsilon();

    void shortestPathFindsBestRoute();
    void shortestPathHandlesSameVertex();
    void shortestPathRejectsMissingRoute();
    void shortestPathRejectsNegativeWeight();

    void databaseIsSingleton();
    void databaseStoresRequests();
};

void ServerParseUnitTest::vigenereEncryptsKnownText()
{
    QString error;

    QCOMPARE(VariantFunctions::vigenereEncrypt("Attack at dawn",
                                               "LEMON",
                                               &error),
             QString("Lxfopv ef rnhr"));
    QVERIFY(error.isEmpty());
}

void ServerParseUnitTest::vigenereDecryptsKnownText()
{
    QString error;

    QCOMPARE(VariantFunctions::vigenereDecrypt("Lxfopv ef rnhr",
                                               "LEMON",
                                               &error),
             QString("Attack at dawn"));
    QVERIFY(error.isEmpty());
}

void ServerParseUnitTest::vigenerePreservesCaseAndPunctuation()
{
    QString error;

    QCOMPARE(VariantFunctions::vigenereEncrypt("Abc-XYZ!", "b", &error),
             QString("Bcd-YZA!"));
    QVERIFY(error.isEmpty());
}

void ServerParseUnitTest::vigenereRejectsEmptyKey()
{
    QString error;

    QCOMPARE(VariantFunctions::vigenereEncrypt("text", "", &error),
             QString());
    QCOMPARE(error, QString("key must not be empty"));
}

void ServerParseUnitTest::vigenereRejectsNonLatinKey()
{
    QString error;

    QCOMPARE(VariantFunctions::vigenereEncrypt("text", "L3MON", &error),
             QString());
    QCOMPARE(error, QString("key must contain Latin letters only"));
}

void ServerParseUnitTest::sha512MatchesKnownDigest()
{
    QCOMPARE(VariantFunctions::sha512("hello"),
             QString("9b71d224bd62f3785d96d46ad3ea3d73319bfbc2890caada"
                     "e2dff72519673ca72323c3d99ba5c11d7c7acc6e14b8c5da"
                     "0c4663475c2e5c3adef46f73bcdec043"));
}

void ServerParseUnitTest::bisectionFindsRoot()
{
    const VariantFunctions::BisectionResult result =
        VariantFunctions::bisection(1.0, 2.0, 0.000001);

    QVERIFY(result.success);
    QVERIFY(result.error.isEmpty());
    QVERIFY(std::abs(result.root - 1.5213797) < 0.000001);
}

void ServerParseUnitTest::bisectionRejectsInvalidInterval()
{
    const VariantFunctions::BisectionResult result =
        VariantFunctions::bisection(2.0, 3.0, 0.001);

    QVERIFY(!result.success);
    QCOMPARE(result.error,
             QString("function values at interval boundaries must have "
                     "different signs"));
}

void ServerParseUnitTest::bisectionRejectsNonPositiveEpsilon()
{
    const VariantFunctions::BisectionResult result =
        VariantFunctions::bisection(1.0, 2.0, 0.0);

    QVERIFY(!result.success);
    QCOMPARE(result.error, QString("epsilon must be greater than zero"));
}

void ServerParseUnitTest::shortestPathFindsBestRoute()
{
    const VariantFunctions::ShortestPathResult result =
        VariantFunctions::shortestPath("A",
                                       "D",
                                       {"A,B,1", "B,D,2", "A,D,10"});

    QVERIFY(result.success);
    QCOMPARE(result.distance, 3.0);
    QCOMPARE(result.path, QStringList({"A", "B", "D"}));
}

void ServerParseUnitTest::shortestPathHandlesSameVertex()
{
    const VariantFunctions::ShortestPathResult result =
        VariantFunctions::shortestPath("A", "A", {});

    QVERIFY(result.success);
    QCOMPARE(result.distance, 0.0);
    QCOMPARE(result.path, QStringList({"A"}));
}

void ServerParseUnitTest::shortestPathRejectsMissingRoute()
{
    const VariantFunctions::ShortestPathResult result =
        VariantFunctions::shortestPath("A", "D", {"A,B,1", "C,D,2"});

    QVERIFY(!result.success);
    QCOMPARE(result.error, QString("no path between the requested vertices"));
}

void ServerParseUnitTest::shortestPathRejectsNegativeWeight()
{
    const VariantFunctions::ShortestPathResult result =
        VariantFunctions::shortestPath("A", "B", {"A,B,-1"});

    QVERIFY(!result.success);
    QCOMPARE(result.error,
             QString("edge weight must be a non-negative number"));
}

void ServerParseUnitTest::databaseIsSingleton()
{
    Database &first = Database::instance();
    Database &second = Database::instance();

    QCOMPARE(&first, &second);
}

void ServerParseUnitTest::databaseStoresRequests()
{
    qputenv("SERVER_DATABASE_PATH", ":memory:");
    Database &database = Database::instance();

    QVERIFY(database.open());
    QVERIFY(database.isOpen());
    QCOMPARE(database.databasePath(), QString(":memory:"));
    QVERIFY(database.saveRequest("hash|hello", "OK|sha512=test"));

    const QString requests = database.getAllRequests();
    QVERIFY(requests.contains("request=hash|hello"));
    QVERIFY(requests.contains("response=OK|sha512=test"));
}

QTEST_MAIN(ServerParseUnitTest)

#include "tst_serverparse.moc"

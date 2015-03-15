#include <QtTest/QtTest>
#include <QtCore/QtCore>

#include "qts3_tst_p.h"
#include "qts3_p.h"

class TestQtS3 : public QObject
{
    Q_OBJECT
private slots:
    // helpers
    void dateTime();

    // signing key creation
    void deriveSigningKey();
    void checkGenerateSigningKey();

    // authorization header creation
    void formatQueryString();
    void formatCanonicalRequest();
    void formatStringToSign();
    void signStringToSign();
    void signRequestData();
    void formatAuthorizationHeader();
    void createAuthorizationHeader();

    // QNetworkRequest creation and signing
    void createAndSignRequest();

    // Integration tests that require netowork access
    // and access to a test bucket on S3.
    void bucketLocation();
};

// test date and time formatting
void TestQtS3::dateTime()
{
    QDate date(AwsTestData::timeStamp.date());
    QCOMPARE(QtS3Private::formatDate(date), AwsTestData::date);
    QDateTime dateTime(AwsTestData::timeStamp);
    QCOMPARE(QtS3Private::formatDateTime(dateTime), AwsTestData::dateTime);
}

// test S3 signing key derivation
void TestQtS3::deriveSigningKey()
{
    QByteArray signingKey = QtS3Private::deriveSigningKey(
        AwsTestData::secretAccessKey, AwsTestData::date, AwsTestData::region, AwsTestData::service);
    QCOMPARE(signingKey.toHex(), AwsTestData::signingKey);
}

// signing key (re-) generation. Verify that the key is generated only when needed
void TestQtS3::checkGenerateSigningKey()
{
    QByteArray key;
    QDateTime keyTime;
    QDateTime t0 = QDateTime(QDate(2000, 1, 1), QTime(0, 00));
    QDateTime t1 = QDateTime(QDate(2000, 1, 1), QTime(0, 30));    // + 30s
    QDateTime t2 = QDateTime(QDate(2000, 1, 2), QTime(0, 30));    // + 1 day: regenerates
    QDateTime t3 = QDateTime(QDate(9999, 12, 30), QTime(23, 59)); // + many years: regenerates
    QDateTime t4 = QDateTime(QDate(4000, 12, 30), QTime(23, 59)); // negative: regenerates

    QVERIFY(QtS3Private::checkGenerateSigningKey(&key, &keyTime, t0, AwsTestData::secretAccessKey,
                                                 AwsTestData::region, AwsTestData::service));
    QVERIFY(!QtS3Private::checkGenerateSigningKey(&key, &keyTime, t0, AwsTestData::secretAccessKey,
                                                  AwsTestData::region, AwsTestData::service));
    QVERIFY(!QtS3Private::checkGenerateSigningKey(&key, &keyTime, t1, AwsTestData::secretAccessKey,
                                                  AwsTestData::region, AwsTestData::service));
    QVERIFY(QtS3Private::checkGenerateSigningKey(&key, &keyTime, t2, AwsTestData::secretAccessKey,
                                                 AwsTestData::region, AwsTestData::service));
    QVERIFY(QtS3Private::checkGenerateSigningKey(&key, &keyTime, t3, AwsTestData::secretAccessKey,
                                                 AwsTestData::region, AwsTestData::service));
    QVERIFY(QtS3Private::checkGenerateSigningKey(&key, &keyTime, t4, AwsTestData::secretAccessKey,
                                                 AwsTestData::region, AwsTestData::service));
}

// test canonicalizing a query string.
void TestQtS3::formatQueryString()
{
    QByteArray canonicalQueryString =
        QtS3Private::createCanonicalQueryString(AwsTestData::inputQueryString);
    QCOMPARE(canonicalQueryString, AwsTestData::canonicalQueryString);
}

// test creating a canonical request + hash from request components
void TestQtS3::formatCanonicalRequest()
{
    // payload hashing
    QCOMPARE(QtS3Private::hash(AwsTestData::content).toHex(), AwsTestData::contentHash);

    // canonical request construction
    QByteArray canonicalRequest = QtS3Private::formatCanonicalRequest(
        AwsTestData::method, AwsTestData::url, AwsTestData::queryString, AwsTestData::headers,
        AwsTestData::contentHash);
    QCOMPARE(canonicalRequest, AwsTestData::canonicalRequest);

    // canonical request hashing
    QCOMPARE(QtS3Private::hash(canonicalRequest).toHex(), AwsTestData::canonicalRequestHash);
}

// test formatting the "string to sign"
void TestQtS3::formatStringToSign()
{
    QByteArray stringToSign =
        QtS3Private::formatStringToSign(AwsTestData::timeStamp, AwsTestData::region,
                                        AwsTestData::service, AwsTestData::canonicalRequestHash);
    QCOMPARE(stringToSign, AwsTestData::stringToSign);
}

// test signing the "string to sign"
void TestQtS3::signStringToSign()
{
    QByteArray signature =
        QtS3Private::sign(QByteArray::fromHex(AwsTestData::signingKey), AwsTestData::stringToSign);
    QCOMPARE(signature.toHex(), AwsTestData::signature);
}

// test signing the request data
void TestQtS3::signRequestData()
{
    QByteArray signature = QtS3Private::signRequestData(
        AwsTestData::headers, AwsTestData::method, AwsTestData::url, QByteArray(),
        AwsTestData::content, QByteArray::fromHex(AwsTestData::signingKey), AwsTestData::timeStamp,
        AwsTestData::region, AwsTestData::service);
    QCOMPARE(signature.toHex(), AwsTestData::signature);
}

// test formatting the authorization header
void TestQtS3::formatAuthorizationHeader()
{
    QByteArray authHeaderValue = QtS3Private::formatAuthorizationHeader(
        AwsTestData::accessKeyId, AwsTestData::timeStamp, AwsTestData::region, AwsTestData::service,
        AwsTestData::signedHeaders, AwsTestData::signature);
    QCOMPARE(authHeaderValue, AwsTestData::authorizationHeaderValue);
}

// test creating the authorization header
void TestQtS3::createAuthorizationHeader()
{
    QByteArray authHeaderValue = QtS3Private::createAuthorizationHeader(
        AwsTestData::headers, AwsTestData::method, AwsTestData::url, QByteArray(),
        AwsTestData::content, AwsTestData::accessKeyId,
        QByteArray::fromHex(AwsTestData::signingKey), AwsTestData::timeStamp, AwsTestData::region,
        AwsTestData::service);
    QCOMPARE(authHeaderValue, AwsTestData::authorizationHeaderValue);
}

// test creating an signing a QNetworkRequest with QtS3Private
void TestQtS3::createAndSignRequest()
{
    // create request
    QNetworkRequest request;
    QtS3Private::setRequestAttributes(&request, QUrl(AwsTestData::url), AwsTestData::headers,
                                      AwsTestData::timeStamp, AwsTestData::host);

    //
    // QCOMPARE(QtS3Private::requestHeaders(&request), AwsTestData::headers);

    // sign request
    QtS3Private::signRequest(&request, AwsTestData::method, AwsTestData::content,
                             AwsTestData::accessKeyId, QByteArray::fromHex(AwsTestData::signingKey),
                             AwsTestData::timeStamp, AwsTestData::region, AwsTestData::service);
    QByteArray authorizationHeaderName = "Authorization";
    QVERIFY(request.rawHeaderList().contains(authorizationHeaderName));

    // QCOMPARE(request.rawHeader(authorizationHeaderName), AwsTestData::authorizationHeaderValue);
}

void TestQtS3::bucketLocation()
{
    QByteArray awsKeyId = qgetenv("AWS_S3_ACCESS_KEY_ID");
    QByteArray awsSecretKey = qgetenv("AWS_S3_SECRET_ACCESS_KEY");
    if (awsKeyId.isEmpty())
        QSKIP("AWS_S3_ACCESS_KEY_ID not set. This tests requires S3 access.");
    if (awsSecretKey.isEmpty())
        QSKIP("AWS_S3_SECRET_ACCESS_KEY not set. This tests requires S3 access.");

    QtS3Private s3(awsKeyId, awsSecretKey);
    QByteArray usBucketLocation = s3.location("qtestbucket-us");
    qDebug() << usBucketLocation;
}

QTEST_MAIN(TestQtS3)

#include "qts3_tst.moc"

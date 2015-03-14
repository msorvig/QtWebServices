#include <QtTest/QtTest>
#include <QtCore/QtCore>

#include "qts3_p.h"

class TestQtS3: public QObject
{
    Q_OBJECT
private slots:
    // helpers
    void dateTime();

    // signing key creation
    void deriveSigningKey();
    void checkGenerateSigningKey();

    // authorization header creation
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

//  Test data from:
//  http://docs.aws.amazon.com/general/latest/gr/sigv4-calculate-signature.html
//  http://docs.aws.amazon.com/general/latest/gr/sigv4-create-canonical-request.html
namespace AwsTestData
{
    // first, a consistent test data set, where the hashes and
    // signatures matches the input data.
    static const QDateTime timeStamp(QDate(2011,9,9), QTime(23,36,0));
    static const QByteArray accessKeyId = "AKIDEXAMPLE";
    static const QByteArray secretAccessKey = "wJalrXUtnFEMI/K7MDENG+bPxRfiCYEXAMPLEKEY";
    static const QByteArray date = "20110909";
    static const QByteArray dateTime = "20110909T233600Z";
    static const QByteArray region = "us-east-1";
    static const QByteArray host = "iam.amazonaws.com";
    static const QByteArray service = "iam";
    static const QByteArray signingKey = "98f1d889fec4f4421adc522bab0ce1f82e6929c262ed15e5a94c90efd1e3b0e7";
    static const QByteArray method = "POST";
    static const QByteArray url = "/";
    static const QByteArray queryString = "";
    static const QByteArray content = "Action=ListUsers&Version=2010-05-08";
    static const QByteArray contentHash = "b6359072c78d70ebee1e81adcbab4f01bf2c23245fa365ef83fe8f1f955085e2";
    static const QByteArray canonicalRequest = ""
        "POST\n"
        "/\n"
        "\n"
        "content-type:application/x-www-form-urlencoded; charset=utf-8\n"
        "host:iam.amazonaws.com\n"
        "x-amz-date:20110909T233600Z\n"
        "\n"
        "content-type;host;x-amz-date\n"
        "b6359072c78d70ebee1e81adcbab4f01bf2c23245fa365ef83fe8f1f955085e2";
    static const QByteArray canonicalRequestHash = "3511de7e95d28ecd39e9513b642aee07e54f4941150d8df8bf94b328ef7e55e2";
    static const QByteArray stringToSign =
            "AWS4-HMAC-SHA256\n"
            "20110909T233600Z\n"
            "20110909/us-east-1/iam/aws4_request\n"
            "3511de7e95d28ecd39e9513b642aee07e54f4941150d8df8bf94b328ef7e55e2";
    static const QHash<QByteArray, QByteArray> headers = {{"Host", "iam.amazonaws.com"},
                                                          {"Content-Type", "application/x-www-form-urlencoded; charset=utf-8"},
                                                          {"X-Amz-Date", "20110909T233600Z"}};
    static const QByteArray signedHeaders = "content-type;host;x-amz-date";
    static const QByteArray signature = "ced6826de92d2bdeed8f846f0bf508e8559e98e4b0199114b84c54174deb456c";
    static const QByteArray authorizationHeaderValue =
        "AWS4-HMAC-SHA256 Credential=AKIDEXAMPLE/20110909/us-east-1/iam/aws4_request, "
        "SignedHeaders=content-type;host;x-amz-date, "
        "Signature=ced6826de92d2bdeed8f846f0bf508e8559e98e4b0199114b84c54174deb456c";

    // extra test data not part of the consistent data set
    static const QByteArray canonicalQueryString =
        "Action=ListUsers&"
        "Version=2010-05-08&"
        "X-Amz-Algorithm=AWS4-HMAC-SHA256&"
        "X-Amz-Credential=AKIAIOSFODNN7EXAMPLE%2F20110909%2Fus-east-1%2Fiam%2Faws4_request&"
        "X-Amz-Date=20110909T233600Z&"
        "X-Amz-SignedHeaders=content-type%3Bhost%3Bx-amz-date";
    static const QByteArray inputQueryString =
        "X-Amz-Algorithm=AWS4-HMAC-SHA256&"
        "X-Amz-Credential=AKIAIOSFODNN7EXAMPLE%2F20110909/us-east-1/iam/aws4_request&"
        "X-Amz-Date=20110909T233600Z&"
        "X-Amz-SignedHeaders=content-type;host;x-amz-date&"
        "Action=ListUsers&"
        "Version=2010-05-08";
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
    QDateTime t0 = QDateTime(QDate(2000,1,1), QTime(0,00));
    QDateTime t1 = QDateTime(QDate(2000,1,1), QTime(0,30)); // + 30s
    QDateTime t2 = QDateTime(QDate(2000,1,2), QTime(0,30)); // + 1 day: regenerates
    QDateTime t3 = QDateTime(QDate(9999,12,30), QTime(23,59)); // + many years: regenerates
    QDateTime t4 = QDateTime(QDate(4000,12,30), QTime(23,59)); // negative: regenerates

    QVERIFY(QtS3Private::checkGenerateSigningKey(&key, &keyTime, t0, AwsTestData::secretAccessKey, AwsTestData::region, AwsTestData::service));
    QVERIFY(!QtS3Private::checkGenerateSigningKey(&key, &keyTime, t0, AwsTestData::secretAccessKey, AwsTestData::region, AwsTestData::service));
    QVERIFY(!QtS3Private::checkGenerateSigningKey(&key, &keyTime, t1, AwsTestData::secretAccessKey, AwsTestData::region, AwsTestData::service));
    QVERIFY(QtS3Private::checkGenerateSigningKey(&key, &keyTime, t2, AwsTestData::secretAccessKey, AwsTestData::region, AwsTestData::service));
    QVERIFY(QtS3Private::checkGenerateSigningKey(&key, &keyTime, t3, AwsTestData::secretAccessKey, AwsTestData::region, AwsTestData::service));
    QVERIFY(QtS3Private::checkGenerateSigningKey(&key, &keyTime, t4, AwsTestData::secretAccessKey, AwsTestData::region, AwsTestData::service));
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
    QByteArray stringToSign = QtS3Private::formatStringToSign(
        AwsTestData::timeStamp, AwsTestData::region, AwsTestData::service,
        AwsTestData::canonicalRequestHash);
    QCOMPARE(stringToSign, AwsTestData::stringToSign);
}

// test signing the "string to sign"
void TestQtS3::signStringToSign()
{
    QByteArray signature = QtS3Private::sign(
        QByteArray::fromHex(AwsTestData::signingKey), AwsTestData::stringToSign);
    QCOMPARE(signature.toHex(), AwsTestData::signature);
}

// test signing the request data
void TestQtS3::signRequestData()
{
    QByteArray signature = QtS3Private::signRequestData(
        AwsTestData::headers,AwsTestData::method, AwsTestData::url, QByteArray(),
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
        AwsTestData::content, AwsTestData::accessKeyId, QByteArray::fromHex(AwsTestData::signingKey),
        AwsTestData::timeStamp, AwsTestData::region, AwsTestData::service);
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

    //QCOMPARE(request.rawHeader(authorizationHeaderName), AwsTestData::authorizationHeaderValue);
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

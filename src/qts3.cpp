#include "qts3.h"

#include <QtCore>

#include "qts3_p.h"


//    The life and times of an AWS request. Data flow.
//
//                        AWS Secret key ->
//                                Date   -> Signing
//                               Region  ->   Key    ---------------|
//                               Service ->                         |
//                                                                  |
//        Headers: Host, X-Amz-Date                                 |  DateTime
//  APP                 |                                           |  Date/region/service
//   |                  |                                           S    |
//    Request  -> Canonical Request -> CanonicalRequest Hash -> String to Sign -> Request Signature
//        |                                                             |             |
//    Request                                                           |             |
//   with Auth  <---------------Athentication Header-----------------------------------
//     header                             ^
//        |                          AWS Key Id
//    THE INTERNET

//
//  Control flow
//  checkGenerateSigningKey
//      deriveSigningKey
//
//  createAuthorizationHeader
//      signRequestData
//          formatCanonicalRequest
//          formatStringToSign
//      formatAuthorizationHeader
//
//
//    Implementation notes.
//
//    We implement general utility functions for signing QNetworkAccessManager AWS requsts,
//    whith a completing S3 API on top.
//
//    Function types:
//    - format*
//      formats one or more data items as expected by AWS, with minimal processing. Returns a
//        QByteArray
//
//

QtS3Private::QtS3Private() : m_networkAccessManager(0) {}

QtS3Private::QtS3Private(QByteArray accessKeyId, QByteArray secretAccessKey)
    : m_accessKeyId(accessKeyId), m_secretAccessKey(secretAccessKey),
      m_networkAccessManager(new QNetworkAccessManager)
{
    init();
}

//  Returns a date formatted as YYYYMMDD.
QByteArray QtS3Private::formatDate(const QDate &date)
{
    return date.toString(QStringLiteral("yyyyMMdd")).toLatin1();
}

//  Returns a date formatted as YYYYMMDDTHHMMSSZ
QByteArray QtS3Private::formatDateTime(const QDateTime &dateTime)
{
    // Both Qt and AWS support ISO 8601 dates. However, Qt produces
    // a datetime formatted like YYYY-MM-DDTHH:MM:SSZ, while AWS expects
    // YYYYMMDDTHHMMSSZ.
    return dateTime.toString(QStringLiteral("yyyyMMddThhmmssZ")).toLatin1();
}

//  SHA256.
QByteArray QtS3Private::hash(const QByteArray &data)
{
    return QCryptographicHash::hash(data, QCryptographicHash::Sha256);
}

//  HMAC_SHA256.
QByteArray QtS3Private::sign(const QByteArray &key, const QByteArray &data)
{
    return QMessageAuthenticationCode::hash(data, key, QCryptographicHash::Sha256);
}

// Canonicalizes a list of http headers.
//    * sorted on header key (using QMap)
//    * whitespace trimmed.
QMap<QByteArray, QByteArray> QtS3Private::canonicalHeaders(const QHash<QByteArray, QByteArray> &headers)
{
    QMap<QByteArray, QByteArray> canonical;

    for (auto it = headers.begin(); it != headers.end(); ++it)
        canonical[it.key().toLower()] = it.value().trimmed();
    return canonical;
}

// Creates newline-separated list of headers on the "name:values" form
QByteArray QtS3Private::formatHeaderNameValueList(const QMap<QByteArray, QByteArray> &headers)
{
    QByteArray nameValues;
    for (auto it = headers.begin(); it != headers.end(); ++it)
        nameValues += it.key() + ":" + it.value() + "\n";
    return nameValues;
}

// Creates a semicolon-separated list of header names
QByteArray QtS3Private::formatHeaderNameList(const QMap<QByteArray, QByteArray> &headers)
{
    QByteArray names;
    for (auto it = headers.begin(); it != headers.end(); ++it)
        names += it.key() + ";";
    names.chop(1); // remove final ";"
    return names;
}

//  Derives an AWS version 4 signing key. \a secretAccessKey is the aws secrect key,
//  \a dateString is a YYYYMMDD date. The signing key is valid for a limited number of days
//  (currently 7). \a region is the bucket region, for example "us-east-1". \a service the
//  aws service ("s3", ...)
QByteArray QtS3Private::deriveSigningKey(const QByteArray &secretAccessKey,
                                         const QByteArray dateString, const QByteArray &region,
                                         const QByteArray &service)
{
    return sign(sign(sign(sign("AWS4" + secretAccessKey, dateString), region), service), "aws4_request");
}

// Generates a new AWS signing key when required. This will typically happen
// on the first call or when the key expires. QtS3 expires the key after one
// day, well before the (current) AWS 7-day expiry period. The key is tied
// to the bucket region and the s3 service. Returns whether the a key was
// created.
bool QtS3Private::checkGenerateSigningKey(QByteArray *currentKey, QDateTime *currentKeyTimestamp,
                                             const QDateTime &now,
                                             const QByteArray &secretAccessKey,
                                             const QByteArray &region, const QByteArray &service)
{
    const int secondsInDay = 60 * 60 * 24;
    const qint64 keyAge = currentKeyTimestamp->secsTo(now);

    if (!currentKey->isEmpty() && currentKeyTimestamp->isValid()
        && (keyAge >= 0 && keyAge < secondsInDay))
        return false; // Key OK, not recreated.

    *currentKey = deriveSigningKey(secretAccessKey, formatDate(now.date()), region, service);
    *currentKeyTimestamp = now;
    return true;
}

QByteArray QtS3Private::formatStringToSign(const QDateTime &timeStamp, const QByteArray &region,
                                        const QByteArray &service,
                                        const QByteArray &canonicalRequestHash)
{
    QByteArray string = "AWS4-HMAC-SHA256\n" + formatDateTime(timeStamp) + "\n"
                        + formatDate(timeStamp.date()) + "/" + region + "/" + service
                        + "/aws4_request\n" + canonicalRequestHash;
    return string;
}

QByteArray QtS3Private::formatAuthorizationHeader(const QByteArray &awsAccessKeyId, const QDateTime &timeStamp, const QByteArray &region,
                                               const QByteArray &service, const QByteArray &signedHeaders,
                                               const QByteArray &signature)
{
    QByteArray headerValue =
                        "AWS4-HMAC-SHA256 "
                        "Credential=" + awsAccessKeyId + "/"
                      + formatDate(timeStamp.date()) + "/" + region + "/" + service
                      + "/aws4_request, " +
                      + "SignedHeaders=" + signedHeaders + ", "
                      + "Signature=" + signature;
    return headerValue;
}

// Copies the request headers form a QNetworkRequest
QHash<QByteArray, QByteArray> QtS3Private::requestHeaders(const QNetworkRequest *request)
{
    QHash<QByteArray, QByteArray> headers;
    for (const QByteArray &header : request->rawHeaderList()) {
        headers.insert(header, request->rawHeader(header));
    }
    return headers;
}

QHash<QByteArray, QByteArray> QtS3Private::parseHeaderList(const QStringList &headers)
{
    QHash<QByteArray, QByteArray> parsedHeaders;
    foreach (const QString &header, headers) {
        QStringList parts = header.split(":");
        parsedHeaders.insert(parts.at(0).toLatin1(), parts.at(1).toLatin1());
    }
    return parsedHeaders;
}

// Creates a canonical request string (example):
//     POST
//     /
//
//     content-type:application/x-www-form-urlencoded; charset=utf-8\n
//     host:iam.amazonaws.com\n
//     x-amz-date:20110909T233600Z\n
//
//     content-type;host;x-amz-date\n
//     b6359072c78d70ebee1e81adcbab4f01bf2c23245fa365ef83fe8f1f955085e2
QByteArray QtS3Private::formatCanonicalRequest(const QByteArray &method, const QByteArray &url,
                                         const QByteArray &queryString,
                                         const QHash<QByteArray, QByteArray> &headers,
                                         const QByteArray &payloadHash)
{
    const auto canon = canonicalHeaders(headers);

    QByteArray request;
    request += method;
    request += "\n";
    request += url;
    request += "\n";
    request += queryString;
    request += "\n";
    request += formatHeaderNameValueList(canon);
    request += "\n";
    request += formatHeaderNameList(canon);
    request += "\n";
    request += payloadHash;
    return request;
}

QByteArray QtS3Private::signRequestData(const QHash<QByteArray, QByteArray> headers,
                                        const QByteArray &verb, const QByteArray &url,
                                        const QByteArray &queryString, const QByteArray &payload,
                                        const QByteArray &signingKey, const QDateTime &dateTime,
                                        const QByteArray &region, const QByteArray &service)
{
    // create canonical request representation and hash
    QByteArray payloadHash = hash(payload).toHex();
    QByteArray canonoicalRequest
        = formatCanonicalRequest(verb, url, queryString, headers, payloadHash);
    QByteArray canonialRequestHash = hash(canonoicalRequest).toHex();

    // create (and sign) stringToSign
    QByteArray stringToSign = formatStringToSign(dateTime, region, service, canonialRequestHash);
    QByteArray signature = sign(signingKey, stringToSign);
    return signature;
}

QByteArray QtS3Private::createAuthorizationHeader(
    const QHash<QByteArray, QByteArray> headers, const QByteArray &verb, const QByteArray &url,
    const QByteArray &queryString, const QByteArray &payload, const QByteArray accessKeyId,
    const QByteArray &signingKey, const QDateTime &dateTime, const QByteArray &region,
    const QByteArray &service)
{
    // sign request
    QByteArray signature = signRequestData(headers, verb, url, queryString, payload, signingKey,
                                           dateTime, region, service);

    // crate Authorization header;
    QByteArray headerNames = formatHeaderNameList(canonicalHeaders(headers));
    return formatAuthorizationHeader(accessKeyId, dateTime, region, service, headerNames, signature.toHex());
}

void QtS3Private::setRequestAttributes(QNetworkRequest *request, const QUrl &url,
                                       const QHash<QByteArray, QByteArray> &headers,
                                       const QDateTime &timeStamp, const QByteArray &host)
{
    // Build request from user input
    request->setUrl(url);
    for (auto it = headers.begin(); it != headers.end(); ++it)
        request->setRawHeader(it.key(), it.value());

    // Add standard AWS headers
    request->setRawHeader("User-Agent", "Qt");
    request->setRawHeader("Host", host);
    request->setRawHeader("X-Amz-Date", formatDateTime(timeStamp));
}

// Signs an aws request by adding an authorization header
void QtS3Private::signRequest(QNetworkRequest *request, const QByteArray &verb,
                                 const QByteArray &payload, const QByteArray accessKeyId, const QByteArray &signingKey,
                                 const QDateTime &dateTime, const QByteArray &region,
                                 const QByteArray &service)
{

    QByteArray payloadHash = hash(payload).toHex();
    request->setRawHeader("x-amz-content-sha256", payloadHash);

    // get headers from request
    QHash<QByteArray, QByteArray> headers = requestHeaders(request);
    QUrl url = request->url();
    // create authorization header (value)
    QByteArray authHeaderValue
        = createAuthorizationHeader(headers, verb, url.path().toLatin1(), url.query().toLatin1(),
                                    payload, accessKeyId, signingKey, dateTime, region, service);
    qDebug() << authHeaderValue;

    // add authorization header to request
    request->setRawHeader("Authorization", authHeaderValue);
}

//
// Stateful non-static functons below
//

void QtS3Private::init()
{
    if (m_accessKeyId.isEmpty()) {
        qWarning() << "access key id not set";
    }

    if (m_secretAccessKey.isEmpty()) {
        qWarning() << "secret access key not set";
    }

    m_region = "us-east-1";
    m_service = "s3";

    m_networkAccessManager = new QNetworkAccessManager();
}

void QtS3Private::checkGenerateS3SigningKey()
{
    QDateTime now = QDateTime::currentDateTimeUtc();
    // lock
    checkGenerateSigningKey(&currents3SigningKey, &s3SigningKeyTimeStamp, now, m_secretAccessKey,
                            m_region, m_service);
    // std::tuple keyTimeCopy { currents3SigningKey, s3SigningKeyTimeStamp };
    // return keyTimeCopy;
}

QNetworkRequest *QtS3Private::createSignedRequest(const QByteArray &verb, const QUrl &url,
                                                  const QHash<QByteArray, QByteArray> &headers,
                                                  const QByteArray &host, const QByteArray &payload)
{
    checkGenerateS3SigningKey();
    QDateTime requestTime = QDateTime::currentDateTimeUtc();

    // Create and sign request
    QNetworkRequest *request = new QNetworkRequest();
    setRequestAttributes(request, url, headers, requestTime, host);
    signRequest(request, verb, payload, m_accessKeyId, currents3SigningKey, requestTime, m_region,
                m_service);
    return request;
}

namespace
{
// The read buffer for the current in-flight request. The synchronous API guarantees
// that there will be only one reuest in progress per thread at any time.
__thread QBuffer *m_inFlightBuffer = 0;
}

QNetworkReply *QtS3Private::sendRequest(const QByteArray &verb, const QNetworkRequest &request,
                                        const QByteArray &payload)
{

    m_inFlightBuffer = 0;
    if (!payload.isEmpty()) {
        m_inFlightBuffer = new QBuffer(const_cast<QByteArray *>(&payload));
        m_inFlightBuffer->open(QIODevice::ReadOnly);
    }

    // Send request
    QNetworkReply *reply
        = m_networkAccessManager->sendCustomRequest(request, verb, m_inFlightBuffer);
    return reply;
}

QNetworkReply *QtS3Private::sendS3Request(const QByteArray &bucketName, const QByteArray &verb,
                                          const QString &path, const QByteArray &queryString,
                                          const QByteArray &content, const QStringList &headers)
{
    const QByteArray host = bucketName + ".s3.amazonaws.com";
    const QByteArray url = "https://" + host + path.toLatin1() + "?" + queryString;

    QHash<QByteArray, QByteArray> hashHeaders = parseHeaderList(headers);

    QNetworkRequest *request = createSignedRequest(verb, QUrl(url), hashHeaders, host, content);
    return sendRequest(verb, *request, content);
}

void QtS3Private::waitForFinished(QNetworkReply *reply)
{
    // QNetworkReply does not implement waitForReadyRead(),
    // this seems to be the best way to wait.
    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    delete m_inFlightBuffer;
    m_inFlightBuffer = 0;
}

QByteArray QtS3Private::location(const QByteArray &bucketName)
{
    if (bucketName.isEmpty()) {
        qWarning() << "QtS3: Bucket name is empty";
        return QByteArray();
    }

    QNetworkReply *reply
        = sendS3Request(bucketName, "GET", "/", "?location", QByteArray(), QStringList());
    waitForFinished(reply);
    QByteArray replyContent = reply->readAll();
    delete reply;

    return replyContent;
}

bool QtS3Private::put(const QByteArray &bucketName, const QString &path, const QByteArray &content,
                      const QStringList &headers)
{
    if (bucketName.isEmpty()) {
        qWarning() << "QtS3: Bucket name is empty";
        return false;
    }

    QNetworkReply *reply = sendS3Request(bucketName, "PUT", path, QByteArray(), content, headers);
    waitForFinished(reply);
    QByteArray replyContent = reply->readAll();
    delete reply;

    return true;
}

bool QtS3Private::exists(const QByteArray &bucketName, const QString &path)
{
    if (bucketName.isEmpty()) {
        qWarning() << "QtS3: Bucket name is empty";
        return false;
    }

    QNetworkReply *reply
        = sendS3Request(bucketName, "HEAD", path, QByteArray(), QByteArray(), QStringList());
    waitForFinished(reply);
    QByteArray replyContent = reply->readAll();
    delete reply;

    bool e = false;
    return false;
}

QtS3Optional<QByteArray> QtS3Private::get(const QByteArray &bucketName, const QString &path)
{
    if (bucketName.isEmpty()) {
        qWarning() << "QtS3: Bucket name is empty";
        return QtS3Optional<QByteArray>();
    }

    QNetworkReply *reply
        = sendS3Request(bucketName, "GET", path, QByteArray(), QByteArray(), QStringList());
    waitForFinished(reply);
    QByteArray replyContent = reply->readAll();
    delete reply;

    return QtS3Optional<QByteArray>(replyContent);
}

bool QtS3Private::get(QByteArray *destination, const QByteArray &bucketName, const QString &path)
{
    if (bucketName.isEmpty()) {
        qWarning() << "QtS3: Bucket name is empty";
        return false;
    }

    QNetworkReply *reply
        = sendS3Request(bucketName, "GET", path, QByteArray(), QByteArray(), QStringList());
    waitForFinished(reply);
    reply->read(destination->data(), destination->size());
    delete reply;

    return false;
}

//
// QtS3 public class implementation
//

QtS3::QtS3(const QString &accessKeyId, const QString &secretAccessKey) : d(new QtS3Private)
{
    d->m_accessKeyId = accessKeyId.toLatin1();
    d->m_secretAccessKey = secretAccessKey.toLatin1();

    d->init();
}

QtS3::~QtS3() { delete d; }

bool QtS3::put(const QByteArray &bucketName, const QString &path, const QByteArray &content,
               const QStringList &headers)
{
    return d->put(bucketName, path, content, headers);
}

bool QtS3::exists(const QByteArray &bucketName, const QString &path)
{
    return d->exists(bucketName, path);
}

QtS3Optional<QByteArray> QtS3::get(const QByteArray &bucketName, const QString &path)
{
    return d->get(bucketName, path);
}

bool QtS3::get(QByteArray *destination, const QByteArray &bucketName, const QString &path)
{
    return d->get(destination, bucketName, path);
}


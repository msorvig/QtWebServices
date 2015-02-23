#include "qts3.h"

#include <QtCore>

// use libaws?
//#define USE_LIBAWS 1
#ifdef USE_LIBAWS

#include <sstream>
#include <aws.h>
using namespace aws;

class QtS3Private
{
public:
    int errorState;
    QString errorString;

    QString m_accessKeyId;
    QString m_secretAccessKey;

    QByteArray awsSigningKey(const QByteArray &secretAccessKey);
    QByteArray awsSignature(const QByteArray &secretAccessKey);
};

QtS3::QtS3(const QString &accessKeyId, const QString &secretAccessKey)
    :d(new QtS3Private)
{
    d->errorState = 0;
    d->m_accessKeyId = accessKeyId;
    d->m_secretAccessKey = secretAccessKey;

    if (d->m_accessKeyId.isEmpty()) {
        d->errorState = 1;
        d->errorString = "access key id not set";
    }

    if (d->m_secretAccessKey.isEmpty()) {
        d->errorState = 1;
        d->errorString += "/secret access key not set";
    }
}

QtS3::~QtS3()
{
    delete d;
}

int QtS3::errorCode()
{
    return d->errorState;
}

QString QtS3::errorString()
{
    return d->errorString;
}

void QtS3::clearErrorState()
{
    d->errorState = 0;
    d->errorString.clear();
}

int QtS3::put(const QString &bucketName, const QString &path, const QByteArray &content, const QStringList &headers)
{
    if (d->errorState)
        return d->errorState;

    if (bucketName.isEmpty()) {
        d->errorState = 1;
        d->errorString = QStringLiteral("Bucket name is empty");
    }

    aws::S3ConnectionPtr lS3Rest;

    try {
        AWSConnectionFactory* lFactory = AWSConnectionFactory::getInstance();
        lS3Rest = lFactory->createS3Connection(d->m_accessKeyId.toStdString(), d->m_secretAccessKey.toStdString());

        // Check for existing bucket?
        //ListAllBucketsResponsePtr lListBucket = lS3Rest->listBucket(bucketName.toStdString());

        // Create bucket. Will succeed if the bucket name is available or we already own the bucket
        // Will fail if the bucket name is taken (the bucket namespace is shared among all users)
//        CreateBucketResponsePtr lCreateBucket = lS3Rest->createBucket(bucketName.toStdString());
        //std::cout << "Bucket created successfully" << std::endl;
        //std::cout << "  Location: " << lCreateBucket->getLocation() << std::endl;
        //std::cout << "  Date: " << lCreateBucket->getDate() << std::endl;
        //if (!lCreateBucket->getETag().empty())
        //  std::cout << "  ETag: " << lCreateBucket->getETag() << std::endl;
        //std::cout << "  x-amz-id-2: " << lCreateBucket->getAmazonId() << std::endl;
        //std::cout << "  x-amz-request-id: " << lCreateBucket->getRequestId() << std::endl;

        // Parse headers
        std::string contentType;
        std::map<std::string, std::string> headerMap;
        foreach (const QString &header, headers) {
            QStringList parts = header.split(":");
            if (parts.count() != 2) {
                d->errorString = QStringLiteral("QtS3::put headers parse error: format should be\"Key:Value\"");
                return 1;
            }

            // Special case for content type due to libaws API.
            if (parts.at(0).contains("Content-Type")) {
                contentType = parts.at(1).toStdString();
            } else {
                headerMap.insert(std::make_pair(parts.at(0).toStdString(), parts.at(1).toStdString()));
            }
        }

        std::stringstream stream;
        stream.rdbuf()->pubsetbuf(const_cast<char *>(content.data()), content.length());
        PutResponsePtr lPut = lS3Rest->put(bucketName.toStdString(), path.toStdString(), stream, contentType, &headerMap);
    } catch (aws::AWSConnectionException& e) {
        d->errorString = QString::fromStdString(e.what());
        d->errorState = 1;
    } catch (aws::S3Exception& awse) {
        d->errorString = QString::fromStdString(awse.what());
        d->errorState = awse.getErrorCode();
    } catch (...) {
        d->errorString = "Some error";
        d->errorState = 1;
    }

    return d->errorState;
}

int QtS3::exists(const QString &bucketName, const QString &path)
{
    aws::S3ConnectionPtr lS3Rest;
    int code = 0;
    try {
        AWSConnectionFactory* lFactory = AWSConnectionFactory::getInstance();
        lS3Rest = lFactory->createS3Connection(d->m_accessKeyId.toStdString(), d->m_secretAccessKey.toStdString());
        lS3Rest->head(bucketName.toStdString(), path.toStdString());
        return code;
    } catch (aws::AWSConnectionException& e) {
        d->errorString = QString::fromStdString(e.what());
        d->errorState = 1;
    } catch (aws::S3Exception& awse) {
        d->errorString = QString::fromStdString(awse.what());
        code = awse.getErrorCode();
    } catch (...) {
        d->errorString = "Some error";
        d->errorState = 1;
    }
    return code;
}

QtS3Optional<QByteArray> QtS3::get(const QString &bucketName, const QString &path)
{
    QByteArray contents;
    aws::S3ConnectionPtr lS3Rest;
    int code = 0;
    try {
        AWSConnectionFactory *lFactory = AWSConnectionFactory::getInstance();
        lS3Rest = lFactory->createS3Connection(d->m_accessKeyId.toStdString(),
                                               d->m_secretAccessKey.toStdString());
        lS3Rest->get(bucketName.toStdString(), path.toStdString());
        return QtS3Optional<QByteArray>(contents);
    } catch (aws::AWSConnectionException &e) {
        d->errorString = QString::fromStdString(e.what());
        d->errorState = 1;
    } catch (aws::S3Exception &awse) {
        d->errorString = QString::fromStdString(awse.what());
        code = awse.getErrorCode();
    } catch (...) {
        d->errorString = "Some error";
        d->errorState = 1;
    }
    return QtS3Optional<QByteArray>(contents);
}

#else

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
    request += method + "\n";
    request += url + "\n";
    request += queryString + "\n";
    request += formatHeaderNameValueList(canon);
    request += "\n";
    request += formatHeaderNameList(canon);
    request += "\n";
    request += payloadHash;
    return request;
}

QByteArray QtS3Private::signRequestData(const QHash<QByteArray, QByteArray> headers, const QByteArray &verb, const QByteArray &url,
                                            const QByteArray &payload, const QByteArray &signingKey,
                                            const QDateTime &dateTime, const QByteArray &region,
                                            const QByteArray &service)
{
    // create canonical request representation and hash
    QByteArray payloadHash = hash(payload).toHex();
    QByteArray canonoicalRequest = formatCanonicalRequest(verb, url, QByteArray(), headers, payloadHash);
    QByteArray canonialRequestHash = hash(canonoicalRequest).toHex();

    // create (and sign) stringToSign
    QByteArray stringToSign = formatStringToSign(dateTime, region, service, canonialRequestHash);
    QByteArray signature = sign(signingKey, stringToSign);
}

QByteArray QtS3Private::createAuthorizationHeader(const QHash<QByteArray, QByteArray> headers, const QByteArray &verb, const QByteArray &url,
                                                 const QByteArray &payload, const QByteArray accessKeyId, const QByteArray &signingKey,
                                                 const QDateTime &dateTime, const QByteArray &region,
                                                 const QByteArray &service)
{
    // sign request
    QByteArray signature = signRequestData(headers, verb, url, payload, signingKey, dateTime, region, service);

    // crate Authorization header;
    QByteArray headerNames = formatHeaderNameList(canonicalHeaders(headers));
    return formatAuthorizationHeader(accessKeyId, dateTime, region, service, headerNames, signature.toHex());
}

QNetworkRequest *QtS3Private::createRequest(const QUrl &url,
                                            const QHash<QByteArray, QByteArray> &headers,
                                            const QDateTime &timeStamp, const QByteArray &host)
{
    // Build request from user input
    QNetworkRequest *request = new QNetworkRequest();
    request->setUrl(url);
    for (auto it = headers.begin(); it != headers.end(); ++it)
        request->setRawHeader(it.key(), it.value());

    // Add standard AWS headers
    request->setRawHeader("Host", host);
    request->setRawHeader("X-Amz-Date", formatDateTime(timeStamp));

    return request;
}

// Signs an aws request by adding an authorization header
void QtS3Private::signRequest(QNetworkRequest *request, const QByteArray &verb,
                                 const QByteArray &payload, const QByteArray accessKeyId, const QByteArray &signingKey,
                                 const QDateTime &dateTime, const QByteArray &region,
                                 const QByteArray &service)
{
    // get headers from request
    QHash<QByteArray, QByteArray> headers = requestHeaders(request);
    QByteArray url = request->url().toString().toLatin1();

    // create authorization header (value)
    QByteArray authHeaderValue = createAuthorizationHeader(headers, verb, url, payload, accessKeyId,
                                                           signingKey, dateTime, region, service);
    // add authorization header to request
    request->setRawHeader("Authorization", authHeaderValue);
}

void QtS3Private::checkGenerateS3SigningKey()
{
    QDateTime now = QDateTime::currentDateTimeUtc();
    // lock
    checkGenerateSigningKey(&currents3SigningKey, &s3SigningKeyTimeStamp, now, m_secretAccessKey,
                               region, service);
    // std::tuple keyTimeCopy { currents3SigningKey, s3SigningKeyTimeStamp };
    // return keyTimeCopy;
}

QNetworkRequest *QtS3Private::createSignedRequest(const QByteArray &verb, const QUrl &url,
                                              const QHash<QByteArray, QByteArray> &headers,
                                              const QByteArray &payload)
{
    checkGenerateS3SigningKey();
    QDateTime requestTime = QDateTime::currentDateTimeUtc();

    // Create and sign request
    QNetworkRequest *request = createRequest(url, headers, requestTime, host);
    signRequest(request, verb, payload, m_accessKeyId, currents3SigningKey, requestTime, region, service);
    return request;
}

void QtS3Private::sendRequest(const QByteArray &verb, QNetworkRequest *request,
                              const QByteArray &payload)
{
    // Send requiest
    // QNetworkAccessManager::sendCustomRequest();

    // Wait for reply
}

// http://docs.aws.amazon.com/AmazonS3/latest/API/sig-v4-authenticating-requests.html
// http://stackoverflow.com/questions/27091816/retrieve-buckets-objects-without-knowing-buckets-region-with-aws-s3-rest-api
// http://docs.aws.amazon.com/general/latest/gr/sigv4-calculate-signature.html
// http://docs.aws.amazon.com/general/latest/gr/sigv4_signing.html

QtS3::QtS3(const QString &accessKeyId, const QString &secretAccessKey) : d(new QtS3Private)
{
    d->errorState = 0;
#ifdef USE_LIBAWS
    d->m_accessKeyId = accessKeyId;
    d->m_secretAccessKey = secretAccessKey;
#else
    d->m_accessKeyId = accessKeyId.toLatin1();
    d->m_secretAccessKey = secretAccessKey.toLatin1();
#endif
    if (d->m_accessKeyId.isEmpty()) {
        d->errorState = 1;
        d->errorString = "access key id not set";
    }

    if (d->m_secretAccessKey.isEmpty()) {
        d->errorState = 1;
        d->errorString += "/secret access key not set";
    }
}

QtS3::~QtS3() { delete d; }

int QtS3::errorCode() { return d->errorState; }

QString QtS3::errorString() { return d->errorString; }

void QtS3::clearErrorState()
{
    d->errorState = 0;
    d->errorString.clear();
}

int QtS3::put(const QString &bucketName, const QString &path, const QByteArray &content,
              const QStringList &headers)
{
    if (bucketName.isEmpty()) {
        qWarning() << "Bucket name is empty";
    }

    return 0;
}

int QtS3::exists(const QString &bucketName, const QString &path)
{
    int code = 0;
    return code;
}

QtS3Optional<QByteArray> QtS3::get(const QString &bucketName, const QString &path)
{
    QByteArray contents;
    return QtS3Optional<QByteArray>(contents);
}

#endif

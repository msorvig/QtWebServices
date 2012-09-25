#include "qts3.h"

#include <QtCore>

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
    aws::S3ConnectionPtr lS3Rest;
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

    try {
        AWSConnectionFactory* lFactory = AWSConnectionFactory::getInstance();
        if (d->lS3Rest.isNull()) {
            d->lS3Rest = lFactory->createS3Connection(d->m_accessKeyId.toStdString(), d->m_secretAccessKey.toStdString());
        }

        // Check for existing bucket?
        //ListAllBucketsResponsePtr lListBucket = d->lS3Rest->listBucket(bucketName.toStdString());

        // Create bucket. Will succeed if the bucket name is available or we already own the bucket
        // Will fail if the bucket name is taken (the bucket namespace is shared among all users)
        CreateBucketResponsePtr lCreateBucket = d->lS3Rest->createBucket(bucketName.toStdString());
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
        PutResponsePtr lPut = d->lS3Rest->put(bucketName.toStdString(), path.toStdString(), stream, contentType, &headerMap);
    } catch (aws::AWSConnectionException& e) {
        d->errorString = QString::fromStdString(e.what());
        d->errorState = 1;
    } catch (aws::AWSException& awse) {
        d->errorString = QString::fromStdString(awse.what());
        d->errorState = 1;
    } catch (...) {
        d->errorString = "Some error";
        d->errorState = 1;
    }

    return d->errorState;
}

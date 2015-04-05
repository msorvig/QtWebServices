#ifndef QTS3_H
#define QTS3_H

#include <QtCore>
#include <QtNetwork>

class QtS3Private;
class QtS3;
class QtS3ReplyPrivate;

class QtS3ReplyBase
{
public:
    enum S3Error {
        NoError,
        NetworkError,
        CredentialsError,
        BucketNameInvalidError,
        BucketNotFoundError,
        ObjectNameInvalidError,
        ObjectNotFoundError,
        GenereicS3Error,
        InternalSignatureError,
        InternalReplyInitializationError,
        InternalError,
        UnknownError,
    };

    QtS3ReplyBase(QtS3ReplyPrivate *replyPrivate);

    // error handling
    bool isSuccess();
    QNetworkReply::NetworkError networkError();
    QString networkErrorString();
    S3Error s3Error();
    QString s3ErrorString();
    QString anyErrorString();

    // verbatim reply as returned by AWS
    QByteArray replyData();

protected:
    QtS3ReplyPrivate *d; // ### should be explicitly shared.
};

template <typename T> class QtS3Reply : public QtS3ReplyBase
{
public:
    QtS3Reply(QtS3ReplyPrivate *replyPrivate);
    T value();
};

template <typename T>
QtS3Reply<T>::QtS3Reply(QtS3ReplyPrivate *replyPrivate)
    : QtS3ReplyBase(replyPrivate)
{
}

class QtS3
{
public:
    QtS3(const QString &accessKeyId, const QString &secretAccessKey);
    ~QtS3();

    QtS3Reply<QByteArray> location(const QByteArray &bucketName);
    QtS3Reply<void> put(const QByteArray &bucketName, const QString &path,
                        const QByteArray &content, const QStringList &headers);
    QtS3Reply<bool> exists(const QByteArray &bucketName, const QString &path);
    QtS3Reply<int> size(const QByteArray &bucketName, const QString &path);
    QtS3Reply<QByteArray> get(const QByteArray &bucketName, const QString &path);

private:
    QSharedPointer<QtS3Private> d;
};

#endif

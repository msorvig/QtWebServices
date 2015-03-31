#include "qts3.h"
#include "qts3_p.h"

QtS3ReplyBase::QtS3ReplyBase(QtS3ReplyPrivate *replyPrivate) : d(replyPrivate) {}
// error handling
bool QtS3ReplyBase::isSuccess() { return d->isSuccess(); }

QNetworkReply::NetworkError QtS3ReplyBase::networkError() { return d->networkError(); }

QString QtS3ReplyBase::networkErrorString() { return d->networkErrorString(); }

QtS3ReplyBase::S3Error QtS3ReplyBase::s3Error() { return d->s3Error(); }

QString QtS3ReplyBase::s3ErrorString() { return d->s3ErrorString(); }

QString QtS3ReplyBase::anyErrorString() { return d->anyErrorString(); }

QByteArray QtS3ReplyBase::replyData() { return d->bytearrayValue(); }

template <> void QtS3Reply<void>::value() {}
template <> bool QtS3Reply<bool>::value() { return d->boolValue(); }
template <> int QtS3Reply<int>::value() { return d->intValue(); }
template <> QByteArray QtS3Reply<QByteArray>::value() { return d->bytearrayValue(); }

QtS3::QtS3(const QString &accessKeyId, const QString &secretAccessKey) : d(new QtS3Private)
{
    d->m_accessKeyId = accessKeyId.toLatin1();
    d->m_secretAccessKey = secretAccessKey.toLatin1();

    d->init();
}

QtS3::~QtS3() { delete d; }

QtS3Reply<QByteArray> QtS3::location(const QByteArray &bucketName)
{
    return QtS3Reply<QByteArray>(d->location(bucketName));
}

QtS3Reply<void> QtS3::put(const QByteArray &bucketName, const QString &path,
                          const QByteArray &content, const QStringList &headers)
{
    return QtS3Reply<void>(d->put(bucketName, path, content, headers));
}

QtS3Reply<bool> QtS3::exists(const QByteArray &bucketName, const QString &path)
{
    return QtS3Reply<bool>(d->exists(bucketName, path));
}

QtS3Reply<int> QtS3::size(const QByteArray &bucketName, const QString &path)
{
    return QtS3Reply<int>(d->size(bucketName, path));
}

QtS3Reply<QByteArray> QtS3::get(const QByteArray &bucketName, const QString &path)
{
    return QtS3Reply<QByteArray>(d->get(bucketName, path));
}

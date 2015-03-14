#include "qts3.h"
#include "qts3_p.h"

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

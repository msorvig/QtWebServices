#include <QtCore>

class QtS3Private;
class QtS3
{
public:
    QtS3(const QString &accessKeyId, const QString &secretAccessKey);
    ~QtS3();
    
    bool checkETag(const QString& bucketName, const QString& etag);
    QByteArray get(const QString& bucketName);
    bool put(const QString &bucketName, const QByteArray &content, const QString &contentType);
private:
    QtS3Private *d;
};

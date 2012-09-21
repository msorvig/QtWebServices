

class QtS3
{
public:
    QtS3(const QString &developerId, const QString &developerSecretKey);
    QtS3(); // Uses cahced id from settings.
    
    bool checkETag(const QString& bucketName, const QStrng& etag);
    QByteArray get(const QString& bucketName);
    void put(const QString &bucketName, const QbyteArray &content, const QString &contentType);
}

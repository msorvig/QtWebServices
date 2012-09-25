#include <QtCore>

class QtS3Private;
class QtS3
{
public:
    QtS3(const QString &accessKeyId, const QString &secretAccessKey);
    ~QtS3();
    
    int put(const QString &bucketName, const QString &path, const QByteArray &content, const QStringList &headers);

    int errorCode();
    QString errorString();
    void clearErrorState();

private:
    QtS3Private *d;
};

#include <QtCore>
#include <qts3.h>

QByteArray deflate(const QByteArray &source)
{
    QByteArray compressed = qCompress(source);
    return compressed.right(compressed.size() - 4); // remove qCompress header, leave deflate format
}

QString guessContentType(const QString &fileName)
{
    if (fileName.endsWith(".html"))
        return QStringLiteral("Content-Type:text/html");
    if (fileName.endsWith(".js"))
        return QStringLiteral("Content-Type:text/javascript");
    if (fileName.endsWith(".nmf"))
        return QStringLiteral("Content-Type:application/json");

    return QStringLiteral("Content-Type:application/octet-stream"); // binary
}

void transferDirectory(const QString sourcePath, const QString &targetBucket, const QString &targetPathPrefix = QString())
{
    // Connect to S3 using a accessKeyId/secretAccessKey pair.
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    QtS3 s3(environment.value("AWS_ACCESS_KEY_ID"), environment.value("AWS_SECRET_ACCESS_KEY"));
    if (s3.errorCode()) {
        qDebug() << "QtS3 fail:" << s3.errorString();
    }

    int fileCount = 0;
    
    // iterate over all files and subdirs
    QDirIterator it(sourcePath, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        if (!QFileInfo(filePath).isFile())
            continue;

        QString targetFileName = QFileInfo(filePath).fileName();
        if (targetFileName == QStringLiteral(".") || targetFileName == QStringLiteral(".."))
            continue;

        qDebug() << "Start transfer of file" << filePath;

        // read and compress file.
        QFile file(filePath);
        file.open(QIODevice::ReadOnly);
        QByteArray compressedContent = deflate(file.readAll());
        QByteArray etag = QByteArray::number(qHash(compressedContent), 16);

        // targetFilePath is the "local" path left after removing the source path
        QString targetFilePath = targetPathPrefix + filePath.right(filePath.count() - sourcePath.count() - 1);
        
        // upload to s3
        QStringList headers = QStringList() << guessContentType(filePath)
                                           << "Content-Encoding:deflate"
                                           << "Etag:\"" + etag + "\""
                                           << "Cache-Control:max-age=2"; // make cloudfront verify it has the most recent
                                                                         // version of the file.
        int fail = s3.put(targetBucket, targetFilePath, compressedContent, headers);
        if (fail) {
            qDebug() << "S3 upload failed:" << s3.errorString();
            s3.clearErrorState();
        } else {
            ++fileCount;
            qDebug() << "Transferred" << fileCount << "files";
        }
    }
}

void simpletest()
{
    // Connect to S3 using a accessKeyId/secretAccessKey pair.
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    QtS3 s3(environment.value("AWS_ACCESS_KEY_ID"), environment.value("AWS_SECRET_ACCESS_KEY"));

    // Transfer files:
    s3.put("qt-test-bucket", "test1", "TEST1", QStringList() << "Content-Type: text/plain" << "Content-Encoding:deflate");
    s3.put("qt-test-bucket", "test2", "TEST2", QStringList() << "Content-Type: text/plain" << "Content-Encoding:deflate");
    s3.put("qt-test-bucket", "test3", "TEST3", QStringList() << "Content-Type: text/plain" << "Content-Encoding:deflate");

    // Check for errors. QtS3 supports making multiple API calls without checking the
    // error state for each call. A failure sets an error flag on the QtS3 object,
    // subsequent API calls are fast no-ops. Calling takeErrorState clears the flag.
    QString errorString;
    int fail = s3.errorCode();
    if (fail) {
        qDebug() << "S3 upload failed:" << s3.errorString();
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QStringList args = app.arguments();
    if (args.count() < 3) {
        qDebug() << "usage: s3 sourcePath bucket [targetPath]";
        return 0;
    }

    QString sourcePath = QDir(args.at(1)).canonicalPath();
    QString bucket = args.at(2);
    if (sourcePath.isEmpty()) {
        qDebug() << "sourcePath not found: " << args.at(1);
        return 0;
    }

    QString targetPath;
    if (args.count() == 4)
        targetPath = args.at(3);

    transferDirectory(sourcePath, bucket, targetPath);
}

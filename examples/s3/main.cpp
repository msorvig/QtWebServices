#include <QtCore>
#include <qts3.h>

#ifdef HAVE_QTCONCURRENT
#include <qthreadfunctions.h>
#endif

// QDirIterator STL wrapper
class QDirIteratorIterator
{
public:
    typedef QString value_type;
    typedef QString &reference;
    typedef QString *pointer;

    QDirIteratorIterator(QDirIterator *it = 0)
        :_it(it) { if (it) ++(*this); }

    QString operator*() const {
        return _current;
    }
    QDirIteratorIterator& operator++() {
        _current = _it->next();
        return *this;
    }
    bool operator!=(const QDirIteratorIterator &other) {
        return !(_it == other._it || _current == other._current);
    }

    QDirIterator *_it;
    QString _current;
};

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

QHash<QThread *, int> seenThreads;

void transferDirectory(const QString sourcePath, const QString &targetBucket, const QString &targetPathPrefix = QString())
{
    int fileCount = 0;

    // Connect to S3 using a accessKeyId/secretAccessKey pair.
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    QtS3 s3(environment.value("AWS_ACCESS_KEY_ID"), environment.value("AWS_SECRET_ACCESS_KEY"));
    if (s3.errorCode()) {
        qDebug() << "QtS3 fail:" << s3.errorString();
    }
    
    auto transferFile = [&](const QString &filePath){
        if (!QFileInfo(filePath).isFile())
            return;

        QString targetFileName = QFileInfo(filePath).fileName();
        if (targetFileName == QStringLiteral(".") || targetFileName == QStringLiteral(".."))
            return;

        qDebug() << "Start compressing file" << filePath << QThread::currentThread();
        ++seenThreads[QThread::currentThread()];

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

        qDebug() << "Start transferring file" << filePath;

        int fail = s3.put(targetBucket, targetFilePath, compressedContent, headers);
        if (fail) {
            qDebug() << "S3 upload failed:" << s3.errorString();
            s3.clearErrorState();
        } else {
            ++fileCount;
            qDebug() << "Done transferring file" << filePath << "Total" << fileCount;
        }
    };

    QDirIterator dirIt(sourcePath, QDirIterator::Subdirectories);
    QDirIteratorIterator it(&dirIt);
    QDirIteratorIterator end;

#ifdef HAVE_QTCONCURRENT
    parallelForeach(it, end, transferFile);
#else
    std::for_each(it, end, transferFile);
#endif
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

    qDebug() << "Total number of unique threads used:" << seenThreads.count();
    qDebug() << seenThreads;
}

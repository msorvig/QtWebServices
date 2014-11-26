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

QString getKnownContentType(const QString &fileName)
{
    if (fileName.endsWith(".html"))
        return QStringLiteral("Content-Type:text/html");
    if (fileName.endsWith(".js"))
        return QStringLiteral("Content-Type:text/javascript");
    if (fileName.endsWith(".nmf"))
        return QStringLiteral("Content-Type:application/json");
    if (fileName.endsWith("image/jpg"))
        return QStringLiteral("Content-Type:image/jpg");
    return QString(); // unknown
}

QString guessContentType(const QString &fileName)
{
    QString knownContentType = getKnownContentType(fileName);
    if (knownContentType.isEmpty())
        return QStringLiteral("Content-Type:application/octet-stream"); // generic binary
    return knownContentType;
}

QHash<QThread *, int> seenThreads;

void transferDirectory(const QString sourcePath, const QString &bucket, const QString &targetPathPrefix = QString(), bool knownFilesOnly = false)
{
    // Connect to S3 using a accessKeyId/secretAccessKey pair.
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    QtS3 s3(environment.value("AWS_ACCESS_KEY_ID"), environment.value("AWS_SECRET_ACCESS_KEY"));
    if (s3.errorCode()) {
        qDebug() << "QtS3 fail:" << s3.errorString();
    }

    int fileCount = 0;
    auto transferFile = [&](const QString &filePath){
        if (!QFileInfo(filePath).isFile())
            return;

        QString targetFileName = QFileInfo(filePath).fileName();
        if (targetFileName == QStringLiteral(".") || targetFileName == QStringLiteral(".."))
            return;

        qDebug() << "Start processing file" << filePath << QThread::currentThread();
        ++seenThreads[QThread::currentThread()];

        // Get/guess the content-type header
        QString contentTypeHeader = getKnownContentType(filePath);
        if (contentTypeHeader.isEmpty()) {
            if (knownFilesOnly)
                return;
            else
                contentTypeHeader = guessContentType(filePath);
        }

        // read file and compute the sha1 hash.
        QFile file(filePath);
        file.open(QIODevice::ReadOnly);
        QByteArray fileContent = file.readAll();
        QByteArray hash = QCryptographicHash::hash(fileContent, QCryptographicHash::Sha1).toHex();
        QString targetFilePath = targetPathPrefix + hash;

        qDebug() << "target filePath" << targetFilePath << contentTypeHeader;

        QStringList headers = QStringList() << contentTypeHeader
                                            << "Etag:\"" + hash + "\"";

        int code = s3.exists(bucket, hash);
        if (code == 0) {
            qDebug() << bucket << "has file" << filePath << hash;
            return; // it exists; done
        }
        
        qDebug() << "S3 error code" << filePath <<  code << s3.errorString();
            
        s3.clearErrorState();
        s3.put(bucket, targetFilePath, fileContent, headers);
        if (s3.errorCode()) {
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
    QCoreApplication::setApplicationName("S3CAS");
    QCoreApplication::setApplicationVersion("0.1");
    
    QCommandLineParser parser;
    parser.setApplicationDescription("The Stupid S3 Content Addressable Storage Uploader");
    parser.addHelpOption();
    parser.addVersionOption();
    
    parser.addPositionalArgument("source", "Source directory to copy files from");
    parser.addPositionalArgument("bucket", "Destination bucket");
    parser.addPositionalArgument("target", "Target directory in bucket");
    
    QCommandLineOption unlyKnownFilesOption(QStringList() << "k" << "known", "Upload known file types only");
    parser.addOption(unlyKnownFilesOption);

#ifdef HAVE_QTCONCURRENT
    QCommandLineOption threadCountOption(QStringList() << "t" << "threads", "Use N threads", 
                                         "threadCount", QString::number(QThread::idealThreadCount()));
    parser.addOption(threadCountOption);
#endif    
    
    parser.process(app);
    const QStringList args = parser.positionalArguments();
    if (args.count() < 2) {
        qDebug() << "S3CAS requires at least two arguments";
        return 0;
    }
    QString source = args.at(0);
    QString bucket = args.at(1);
    QString target = args.count() > 2 ? args.at(3) : QString();
    bool knownFilesOnly = parser.isSet(unlyKnownFilesOption);
    qDebug() << "Source" << source;
    qDebug() << "Bucket " << bucket;
    if (!target.isEmpty()) 
        qDebug() << "Target" << target;
    qDebug() << "Known Files Only" << knownFilesOnly;
    
#ifdef HAVE_QTCONCURRENT
    QString threadCountOptionValue = parser.value(threadCountOption);
    int threadCount = threadCountOptionValue.toInt();
    qDebug() << "Thread Count" << threadCount;
    QThreadContext::current()->setMaxThreadCount(threadCount);
#endif    
    transferDirectory(source, bucket, target, knownFilesOnly);

    qDebug() << "Total number of unique threads used:" << seenThreads.count();
    qDebug() << seenThreads;
}

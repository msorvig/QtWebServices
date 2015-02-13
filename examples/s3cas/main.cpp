#include <QtCore>
#include <qts3.h>

bool g_knownFilesOnly = false;
bool g_enableCompression = false;
QByteArray g_bucket;
QByteArray g_salt = "";

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

void transferDirectory(const QString sourcePath, const QString &targetPathPrefix = QString())
{
    // Connect to S3 using a accessKeyId/secretAccessKey pair.
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    QtS3 s3(environment.value("S3CAS_AWS_ACCESS_KEY_ID"), environment.value("S3CAS_AWS_SECRET_ACCESS_KEY"));
    if (s3.errorCode()) {
        qDebug() << "QtS3 fail:" << s3.errorString();
    }

    // Keep a record of all uploaded files in a json
    // array of objects.
    //  
    // [ { "path/top/file" : "sha1hash" },
    //   ...
    // ]
    //
    QJsonArray tree;
    int fileCount = 0;

    // transfers file contents to a given path
    auto transferFileContents = [&](const QString &targetFilePath, const QByteArray &fileContent, QStringList headers) {
        s3.clearErrorState();
        s3.put(g_bucket, targetFilePath, fileContent, headers);
        if (s3.errorCode()) {
            qDebug() << "S3 upload failed:" << s3.errorString();
            s3.clearErrorState();
            return false;
        } 
        return true;
    };

    // stores the given file contents with a content-derived adresss
    auto storeFileContents = [&](const QByteArray &fileContent_, QString contentTypeHeader) {
        
        QByteArray fileContent = fileContent_;

        // compute hash, which becomes the file adress.
        QByteArray hash = QCryptographicHash::hash(fileContent, QCryptographicHash::Sha1).toHex();
        QString targetFilePath = targetPathPrefix + hash;
        
        // check for existence
        int code = s3.exists(g_bucket, hash);
        if (code == 0) {
            return hash; // it exists; done
        }

        // compress
        if (g_enableCompression) {
            // FIXME: store in deflate format (not qComprees)
            fileContent = qCompress(fileContent);
        }

        // upload
        QStringList headers = QStringList() << contentTypeHeader
                                            << "Etag:\"" + hash + "\"";
        transferFileContents(targetFilePath, fileContent, headers);
        ++fileCount;
        return hash;
    };

    // stores the given file with a content-derived adresss
    auto storeFile = [&](const QString &filePath){

        // sanity check the file path
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
            if (g_knownFilesOnly)
                return;
            else
                contentTypeHeader = guessContentType(filePath);
        }

        // read and store file
        QFile file(filePath);
        QFileInfo fileInfo(filePath);
        file.open(QIODevice::ReadOnly);
        QByteArray fileContent = file.readAll();
        QByteArray hash = storeFileContents(fileContent, contentTypeHeader);
        QString targetFilePath = targetPathPrefix + hash;

        // recoord file entry in file tree
        QJsonObject fileEntry;
        fileEntry[filePath] = targetFilePath;
        fileEntry["type"] = "blob";
        fileEntry["datetime"] = QString::number(fileInfo.created().toMSecsSinceEpoch());
        
        tree.append(fileEntry);
    };

    // Iterate over the soruce path, recurse into subdirectories
    QDirIterator dirIt(sourcePath, QDirIterator::Subdirectories);
    QDirIteratorIterator it(&dirIt);
    QDirIteratorIterator end;
#ifdef HAVE_QTCONCURRENT
    parallelForeach(it, end, storeFile);
#else
    std::for_each(it, end, storeFile);
#endif

    // uppload tree structure
    QByteArray jsonFileTree = QJsonDocument(tree).toJson();
    QByteArray treeHash = storeFileContents(jsonFileTree, "Content-Type:text/json");
    qDebug() << treeHash;

    // update the tag
    QStringList headers = QStringList() << "Content-Type:text/plain";
    QString tagName = "tag-fooo";
    transferFileContents(targetPathPrefix + tagName, treeHash, headers);
}

void downloadDirectory(const QString &tagName, const QString &targetDirectory)
{
    // Set up AWS S3 bucket url
    QString awss3Url = "https://s3.amazonaws.com/";
    QString bucketUrl = awss3Url + g_bucket + "/";
    QString tagUrl  = bucketUrl + tagName;

    // fetch tag
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("S3CAS");
    QCoreApplication::setApplicationVersion("0.1");

    QCommandLineParser parser;
    parser.setApplicationDescription("\n The Stupid S3 Content Addressable Storage Uploader\n"
                                     "\n"
                                     "S3CAS is configured using the following environment variables: \n"
                                     "\n"
                                     "    S3CAS_AWS_ACCESS_KEY_ID\n"
                                     "    S3CAS_AWS_SECRET_ACCESS_KEY\n"
                                     "    S3CAS_AWS_S3_BUCKET\n"
                                     "    S3CAS_SALT\n"
                                     "    S3CAS_ENABLE_COMPRESSION");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument("source", "Source directory to copy files from");
    parser.addPositionalArgument("target", "Target directory in bucket");

    QCommandLineOption unlyKnownFilesOption(QStringList() << "k" << "known", "Upload known file types only");
    parser.addOption(unlyKnownFilesOption);
    QCommandLineOption bucketOption(QStringList() << "b" << "bucket", "S3 bucket id");
    parser.addOption(unlyKnownFilesOption);

#ifdef HAVE_QTCONCURRENT
    QCommandLineOption threadCountOption(QStringList() << "t" << "threads", "Use N threads", 
                                         "threadCount", QString::number(QThread::idealThreadCount()));
    parser.addOption(threadCountOption);
#endif    

    parser.process(app);
    const QStringList args = parser.positionalArguments();
    if (args.count() < 1) {
        qDebug() << "S3CAS requires at least one argument";
        return 0;
    }
    QString source = args.at(0);
    QString target = args.count() > 2 ? args.at(3) : QString();
    g_knownFilesOnly = parser.isSet(unlyKnownFilesOption);
    QString g_bucket = qgetenv("S3CAS_AWS_S3_BUCKET");
    if (g_bucket.isEmpty())
        g_bucket = parser.value(bucketOption);

    g_salt = qgetenv("S3CAS_SALT");
    g_enableCompression = !qgetenv("S3CAS_ENABLE_COMPRESSION").isEmpty();

    qDebug() << "Source" << source;
    qDebug() << "Bucket " << g_bucket;
    qDebug() << "compression" << g_enableCompression;
    qDebug() << "salt" << g_salt;

    if (!target.isEmpty()) 
        qDebug() << "Target" << target;
    qDebug() << "Known Files Only" << g_knownFilesOnly;

#ifdef HAVE_QTCONCURRENT
    QString threadCountOptionValue = parser.value(threadCountOption);
    int threadCount = threadCountOptionValue.toInt();
    qDebug() << "Thread Count" << threadCount;
    QThreadContext::current()->setMaxThreadCount(threadCount);
#endif    
    transferDirectory(source, target);

    qDebug() << "Total number of unique threads used:" << seenThreads.count();
    qDebug() << seenThreads;
}

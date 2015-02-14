#include "qts3cas.h"
#include "qts3.h"

#ifdef HAVE_QTCONCURRENT
#include <qthreadfunctions.h>
#endif

class S3CasPrivate
{
public:
    S3Cas::Configs config;

    QByteArray awsAccessKey;
    QByteArray awsSecretKey;
    QByteArray bucket;
    QByteArray salt;
    bool compression;
    QString targetPathPrefix;
    QtS3 *s3;

    QJsonArray tree;
    int fileCount = 0;

    QHash<QThread *, int> seenThreads;

    S3CasPrivate(S3Cas::Configs _config);
    ~S3CasPrivate();

    bool checkConfigCreateS3();
    bool uploadContent(const QString &targetFilePath, const QByteArray &fileContent,
                       QStringList headers);

    QByteArray storeContent(const QByteArray &contents,
                            const QString &contentHeader
                            = QStringLiteral("Content-Type:application/octet-stream"));
    QByteArray storeFile(const QString &filePath);
    QByteArray storeDirectory(const QString &path);

    bool hasContent(const QByteArray &address);
    QByteArray getContent(const QByteArray &address);
    bool getDirectory(const QByteArray &treeAddress, const QString &destination);
};

// QDirIterator STL wrapper
class QDirIteratorIterator
{
public:
    typedef QString value_type;
    typedef QString &reference;
    typedef QString *pointer;

    QDirIteratorIterator(QDirIterator *it = 0) : _it(it)
    {
        if (it)
            ++(*this);
    }

    QString operator*() const { return _current; }
    QDirIteratorIterator &operator++()
    {
        _current = _it->next();
        return *this;
    }
    bool operator!=(const QDirIteratorIterator &other)
    {
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

S3Cas::S3Cas(Configs config) : d(new S3CasPrivate(config)) {}

S3Cas::~S3Cas() { delete d; }

void S3Cas::setAwsAccessKey(const QByteArray &accessKey) { d->awsAccessKey = accessKey; }

void S3Cas::setAwsSecretKey(const QByteArray &secretKey) { d->awsSecretKey = secretKey; }

void S3Cas::setSalt(const QByteArray &salt) { d->salt = salt; }

void S3Cas::setCompression(bool &enable) { d->compression = enable; }

QByteArray S3Cas::storeContent(const QByteArray &contents) { return d->storeContent(contents); }

QByteArray S3Cas::storeFile(const QString &path) { return d->storeFile(path); }

QByteArray S3Cas::storeDirectory(const QString &path) { return d->storeDirectory(path); }

QByteArray S3Cas::getContent(const QByteArray &blobAddress) { return d->getContent(blobAddress); }

bool S3Cas::getDirectory(const QByteArray &treeAddress, const QString &destination)
{
    return d->getDirectory(treeAddress, destination);
}

S3CasPrivate::S3CasPrivate(S3Cas::Configs _config) : config(_config), s3(0), fileCount(0)
{
    awsAccessKey = qgetenv("S3CAS_AWS_ACCESS_KEY_ID");
    awsSecretKey = qgetenv("S3CAS_AWS_SECRET_ACCESS_KEY");
    bucket = qgetenv("S3CAS_AWS_S3_BUCKET");
    salt = qgetenv("S3CAS_SALT");
    compression = qgetenv("S3CAS_ENABLE_COMPRESSION").toInt();
}

S3CasPrivate::~S3CasPrivate() { delete s3; }

bool S3CasPrivate::checkConfigCreateS3()
{
    if (awsAccessKey.isEmpty()) {
        qWarning() << "Aws access key not set";
        return false;
    }
    if (awsSecretKey.isEmpty()) {
        qWarning() << "Aws secret key not set";
        return false;
    }
    s3 = new QtS3(awsAccessKey, awsSecretKey);
    if (s3->errorCode()) {
        qWarning() << "QtS3 fail:" << s3->errorString();
        delete s3;
        return false;
    }
    return true;
}

bool S3CasPrivate::uploadContent(const QString &targetFilePath, const QByteArray &fileContent,
                                 QStringList headers)
{
    Q_ASSERT(s3);
    s3->clearErrorState();
    s3->put(bucket, targetFilePath, fileContent, headers);
    if (s3->errorCode()) {
        qWarning() << "S3 upload failed:" << s3->errorString();
        s3->clearErrorState();
        return false;
    }
    return true;
}

QByteArray S3CasPrivate::storeContent(const QByteArray &contents, const QString &contentTypeHeader)
{
    if (!checkConfigCreateS3())
        return QByteArray();

    QByteArray fileContent = contents;

    // compute content hash
    QByteArray hash = QCryptographicHash::hash(fileContent, QCryptographicHash::Sha1).toHex();
    QString targetFilePath = targetPathPrefix + hash;

    // check for existence
    int code = s3->exists(bucket, hash);
    if (code == 0) {
        return hash; // it exists; done
    }

    // compress
    if (compression) {
        // FIXME: store in deflate format (not qComprees)
        fileContent = qCompress(fileContent);
    }

    // upload
    QStringList headers = QStringList() << contentTypeHeader << "Etag:\"" + hash + "\"";
    uploadContent(targetFilePath, fileContent, headers);
    ++fileCount;
    return hash;
}

QByteArray S3CasPrivate::storeFile(const QString &filePath)
{
    if (!checkConfigCreateS3())
        return QByteArray();

    // sanity check the file path
    if (!QFileInfo(filePath).isFile())
        return QByteArray();
    QString targetFileName = QFileInfo(filePath).fileName();
    if (targetFileName == QStringLiteral(".") || targetFileName == QStringLiteral(".."))
        return QByteArray();

    qDebug() << "Start processing file" << filePath << QThread::currentThread();
    ++seenThreads[QThread::currentThread()];

#if 0
    // Get/guess the content-type header
    QString contentTypeHeader = getKnownContentType(filePath);
    if (contentTypeHeader.isEmpty()) {
        if (g_knownFilesOnly)
            return QByteArray();
        else
            contentTypeHeader = guessContentType(filePath);
    }
#endif

    // read and store file
    QFile file(filePath);
    QFileInfo fileInfo(filePath);
    file.open(QIODevice::ReadOnly);
    QByteArray fileContent = file.readAll();
    QByteArray hash = storeContent(fileContent);
    QString targetFilePath = targetPathPrefix + hash;

    // record file entry in file tree
    QJsonObject fileEntry;
    fileEntry[filePath] = targetFilePath;
    fileEntry["type"] = "blob";
    fileEntry["datetime"] = QString::number(fileInfo.created().toMSecsSinceEpoch());

    tree.append(fileEntry);

    return hash;
}

QByteArray S3CasPrivate::storeDirectory(const QString &path)
{
    if (!checkConfigCreateS3())
        return QByteArray();

    tree = QJsonArray();

    // Iterate over files in and below the source path
    QDirIterator dirIt(path, QDirIterator::Subdirectories);
    QDirIteratorIterator it(&dirIt);
    QDirIteratorIterator end;

    auto storeFile = [&](const QString &filePath) { this->storeFile(filePath); };

#ifdef HAVE_QTCONCURRENT
    parallelForeach(it, end, storeFile);
#else
    std::for_each(it, end, storeFile);
#endif

    // upload tree structure
    QByteArray jsonFileTree = QJsonDocument(tree).toJson();
    QByteArray treeHash = storeContent(jsonFileTree, "Content-Type:text/json");
    qDebug() << jsonFileTree;

    // update the tag
    QStringList headers = QStringList() << "Content-Type:text/plain";
    QString tagName = "tag-fooo";
    uploadContent(targetPathPrefix + tagName, treeHash, headers);

    return treeHash;
}

bool S3CasPrivate::hasContent(const QByteArray &address)
{
    if (!checkConfigCreateS3())
        return false;
}

QByteArray S3CasPrivate::getContent(const QByteArray &address)
{
    if (!checkConfigCreateS3())
        return QByteArray();
}

bool S3CasPrivate::getDirectory(const QByteArray &treeAddress, const QString &destination)
{
    if (!checkConfigCreateS3())
        return false;

    QByteArray tagName;
    // Set up AWS S3 bucket url
    QString awss3Url = "https://s3.amazonaws.com/";
    QString bucketUrl = awss3Url + bucket + "/";
    QString tagUrl = bucketUrl + tagName;

    // fetch tag
}

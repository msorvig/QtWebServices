#include <qts3.h>
#include <qts3cas.h>

#include <QtCore>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("S3CAS");
    QCoreApplication::setApplicationVersion("0.2");

    // Configure command line options
    QCommandLineParser parser;
    parser.setApplicationDescription(
        "\n The Stupid S3 Content Addressable Storage Uploader\n"
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

    QCommandLineOption unlyKnownFilesOption(QStringList() << "k"
                                                          << "known",
                                            "Upload known file types only");
    parser.addOption(unlyKnownFilesOption);

    QCommandLineOption bucketOption(QStringList() << "b"
                                                  << "bucket",
                                    "S3 bucket id");
    parser.addOption(bucketOption);

#ifdef HAVE_QTCONCURRENT
    QCommandLineOption threadCountOption(QStringList() << "t"
                                                       << "threads",
                                         "Use N threads", "threadCount",
                                         QString::number(QThread::idealThreadCount()));
    parser.addOption(threadCountOption);
#endif

    // Parse and process command line options
    parser.process(app);
    const QStringList args = parser.positionalArguments();
    if (args.count() < 1) {
        qDebug() << "S3CAS requires at least one argument";
        return 0;
    }

    S3Cas s3cas;

    QString source = args.at(0);
    QString target = args.count() > 2 ? args.at(3) : QString();

    if (parser.isSet(unlyKnownFilesOption))
        // s3cas.setKnownFileTypesOnly(parser.value(unlyKnownFilesOption).toInt());
        if (parser.isSet(bucketOption))
            // s3cas.setBucket(parser.value(bucketOption));

            // Print active options
    qDebug() << "Source" << source;
    qDebug() << "Bucket " << bucket;
    // qDebug() << "compression" << s3cas.compressionEnabled();
    // qDebug() << "salt" << s3cas.salt();
    // qDebug() << "Known Files Only" << s3cas.knownFileTypesOnly;
    qDebug() << "Target sub-path" << target;

    // Run command
    s3cas.storeDirectory(source);
    return 0;
}

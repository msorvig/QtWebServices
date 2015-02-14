#include <QtCore>

class S3CasPrivate;
class S3Cas
{
public:
    enum Config {
        NoAutoConfig = 0x0,
        EnvironmentVariablesConfig = 0x1,
        DotFileConfig = 0x2,
    };
    Q_DECLARE_FLAGS(Configs, Config)

    S3Cas(Configs config = EnvironmentVariablesConfig);
    ~S3Cas();

    // Config
    void setAwsAccessKey(const QByteArray &accessKey);
    void setAwsSecretKey(const QByteArray &secretKey);
    void setSalt(const QByteArray &salt);
    void setCompression(bool &enable);

    // Store/upload
    QByteArray storeContent(const QByteArray &contents);
    QByteArray storeFile(const QString &path);
    QByteArray storeDirectory(const QString &path);

    // Get/download
    bool hasContent(const QByteArray &address);
    QByteArray getContent(const QByteArray &address);
    bool getDirectory(const QByteArray &treeAddress, const QString &destination);

private:
    S3CasPrivate *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(S3Cas::Configs)

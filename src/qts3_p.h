#ifndef QTS3_P_H
#define QTS3_P_H

#include "qts3.h"

#include <QtNetwork>

class QtS3Private
{
public:
    QtS3Private();
    QtS3Private(QByteArray accessKeyId, QByteArray secretAccessKey);

    QByteArray m_accessKeyId;
    QByteArray m_secretAccessKey;

    QByteArray m_region;
    QByteArray m_service;

    QNetworkAccessManager *m_networkAccessManager;

    QDateTime s3SigningKeyTimeStamp;
    QByteArray currents3SigningKey;

    static QByteArray hash(const QByteArray &data);
    static QByteArray sign(const QByteArray &key, const QByteArray &data);

    static QByteArray deriveSigningKey(const QByteArray &secretAccessKey,
                                       const QByteArray dateString, const QByteArray &m_region,
                                       const QByteArray &m_service);

    static QByteArray formatDate(const QDate &date);
    static QByteArray formatDateTime(const QDateTime &dateTime);
    static QByteArray formatHeaderNameValueList(const QMap<QByteArray, QByteArray> &headers);
    static QByteArray formatHeaderNameList(const QMap<QByteArray, QByteArray> &headers);
    static QByteArray createCanonicalQueryString(const QByteArray &queryString);
    static QMap<QByteArray, QByteArray> canonicalHeaders(const QHash<QByteArray, QByteArray> &headers);
    static QHash<QByteArray, QByteArray> parseHeaderList(const QStringList &headers);
    static QByteArray formatCanonicalRequest(const QByteArray &method, const QByteArray &url,
                                             const QByteArray &queryString,
                                             const QHash<QByteArray, QByteArray> &headers,
                                             const QByteArray &payloadHash);
    static QByteArray formatStringToSign(const QDateTime &timeStamp, const QByteArray &m_region,
                                         const QByteArray &m_service,
                                         const QByteArray &canonicalReqeustHash);
    static QByteArray
    formatAuthorizationHeader(const QByteArray &awsAccessKeyId, const QDateTime &timeStamp,
                              const QByteArray &m_region, const QByteArray &m_service,
                              const QByteArray &signedHeaders, const QByteArray &signature);

    static QByteArray signRequestData(const QHash<QByteArray, QByteArray> headers,
                                      const QByteArray &verb, const QByteArray &url,
                                      const QByteArray &queryString, const QByteArray &payload,
                                      const QByteArray &signingKey, const QDateTime &dateTime,
                                      const QByteArray &m_region, const QByteArray &m_service);
    static QByteArray
    createAuthorizationHeader(const QHash<QByteArray, QByteArray> headers, const QByteArray &verb,
                              const QByteArray &url, const QByteArray &queryString,
                              const QByteArray &payload, const QByteArray accessKeyId,
                              const QByteArray &signingKey, const QDateTime &dateTime,
                              const QByteArray &m_region, const QByteArray &m_service);

    // Signing key management
    static bool checkGenerateSigningKey(QByteArray *currentKey, QDateTime *currentKeyTimestamp,
                                        const QDateTime &now, const QByteArray &secretAccessKey,
                                        const QByteArray &m_region, const QByteArray &m_service);

    // QNetworkRequest creation and signing;
    static QHash<QByteArray, QByteArray> requestHeaders(const QNetworkRequest *request);
    static void setRequestAttributes(QNetworkRequest *request, const QUrl &url,
                                     const QHash<QByteArray, QByteArray> &headers,
                                     const QDateTime &timeStamp, const QByteArray &m_host);
    static void signRequest(QNetworkRequest *request, const QByteArray &verb,
                            const QByteArray &payload, const QByteArray accessKeyId,
                            const QByteArray &signingKey, const QDateTime &dateTime,
                            const QByteArray &m_region, const QByteArray &m_service);

    // Top-level stateful functions. These read object state and may/will modify it in a thread-safe way.
    void init();
    void checkGenerateS3SigningKey();
    QNetworkRequest *createSignedRequest(const QByteArray &verb, const QUrl &url,
                                         const QHash<QByteArray, QByteArray> &headers,
                                         const QByteArray &host, const QByteArray &payload);
    QNetworkReply *sendRequest(const QByteArray &verb, const QNetworkRequest &request,
                               const QByteArray &payload);
    QNetworkReply *sendS3Request(const QByteArray &bucketName, const QByteArray &verb,
                                 const QString &path, const QByteArray &queryString,
                                 const QByteArray &content, const QStringList &headers);
    void waitForFinished(QNetworkReply *reply);

    // Public API. The public QtS3 class calls these.
    QByteArray location(const QByteArray &bucketName);
    bool put(const QByteArray &bucketName, const QString &path, const QByteArray &content,
             const QStringList &headers);
    bool exists(const QByteArray &bucketName, const QString &path);
    int size(const QByteArray &bucketName, const QString &path);
    QtS3Optional<QByteArray> get(const QByteArray &bucketName, const QString &path);
    bool get(QByteArray *destination, const QByteArray &bucketName, const QString &path);

    template <typename L> bool get(const QByteArray &bucketName, const QString &path, L writer);
};

#endif

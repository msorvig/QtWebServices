#ifndef QTS3_P_H
#define QTS3_P_H

#include "qts3.h"

#include <QtNetwork>

class QtS3Private
{
public:
#ifdef USE_LIBAWS
    QString m_accessKeyId;
    QString m_secretAccessKey;
#else
    QByteArray m_accessKeyId;
    QByteArray m_secretAccessKey;
#endif
    int errorState;
    QString errorString;

    QByteArray region;
    QByteArray service;
    QByteArray host;

    QDateTime s3SigningKeyTimeStamp;
    QByteArray currents3SigningKey;

    static QByteArray hash(const QByteArray &data);
    static QByteArray sign(const QByteArray &key, const QByteArray &data);

    static QByteArray deriveSigningKey(const QByteArray &secretAccessKey, const QByteArray dateString,
                                       const QByteArray &region, const QByteArray &service);


    static QByteArray formatDate(const QDate &date);
    static QByteArray formatDateTime(const QDateTime &dateTime);
    static QByteArray formatHeaderNameValueList(const QMap<QByteArray, QByteArray> &headers);
    static QByteArray formatHeaderNameList(const QMap<QByteArray, QByteArray> &headers);
    static QMap<QByteArray, QByteArray> canonicalHeaders(const QHash<QByteArray, QByteArray> &headers);
    static QByteArray formatCanonicalRequest(const QByteArray &method, const QByteArray &url,
                                             const QByteArray &queryString,
                                             const QHash<QByteArray, QByteArray> &headers,
                                             const QByteArray &payloadHash);
    static QByteArray formatStringToSign(const QDateTime &timeStamp, const QByteArray &region,
                                         const QByteArray &service, const QByteArray &canonicalReqeustHash);
    static QByteArray formatAuthorizationHeader(const QByteArray &awsAccessKeyId, const QDateTime &timeStamp,
                                                const QByteArray &region, const QByteArray &service,
                                                const QByteArray &signedHeaders, const QByteArray &signature);

    static QByteArray signRequestData(const QHash<QByteArray, QByteArray> headers, const QByteArray &verb,  const QByteArray &url,
                                      const QByteArray &payload, const QByteArray &signingKey,
                                      const QDateTime &dateTime, const QByteArray &region,
                                      const QByteArray &service);
    static QByteArray createAuthorizationHeader(const QHash<QByteArray, QByteArray> headers, const QByteArray &verb, const QByteArray &url,
                                                const QByteArray &payload, const QByteArray accessKeyId, const QByteArray &signingKey,
                                                const QDateTime &dateTime, const QByteArray &region,
                                                const QByteArray &service);

    // static QByteArray stringToSign(const QByteArray &dateTimeString, const QByteArray &region,
    // const QByteArray &service, const QByteArray &requestHash);
    // static QByteArray signedRequest(const QByteArray &canonicalRequest, const QByteArray
    // &requestSignature);

    // Signing key management
    static bool checkGenerateSigningKey(QByteArray *currentKey, QDateTime *currentKeyTimestamp,
                                        const QDateTime &now, const QByteArray &secretAccessKey,
                                        const QByteArray &region, const QByteArray &service);

    // QNetworkRequest creation and signing;
    static QHash<QByteArray, QByteArray> requestHeaders(const QNetworkRequest *request);
    static QNetworkRequest *createRequest(const QUrl &url,
                                          const QHash<QByteArray, QByteArray> &headers,
                                          const QDateTime &timeStamp, const QByteArray &host);
    static void signRequest(QNetworkRequest *request, const QByteArray &verb,
                               const QByteArray &payload, const QByteArray accessKeyId, const QByteArray &signingKey,
                               const QDateTime &dateTime, const QByteArray &region,
                               const QByteArray &service);

    // Top-level stateful functions. These read object state and may/will modify it in a thread-safe way.
    void checkGenerateS3SigningKey();
    QNetworkRequest *createSignedRequest(const QByteArray &verb, const QUrl &url,
                                         const QHash<QByteArray, QByteArray> &headers,
                                         const QByteArray &payload);
    void sendRequest(const QByteArray &verb, QNetworkRequest *request, const QByteArray &payload);
    void sendRequest(const QByteArray &verb, const QUrl &url,
                     const QHash<QByteArray, QByteArray> &headers, const QByteArray &payload);

    // Public API. The public QtS3 class calls these.
    void put(const QByteArray &bucketName, const QString &path, const QByteArray &content,
             const QStringList &headers);
    bool exists(const QByteArray &bucketName, const QString &path);
    QtS3Optional<QByteArray> get(const QByteArray &bucketName, const QString &path);
};

#endif

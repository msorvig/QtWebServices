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

    // AWS date-time formatting
    static QByteArray awsDate(const QDate &date);
    static QByteArray awsDateTime(const QDateTime &dateTime);

    // AWS key and signature generation.
    static QByteArray HMAC_SHA256(const QByteArray &key, const QByteArray &data);
    static QByteArray SHA256(const QByteArray &data);
    static QByteArray awsSigningKey(const QByteArray &secretAccessKey, const QByteArray dateString,
                                    const QByteArray &region, const QByteArray &service);
    static QByteArray awsSignature(const QByteArray &awsSignatureKey,
                                   const QByteArray stringToSign);
    static bool checkGenerateAwsSigningKey(QByteArray *currentKey, QDateTime *currentKeyTimestamp,
                                           const QDateTime &now, const QByteArray &secretAccessKey,
                                           const QByteArray &region, const QByteArray &service);
    static QByteArray awsStringToSign(const QDateTime &timeStamp, const QByteArray &region,
                                      const QByteArray &service,
                                      const QByteArray &canonicalReqeustHash);

    // AWS request canoicalization and signing
    static QMap<QByteArray, QByteArray>
    canonicalHeaders(const QHash<QByteArray, QByteArray> &headers);
    static QByteArray canonicalRequest(const QByteArray &method, const QByteArray &uri,
                                       const QByteArray &queryString,
                                       const QHash<QByteArray, QByteArray> &headers,
                                       const QByteArray &payloadHash);
    // static QByteArray stringToSign(const QByteArray &dateTimeString, const QByteArray &region,
    // const QByteArray &service, const QByteArray &requestHash);
    // static QByteArray signedRequest(const QByteArray &canonicalRequest, const QByteArray
    // &requestSignature);

    // Top-level request creation and signing;
    static QNetworkRequest *createS3Request(const QUrl &uri,
                                            const QHash<QByteArray, QByteArray> &headers,
                                            const QDateTime &timeStamp, const QByteArray &host);
    static void signAwsRequest(QNetworkRequest *request, const QByteArray &verb,
                               const QByteArray &payload, const QByteArray &signingKey,
                               const QDateTime &dateTime, const QByteArray &region,
                               const QByteArray &service);

    // Top-level stateful functions. These may/will modify the object state in a thread-safe way.
    void checkGenerateS3SigningKey();
    QNetworkRequest *createS3Request(const QByteArray &verb, const QUrl &uri,
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

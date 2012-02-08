#ifndef BLOCKINGNETWORKACCESSMANAGER_H
#define BLOCKINGNETWORKACCESSMANAGER_H

#include <QNetworkAccessManager>

class BlockingNetworkAccessManager : public QNetworkAccessManager
{
public:
    BlockingNetworkAccessManager(QObject *parent);

    QNetworkReply *syncGet(const QNetworkRequest &request);

private slots:
    void replyFinished(QNetworkReply *);
};

#endif

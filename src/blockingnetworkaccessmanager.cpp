#include "blockingnetworkaccessmanager.h"
#include <QtCore>

BlockingNetworkAccessManager::BlockingNetworkAccessManager(QObject *parent)
    :QNetworkAccessManager(parent)
{

}

QNetworkReply *BlockingNetworkAccessManager::syncGet(const QNetworkRequest &request)
{
    QNetworkReply *reply = get(request);

    QEventLoop loop;
    QObject::connect(this, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    loop.exec();

    return reply;
}

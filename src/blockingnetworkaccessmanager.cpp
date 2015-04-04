#include "blockingnetworkaccessmanager.h"
#include <QtCore>
#include <QtNetwork/QNetworkReply>

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

QNetworkReply *SlottetNetworkAccessManager::sendCustomRequest_slot(const QNetworkRequest &request,
                                                                   const QByteArray &verb,
                                                                   QIODevice *data)
{
    return sendCustomRequest(request, verb, data);
}

// A thread-safe network access manager wrapper.
ThreadsafeBlockingNetworkAccesManager::ThreadsafeBlockingNetworkAccesManager()
{
    m_networkThread = new QThread;
    m_networkThread->start();
    m_networkAccessManager = new SlottetNetworkAccessManager;
    m_networkAccessManager->moveToThread(m_networkThread);
}

ThreadsafeBlockingNetworkAccesManager::~ThreadsafeBlockingNetworkAccesManager()
{
    delete m_networkAccessManager;
    m_networkThread->quit();
    m_networkThread->wait();
    delete m_networkThread;
}

// A synchronous, thread-safe sendCustomRequest.
QNetworkReply *ThreadsafeBlockingNetworkAccesManager::sendCustomRequest(
    const QNetworkRequest &request, const QByteArray &verb, QIODevice *data)
{
    // Call sendCustomRequest on QNetworkAccessMaanger, on the network thread. Use a
    // BlockingQueuedConnection to get the returned reply object.
    QNetworkReply *reply = 0;
    QMetaObject::invokeMethod(m_networkAccessManager, "sendCustomRequest_slot",
                              Qt::BlockingQueuedConnection, Q_RETURN_ARG(QNetworkReply *, reply),
                              Q_ARG(QNetworkRequest, request), Q_ARG(QByteArray, verb),
                              Q_ARG(QIODevice *, data));

    // The reply should wake this thread when the request completes.
    connect(reply, SIGNAL(finished()), this, SLOT(wakeWaitingThreads()), Qt::DirectConnection);

    // Special case for HEAD requests: return on first metaDataChanged(). Needed to reduce
    // wait time - finished() is not signaled until S3 times out and clses the connection.
    const bool isHead = (verb == "HEAD");
    if (isHead)
        connect(reply, SIGNAL(metaDataChanged()), this, SLOT(wakeWaitingThreads()),
                Qt::DirectConnection);

    // Wait until the request completes
    while (!(reply->isFinished() || (isHead && reply->rawHeaderList().count() > 0))) {
        QMutexLocker lock(&m_mutex);
        m_waitConditon.wait(&m_mutex);
    }

    return reply;
}

void ThreadsafeBlockingNetworkAccesManager::wakeWaitingThreads() { m_waitConditon.wakeAll(); }

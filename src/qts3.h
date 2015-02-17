#ifndef QTS3_H
#define QTS3_H

#include <QtCore>

class QtS3Private;

template <typename T> class QtS3Optional
{
public:
    QtS3Optional(T t) : m_t(t), m_isValid(true) {}

    QtS3Optional() : m_t(T()), m_isValid(false) {}

    T get() { return m_t; }

    bool isValid() { return m_isValid; }

private:
    T m_t;
    bool m_isValid;
};

class QtS3
{
public:
    QtS3(const QString &accessKeyId, const QString &secretAccessKey);
    ~QtS3();
    
    int put(const QString &bucketName, const QString &path, const QByteArray &content, const QStringList &headers);
    int exists(const QString &bucketName, const QString &path);
    QtS3Optional<QByteArray> get(const QString &bucketName, const QString &path);

    int errorCode();
    QString errorString();
    void clearErrorState();

private:
    QtS3Private *d;
};

#endif

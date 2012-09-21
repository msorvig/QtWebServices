#include <QtCore>

#ifndef PICASATYPES_H
#define PICASATYPES_H

class QtPicasaThumbnail
{
public:
    int width;
    int height;
    QString url;
};

class QtPicasaImage
{
public:
    QString id;
    QString albumid;
    int width;
    int height;
    int size;
    quint64 timestamp; // milliseconds since 1970
    QString title;
    QString description;
    QString url;
    QList<QtPicasaThumbnail> thumbnails; // Generally three thumbnails of increasing size
};

// corresponds to one picasa album
class QtPicasaAlbum
{
public:
    QString title;
    QList<QtPicasaImage> images;
};

class QtPicasaAlbumMeta
{
public:
    QString title;
    QString id;
};

class QtPicasaFeed
{
public:
    QList<QtPicasaAlbumMeta> albumMetas;

};

QDebug operator<<(QDebug debug, const QtPicasaFeed &feed);
QDebug operator<<(QDebug debug, const QtPicasaAlbumMeta &albumMeta);
QDebug operator<<(QDebug debug, const QtPicasaAlbum &album);
QDebug operator<<(QDebug debug, const QtPicasaImage &image);

QDataStream &operator<<(QDataStream &out, const QtPicasaFeed &feed);
QDataStream &operator>>(QDataStream &in, QtPicasaFeed &feed);
QDataStream &operator<<(QDataStream &out, const QtPicasaAlbumMeta &albumMeta);
QDataStream &operator>>(QDataStream &in, QtPicasaAlbumMeta &albumMeta4);
QDataStream &operator<<(QDataStream &out, const QtPicasaAlbum &album);
QDataStream &operator>>(QDataStream &in, QtPicasaAlbum &album);
QDataStream &operator<<(QDataStream &out, const QtPicasaImage &image);
QDataStream &operator>>(QDataStream &in, QtPicasaImage &image);
QDataStream &operator<<(QDataStream &out, const QtPicasaThumbnail &thumbnail);
QDataStream &operator>>(QDataStream &in, QtPicasaThumbnail &thumbnail);

#endif

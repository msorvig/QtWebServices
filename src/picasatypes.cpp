#include "picasatypes.h"

QDebug operator<<(QDebug debug, const QtPicasaFeed &feed)
{
    debug << feed.albumMetas << "\n";
    return debug;
}

QDebug operator<<(QDebug debug, const QtPicasaAlbumMeta &albumMeta)
{
    debug << albumMeta.title << " " << albumMeta.id << "\n";
    return debug;
}

QDebug operator<<(QDebug debug, const QtPicasaAlbum &album)
{
    debug << album.title << "\n";
    debug << album.images << "\n";
    return debug;
}

QDebug operator<<(QDebug debug, const QtPicasaImage &image)
{
    debug << image.title << image.description << image.url << "\n";
    return debug;
}

QDataStream &operator<<(QDataStream &out, const QtPicasaFeed &feed)
{
    out << feed.albumMetas;
    return out;
}

QDataStream &operator>>(QDataStream &in, QtPicasaFeed &feed)
{
    in >> feed.albumMetas;
    return in;
}

QDataStream &operator<<(QDataStream &out, const QtPicasaAlbumMeta &albumMeta)
{
    out << albumMeta.id << albumMeta.title;
    return out;
}

QDataStream &operator>>(QDataStream &in, QtPicasaAlbumMeta &albumMeta)
{
    in >> albumMeta.id >> albumMeta.title;
    return in;
}

QDataStream &operator<<(QDataStream &out, const QtPicasaAlbum &album)
{
    out << album.title << album.images;
    return out;
}

QDataStream &operator>>(QDataStream &in, QtPicasaAlbum &album)
{
    in >> album.title >> album.images;
    return in;
}

QDataStream &operator<<(QDataStream &out, const QtPicasaImage &image)
{
    out << image.id;
    out << image.albumid;
    out << image.width;
    out << image.height;
    out << image.size;
    out << image.timestamp;
    out << image.title;
    out << image.description;
    out << image.url;
    out << image.thumbnails;
    return out;
}

QDataStream &operator>>(QDataStream &in, QtPicasaImage &image)
{
    in >> image.id;
    in >> image.albumid;
    in >> image.width;
    in >> image.height;
    in >> image.size;
    in >> image.timestamp;
    in >> image.title;
    in >> image.description;
    in >> image.url;
    in >> image.thumbnails;
    return in;
}

QDataStream &operator<<(QDataStream &out, const QtPicasaThumbnail &thumbnail)
{
    out << thumbnail.width;
    out << thumbnail.height;
    out << thumbnail.url;
    return out;
}

QDataStream &operator>>(QDataStream &in, QtPicasaThumbnail &thumbnail)
{
    in >> thumbnail.width;
    in >> thumbnail.height;
    in >> thumbnail.url;
    return in;
}

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

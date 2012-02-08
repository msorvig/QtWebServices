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


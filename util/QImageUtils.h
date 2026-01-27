#include <QImage>

namespace QImageUtils
{
  QImage blurAlphaMask( const QImage& src, int radius )
  {
    if ( radius <= 0 )
        return src;
    // Downscale-Faktor
    const int scale = qMax(1, radius / 2);
    QSize smallSize(
        qMax(1, src.width()  / scale),
        qMax(1, src.height() / scale)
    );
    QImage small = src.scaled(
        smallSize,
        Qt::IgnoreAspectRatio,
        Qt::SmoothTransformation
    );
    QImage blurred = small.scaled(
        src.size(),
        Qt::IgnoreAspectRatio,
        Qt::SmoothTransformation
    );
    return blurred;
  }
  
}
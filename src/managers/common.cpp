
#include <QObject>

#include "common.h"

#if QT_VERSION >= 0x040400
QT_BEGIN_NAMESPACE
#endif

namespace sizeFunks
{

void orderBorders(QSize &minVal, QSize &maxVal)
{
    orderSizeBorders(minVal, maxVal);
}

void orderBorders(QSizeF &minVal, QSizeF &maxVal)
{
    orderSizeBorders(minVal, maxVal);
}

}

QSize qBound(QSize minVal, QSize val, QSize maxVal)
{
    return qBoundSize(minVal, val, maxVal);
}

QSizeF qBound(QSizeF minVal, QSizeF val, QSizeF maxVal)
{
    return qBoundSize(minVal, val, maxVal);
}

#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif


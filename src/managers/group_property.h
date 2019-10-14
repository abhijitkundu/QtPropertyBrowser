
#ifndef QtGroupPropertyManager_H
#define QtGroupPropertyManager_H

#include "../qtpropertybrowser.h"

#if QT_VERSION >= 0x040400
QT_BEGIN_NAMESPACE
#endif

class QT_QTPROPERTYBROWSER_EXPORT QtGroupPropertyManager : public QtAbstractPropertyManager
{
    Q_OBJECT
public:
    QtGroupPropertyManager(QObject *parent = 0);
    ~QtGroupPropertyManager();

protected:
    virtual bool hasValue(const QtProperty *property) const;

    virtual void initializeProperty(QtProperty *property);
    virtual void uninitializeProperty(QtProperty *property);
};

#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif

#endif


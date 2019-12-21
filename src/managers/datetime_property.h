
#ifndef QtDateTimePropertyManager_H
#define QtDateTimePropertyManager_H

#include "../qtpropertybrowser.h"

#if QT_VERSION >= 0x040400
QT_BEGIN_NAMESPACE
#endif

class QDateTime;

class QtDateTimePropertyManagerPrivate;

class QT_QTPROPERTYBROWSER_EXPORT QtDateTimePropertyManager : public QtAbstractPropertyManager
{
    Q_OBJECT
public:
    QtDateTimePropertyManager(QObject *parent = nullptr);
    ~QtDateTimePropertyManager();

    QDateTime value(const QtProperty *property) const;

public Q_SLOTS:
    void setValue(QtProperty *property, const QDateTime &val);
Q_SIGNALS:
    void valueChanged(QtProperty *property, const QDateTime &val);
protected:
    QString valueText(const QtProperty *property) const;
    virtual void initializeProperty(QtProperty *property);
    virtual void uninitializeProperty(QtProperty *property);
private:
    QtDateTimePropertyManagerPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtDateTimePropertyManager)
    Q_DISABLE_COPY(QtDateTimePropertyManager)
};


#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif

#endif


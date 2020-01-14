
#ifndef QtTimePropertyManager_H
#define QtTimePropertyManager_H

#include "../qtpropertybrowser.h"

#if QT_VERSION >= 0x040400
QT_BEGIN_NAMESPACE
#endif

class QTime;

class QtTimePropertyManagerPrivate;

class QT_QTPROPERTYBROWSER_EXPORT QtTimePropertyManager : public QtAbstractPropertyManager
{
    Q_OBJECT
public:
    QtTimePropertyManager(QObject *parent = nullptr);
    ~QtTimePropertyManager();

    QTime value(const QtProperty *property) const;

public Q_SLOTS:
    void setValue(QtProperty *property, const QTime &val);
Q_SIGNALS:
    void valueChanged(QtProperty *property, const QTime &val);
protected:
    QString valueText(const QtProperty *property) const;
    virtual void initializeProperty(QtProperty *property);
    virtual void uninitializeProperty(QtProperty *property);
private:
    QtTimePropertyManagerPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtTimePropertyManager)
    Q_DISABLE_COPY(QtTimePropertyManager)
};

#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif

#endif


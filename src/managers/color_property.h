
#ifndef QtColorPropertyManager_H
#define QtColorPropertyManager_H

#include "../qtpropertybrowser.h"
#include "int_property.h"

#if QT_VERSION >= 0x040400
QT_BEGIN_NAMESPACE
#endif

class QtColorPropertyManagerPrivate;

class QT_QTPROPERTYBROWSER_EXPORT QtColorPropertyManager : public QtAbstractPropertyManager
{
    Q_OBJECT
public:
    QtColorPropertyManager(QObject *parent = nullptr);
    ~QtColorPropertyManager();

    QtIntPropertyManager *subIntPropertyManager() const;

    QColor value(const QtProperty *property) const;

public Q_SLOTS:
    void setValue(QtProperty *property, const QColor &val);
Q_SIGNALS:
    void valueChanged(QtProperty *property, const QColor &val);
protected:
    QString valueText(const QtProperty *property) const;
    QIcon valueIcon(const QtProperty *property) const;
    virtual void initializeProperty(QtProperty *property);
    virtual void uninitializeProperty(QtProperty *property);
private:
    QtColorPropertyManagerPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtColorPropertyManager)
    Q_DISABLE_COPY(QtColorPropertyManager)

private Q_SLOTS:
    void slotIntChanged(QtProperty *, int);
    void slotPropertyDestroyed(QtProperty *);
};

#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif

#endif


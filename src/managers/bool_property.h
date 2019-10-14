
#ifndef QtBoolPropertyManager_H
#define QtBoolPropertyManager_H

#include "../qtpropertybrowser.h"

#if QT_VERSION >= 0x040400
QT_BEGIN_NAMESPACE
#endif

class QtBoolPropertyManagerPrivate;

class QT_QTPROPERTYBROWSER_EXPORT QtBoolPropertyManager : public QtAbstractPropertyManager
{
    Q_OBJECT
public:
    QtBoolPropertyManager(QObject *parent = 0);
    ~QtBoolPropertyManager();

    bool value(const QtProperty *property) const;
    bool textVisible(const QtProperty *property) const;

public Q_SLOTS:
    void setValue(QtProperty *property, bool val);
    void setTextVisible(QtProperty *property, bool textVisible);
Q_SIGNALS:
    void valueChanged(QtProperty *property, bool val);
    void textVisibleChanged(QtProperty *property, bool textVisible);
protected:
    QString valueText(const QtProperty *property) const;
    QIcon valueIcon(const QtProperty *property) const;
    virtual void initializeProperty(QtProperty *property);
    virtual void uninitializeProperty(QtProperty *property);
private:
    QtBoolPropertyManagerPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtBoolPropertyManager)
    Q_DISABLE_COPY(QtBoolPropertyManager)
};


#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif

#endif



#ifndef QtStringPropertyManager_H
#define QtStringPropertyManager_H

#include "../qtpropertybrowser.h"

#if QT_VERSION >= 0x040400
QT_BEGIN_NAMESPACE
#endif

class QtStringPropertyManagerPrivate;

class QT_QTPROPERTYBROWSER_EXPORT QtStringPropertyManager : public QtAbstractPropertyManager
{
    Q_OBJECT
public:
    QtStringPropertyManager(QObject *parent = nullptr);
    ~QtStringPropertyManager();

    QString value(const QtProperty *property) const;
    QRegExp regExp(const QtProperty *property) const;
    int maxLength(const QtProperty *property) const;

public Q_SLOTS:
    void setValue(QtProperty *property, const QString &val);
    void setRegExp(QtProperty *property, const QRegExp &regExp);
    void setMaxLength(QtProperty *property, int maxlen);
Q_SIGNALS:
    void valueChanged(QtProperty *property, const QString &val);
    void regExpChanged(QtProperty *property, const QRegExp &regExp);
    void maxLenChanged(QtProperty *property, int maxlen);
protected:
    QString valueText(const QtProperty *property) const;
    virtual void initializeProperty(QtProperty *property);
    virtual void uninitializeProperty(QtProperty *property);
private:
    QtStringPropertyManagerPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtStringPropertyManager)
    Q_DISABLE_COPY(QtStringPropertyManager)
};

#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif

#endif


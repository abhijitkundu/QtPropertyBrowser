
#ifndef QtColorPropertyManager_H
#define QtColorPropertyManager_H

#include "../qtpropertybrowser.h"
#include "../editor_factory_private.hpp"
#include "int_property.h"
#include "string_property.h"

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
    QtStringPropertyManager *subStringPropertyManager() const;

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
    void slotStringChanged(QtProperty *, const QString &val);
//    void slotIntChanged(QtProperty *, int);
    void slotPropertyDestroyed(QtProperty *);
};



// QtColorEditWidget

class QLabel;
class QLineEdit;
class QToolButton;

class QtColorEditWidget : public QWidget {
    Q_OBJECT

public:
    QtColorEditWidget(QWidget *parent);

    bool eventFilter(QObject *obj, QEvent *ev);

public Q_SLOTS:
    void setValue(const QColor &value);

Q_SIGNALS:
    void valueChanged(const QColor &value);

protected:
    void paintEvent(QPaintEvent *);

private Q_SLOTS:
    void buttonClicked();
    void editFinished();

private:
    QColor m_color;
    QLabel *m_pixmapLabel;
    QLabel *m_label;
    QLineEdit *m_lineEdit;
    QToolButton *m_button;
};




class QtColorEditorFactoryPrivate;

class QT_QTPROPERTYBROWSER_EXPORT QtColorEditorFactory : public QtAbstractEditorFactory<QtColorPropertyManager>
{
    Q_OBJECT
public:
    QtColorEditorFactory(QObject *parent = nullptr);
    ~QtColorEditorFactory();
protected:
    void connectPropertyManager(QtColorPropertyManager *manager);
    QWidget *createEditor(QtColorPropertyManager *manager, QtProperty *property,
                QWidget *parent);
    void disconnectPropertyManager(QtColorPropertyManager *manager);
private:
    QtColorEditorFactoryPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtColorEditorFactory)
    Q_DISABLE_COPY(QtColorEditorFactory)
    Q_PRIVATE_SLOT(d_func(), void slotPropertyChanged(QtProperty *, const QColor &))
    Q_PRIVATE_SLOT(d_func(), void slotEditorDestroyed(QObject *))
    Q_PRIVATE_SLOT(d_func(), void slotSetValue(const QColor &))
};




// QtColorEditorFactoryPrivate
class QtColorEditorFactoryPrivate : public EditorFactoryPrivate<QtColorEditWidget>
{
    QtColorEditorFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtColorEditorFactory)
public:

    void slotPropertyChanged(QtProperty *property, const QColor &value);
    void slotSetValue(const QColor &value);
};



#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif

#endif


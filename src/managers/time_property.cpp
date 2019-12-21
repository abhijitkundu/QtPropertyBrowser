
#include "time_property.h"
#include "../qtpropertybrowserutils_p.h"
#include "common.h"

#include <QtCore/QDateTime>
#include <QStyleOption>
#include <QStyle>
#include <QPainter>
#include <QLabel>
#include <QCheckBox>
#include <QApplication>

#if defined(Q_CC_MSVC)
#    pragma warning(disable: 4786) /* MS VS 6: truncating debug info after 255 characters */
#endif

#if QT_VERSION >= 0x040400
QT_BEGIN_NAMESPACE
#endif


// QtTimePropertyManager

class QtTimePropertyManagerPrivate
{
    QtTimePropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtTimePropertyManager)
public:

    QString m_format;

    typedef QMap<const QtProperty *, QTime> PropertyValueMap;
    PropertyValueMap m_values;
};

/*!
    \class QtTimePropertyManager

    \brief The QtTimePropertyManager provides and manages QTime properties.

    A time property's value can be retrieved using the value()
    function, and set using the setValue() slot.

    In addition, QtTimePropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes.

    \sa QtAbstractPropertyManager, QtTimeEditFactory
*/

/*!
    \fn void QtTimePropertyManager::valueChanged(QtProperty *property, const QTime &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the
    new \a value as parameters.

    \sa setValue()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtTimePropertyManager::QtTimePropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtTimePropertyManagerPrivate;
    d_ptr->q_ptr = this;

    QLocale loc;
    d_ptr->m_format = loc.timeFormat(QLocale::ShortFormat);
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtTimePropertyManager::~QtTimePropertyManager()
{
    clear();
    delete d_ptr;
}

/*!
    Returns the given \a property's value.

    If the given property is not managed by this manager, this
    function returns an invalid time object.

    \sa setValue()
*/
QTime QtTimePropertyManager::value(const QtProperty *property) const
{
    return d_ptr->m_values.value(property, QTime());
}

/*!
    \reimp
*/
QString QtTimePropertyManager::valueText(const QtProperty *property) const
{
   const QtTimePropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QString();
    return it.value().toString(d_ptr->m_format);
}

/*!
    \fn void QtTimePropertyManager::setValue(QtProperty *property, const QTime &value)

    Sets the value of the given \a property to \a value.

    \sa value(), valueChanged()
*/
void QtTimePropertyManager::setValue(QtProperty *property, const QTime &val)
{
    setSimpleValue<const QTime &, QTime, QtTimePropertyManager>(d_ptr->m_values, this,
                &QtTimePropertyManager::propertyChanged,
                &QtTimePropertyManager::valueChanged,
                property, val);
}

/*!
    \reimp
*/
void QtTimePropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QTime::currentTime();
}

/*!
    \reimp
*/
void QtTimePropertyManager::uninitializeProperty(QtProperty *property)
{
    d_ptr->m_values.remove(property);
}



#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif



#include "datetime_property.h"
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

// QtDateTimePropertyManager

class QtDateTimePropertyManagerPrivate
{
    QtDateTimePropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtDateTimePropertyManager)
public:

    QString m_format;

    typedef QMap<const QtProperty *, QDateTime> PropertyValueMap;
    PropertyValueMap m_values;
};

/*! \class QtDateTimePropertyManager

    \brief The QtDateTimePropertyManager provides and manages QDateTime properties.

    A date and time property has a current value which can be
    retrieved using the value() function, and set using the setValue()
    slot. In addition, QtDateTimePropertyManager provides the
    valueChanged() signal which is emitted whenever a property created
    by this manager changes.

    \sa QtAbstractPropertyManager, QtDateTimeEditFactory, QtDatePropertyManager
*/

/*!
    \fn void QtDateTimePropertyManager::valueChanged(QtProperty *property, const QDateTime &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.
*/

/*!
    Creates a manager with the given \a parent.
*/
QtDateTimePropertyManager::QtDateTimePropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtDateTimePropertyManagerPrivate;
    d_ptr->q_ptr = this;

    QLocale loc;
    d_ptr->m_format = loc.dateFormat(QLocale::ShortFormat);
    d_ptr->m_format += QLatin1Char(' ');
    d_ptr->m_format += loc.timeFormat(QLocale::ShortFormat);
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtDateTimePropertyManager::~QtDateTimePropertyManager()
{
    clear();
    delete d_ptr;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by this manager, this
    function returns an invalid QDateTime object.

    \sa setValue()
*/
QDateTime QtDateTimePropertyManager::value(const QtProperty *property) const
{
    return d_ptr->m_values.value(property, QDateTime());
}

/*!
    \reimp
*/
QString QtDateTimePropertyManager::valueText(const QtProperty *property) const
{
   const QtDateTimePropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QString();
    return it.value().toString(d_ptr->m_format);
}

/*!
    \fn void QtDateTimePropertyManager::setValue(QtProperty *property, const QDateTime &value)

    Sets the value of the given \a property to \a value.

    \sa value(), valueChanged()
*/
void QtDateTimePropertyManager::setValue(QtProperty *property, const QDateTime &val)
{
    setSimpleValue<const QDateTime &, QDateTime, QtDateTimePropertyManager>(d_ptr->m_values, this,
                &QtDateTimePropertyManager::propertyChanged,
                &QtDateTimePropertyManager::valueChanged,
                property, val);
}

/*!
    \reimp
*/
void QtDateTimePropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QDateTime::currentDateTime();
}

/*!
    \reimp
*/
void QtDateTimePropertyManager::uninitializeProperty(QtProperty *property)
{
    d_ptr->m_values.remove(property);
}


#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif


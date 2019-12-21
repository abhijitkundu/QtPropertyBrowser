
#include "date_property.h"
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



// QtDatePropertyManager

class QtDatePropertyManagerPrivate
{
    QtDatePropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtDatePropertyManager)
public:

    struct Data
    {
        Data() : val(QDate::currentDate()), minVal(QDate(1752, 9, 14)),
                maxVal(QDate(7999, 12, 31)) {}
        QDate val;
        QDate minVal;
        QDate maxVal;
        QDate minimumValue() const { return minVal; }
        QDate maximumValue() const { return maxVal; }
        void setMinimumValue(const QDate &newMinVal) { setSimpleMinimumData(this, newMinVal); }
        void setMaximumValue(const QDate &newMaxVal) { setSimpleMaximumData(this, newMaxVal); }
    };

    QString m_format;

    typedef QMap<const QtProperty *, Data> PropertyValueMap;
    QMap<const QtProperty *, Data> m_values;
};

/*!
    \class QtDatePropertyManager

    \brief The QtDatePropertyManager provides and manages QDate properties.

    A date property has a current value, and a range specifying the
    valid dates. The range is defined by a minimum and a maximum
    value.

    The property's values can be retrieved using the minimum(),
    maximum() and value() functions, and can be set using the
    setMinimum(), setMaximum() and setValue() slots. Alternatively,
    the range can be defined in one go using the setRange() slot.

    In addition, QtDatePropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes, and the rangeChanged() signal which is emitted whenever
    such a property changes its range of valid dates.

    \sa QtAbstractPropertyManager, QtDateEditFactory, QtDateTimePropertyManager
*/

/*!
    \fn void QtDatePropertyManager::valueChanged(QtProperty *property, const QDate &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.

    \sa setValue()
*/

/*!
    \fn void QtDatePropertyManager::rangeChanged(QtProperty *property, const QDate &minimum, const QDate &maximum)

    This signal is emitted whenever a property created by this manager
    changes its range of valid dates, passing a pointer to the \a
    property and the new \a minimum and \a maximum dates.

    \sa setRange()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtDatePropertyManager::QtDatePropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtDatePropertyManagerPrivate;
    d_ptr->q_ptr = this;

    QLocale loc;
    d_ptr->m_format = loc.dateFormat(QLocale::ShortFormat);
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtDatePropertyManager::~QtDatePropertyManager()
{
    clear();
    delete d_ptr;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by \e this manager, this
    function returns an invalid date.

    \sa setValue()
*/
QDate QtDatePropertyManager::value(const QtProperty *property) const
{
    return getValue<QDate>(d_ptr->m_values, property);
}

/*!
    Returns the given \a  property's  minimum date.

    \sa maximum(), setRange()
*/
QDate QtDatePropertyManager::minimum(const QtProperty *property) const
{
    return getMinimum<QDate>(d_ptr->m_values, property);
}

/*!
    Returns the given \a property's maximum date.

    \sa minimum(), setRange()
*/
QDate QtDatePropertyManager::maximum(const QtProperty *property) const
{
    return getMaximum<QDate>(d_ptr->m_values, property);
}

/*!
    \reimp
*/
QString QtDatePropertyManager::valueText(const QtProperty *property) const
{
    const QtDatePropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QString();
    return it.value().val.toString(d_ptr->m_format);
}

/*!
    \fn void QtDatePropertyManager::setValue(QtProperty *property, const QDate &value)

    Sets the value of the given \a property to \a value.

    If the specified \a value is not a valid date according to the
    given \a property's range, the value is adjusted to the nearest
    valid value within the range.

    \sa value(), setRange(), valueChanged()
*/
void QtDatePropertyManager::setValue(QtProperty *property, const QDate &val)
{
    void (QtDatePropertyManagerPrivate::*setSubPropertyValue)(QtProperty *, const QDate &) = nullptr;
    setValueInRange<const QDate &, QtDatePropertyManagerPrivate, QtDatePropertyManager, const QDate>(this, d_ptr,
                &QtDatePropertyManager::propertyChanged,
                &QtDatePropertyManager::valueChanged,
                property, val, setSubPropertyValue);
}

/*!
    Sets the minimum value for the given \a property to \a minVal.

    When setting the minimum value, the maximum and current values are
    adjusted if necessary (ensuring that the range remains valid and
    that the current value is within in the range).

    \sa minimum(), setRange()
*/
void QtDatePropertyManager::setMinimum(QtProperty *property, const QDate &minVal)
{
    setMinimumValue<const QDate &, QtDatePropertyManagerPrivate, QtDatePropertyManager, QDate, QtDatePropertyManagerPrivate::Data>(this, d_ptr,
                &QtDatePropertyManager::propertyChanged,
                &QtDatePropertyManager::valueChanged,
                &QtDatePropertyManager::rangeChanged,
                property, minVal);
}

/*!
    Sets the maximum value for the given \a property to \a maxVal.

    When setting the maximum value, the minimum and current
    values are adjusted if necessary (ensuring that the range remains
    valid and that the current value is within in the range).

    \sa maximum(), setRange()
*/
void QtDatePropertyManager::setMaximum(QtProperty *property, const QDate &maxVal)
{
    setMaximumValue<const QDate &, QtDatePropertyManagerPrivate, QtDatePropertyManager, QDate, QtDatePropertyManagerPrivate::Data>(this, d_ptr,
                &QtDatePropertyManager::propertyChanged,
                &QtDatePropertyManager::valueChanged,
                &QtDatePropertyManager::rangeChanged,
                property, maxVal);
}

/*!
    \fn void QtDatePropertyManager::setRange(QtProperty *property, const QDate &minimum, const QDate &maximum)

    Sets the range of valid dates.

    This is a convenience function defining the range of valid dates
    in one go; setting the \a minimum and \a maximum values for the
    given \a property with a single function call.

    When setting a new date range, the current value is adjusted if
    necessary (ensuring that the value remains in date range).

    \sa setMinimum(), setMaximum(), rangeChanged()
*/
void QtDatePropertyManager::setRange(QtProperty *property, const QDate &minVal, const QDate &maxVal)
{
    void (QtDatePropertyManagerPrivate::*setSubPropertyRange)(QtProperty *, const QDate &,
          const QDate &, const QDate &) = nullptr;
    setBorderValues<const QDate &, QtDatePropertyManagerPrivate, QtDatePropertyManager, QDate>(this, d_ptr,
                &QtDatePropertyManager::propertyChanged,
                &QtDatePropertyManager::valueChanged,
                &QtDatePropertyManager::rangeChanged,
                property, minVal, maxVal, setSubPropertyRange);
}

/*!
    \reimp
*/
void QtDatePropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtDatePropertyManagerPrivate::Data();
}

/*!
    \reimp
*/
void QtDatePropertyManager::uninitializeProperty(QtProperty *property)
{
    d_ptr->m_values.remove(property);
}



#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif


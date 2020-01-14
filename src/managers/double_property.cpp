
#include "double_property.h"
#include "../qtpropertybrowserutils_p.h"
#include "common.h"

#include <QObject>


#if defined(Q_CC_MSVC)
#    pragma warning(disable: 4786) /* MS VS 6: truncating debug info after 255 characters */
#endif

#if QT_VERSION >= 0x040400
QT_BEGIN_NAMESPACE
#endif

// QtDoublePropertyManager

class QtDoublePropertyManagerPrivate
{
    QtDoublePropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtDoublePropertyManager)
public:

    struct Data
    {
        Data() : val(0), minVal(-INT_MAX), maxVal(INT_MAX), singleStep(1), decimals(2) {}
        double val;
        double minVal;
        double maxVal;
        double singleStep;
        int decimals;
        double minimumValue() const { return minVal; }
        double maximumValue() const { return maxVal; }
        void setMinimumValue(double newMinVal) { setSimpleMinimumData(this, newMinVal); }
        void setMaximumValue(double newMaxVal) { setSimpleMaximumData(this, newMaxVal); }
    };

    typedef QMap<const QtProperty *, Data> PropertyValueMap;
    PropertyValueMap m_values;
};

/*!
    \class QtDoublePropertyManager

    \brief The QtDoublePropertyManager provides and manages double properties.

    A double property has a current value, and a range specifying the
    valid values. The range is defined by a minimum and a maximum
    value.

    The property's value and range can be retrieved using the value(),
    minimum() and maximum() functions, and can be set using the
    setValue(), setMinimum() and setMaximum() slots.
    Alternatively, the range can be defined in one go using the
    setRange() slot.

    In addition, QtDoublePropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes, and the rangeChanged() signal which is emitted whenever
    such a property changes its range of valid values.

    \sa QtAbstractPropertyManager, QtDoubleSpinBoxFactory
*/

/*!
    \fn void QtDoublePropertyManager::valueChanged(QtProperty *property, double value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.

    \sa setValue()
*/

/*!
    \fn void QtDoublePropertyManager::rangeChanged(QtProperty *property, double minimum, double maximum)

    This signal is emitted whenever a property created by this manager
    changes its range of valid values, passing a pointer to the
    \a property and the new \a minimum and \a maximum values

    \sa setRange()
*/

/*!
    \fn void QtDoublePropertyManager::decimalsChanged(QtProperty *property, int prec)

    This signal is emitted whenever a property created by this manager
    changes its precision of value, passing a pointer to the
    \a property and the new \a prec value

    \sa setDecimals()
*/

/*!
    \fn void QtDoublePropertyManager::singleStepChanged(QtProperty *property, double step)

    This signal is emitted whenever a property created by this manager
    changes its single step property, passing a pointer to the
    \a property and the new \a step value

    \sa setSingleStep()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtDoublePropertyManager::QtDoublePropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtDoublePropertyManagerPrivate;
    d_ptr->q_ptr = this;
}

/*!
    Destroys  this manager, and all the properties it has created.
*/
QtDoublePropertyManager::~QtDoublePropertyManager()
{
    clear();
    delete d_ptr;
}

/*!
    Returns the given \a property's value.

    If the given property is not managed by this manager, this
    function returns 0.

    \sa setValue()
*/
double QtDoublePropertyManager::value(const QtProperty *property) const
{
    return getValue<double>(d_ptr->m_values, property, 0.0);
}

/*!
    Returns the given \a property's minimum value.

    \sa maximum(), setRange()
*/
double QtDoublePropertyManager::minimum(const QtProperty *property) const
{
    return getMinimum<double>(d_ptr->m_values, property, 0.0);
}

/*!
    Returns the given \a property's maximum value.

    \sa minimum(), setRange()
*/
double QtDoublePropertyManager::maximum(const QtProperty *property) const
{
    return getMaximum<double>(d_ptr->m_values, property, 0.0);
}

/*!
    Returns the given \a property's step value.

    The step is typically used to increment or decrement a property value while pressing an arrow key.

    \sa setSingleStep()
*/
double QtDoublePropertyManager::singleStep(const QtProperty *property) const
{
    return getData<double>(d_ptr->m_values, &QtDoublePropertyManagerPrivate::Data::singleStep, property, 0);
}

/*!
    Returns the given \a property's precision, in decimals.

    \sa setDecimals()
*/
int QtDoublePropertyManager::decimals(const QtProperty *property) const
{
    return getData<int>(d_ptr->m_values, &QtDoublePropertyManagerPrivate::Data::decimals, property, 0);
}

/*!
    \reimp
*/
QString QtDoublePropertyManager::valueText(const QtProperty *property) const
{
    const QtDoublePropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QString();
    return QString::number(it.value().val, 'f', it.value().decimals);
}

/*!
    \fn void QtDoublePropertyManager::setValue(QtProperty *property, double value)

    Sets the value of the given \a property to \a value.

    If the specified \a value is not valid according to the given
    \a property's range, the \a value is adjusted to the nearest valid value
    within the range.

    \sa value(), setRange(), valueChanged()
*/
void QtDoublePropertyManager::setValue(QtProperty *property, double val)
{
    void (QtDoublePropertyManagerPrivate::*setSubPropertyValue)(QtProperty *, double) = nullptr;
    setValueInRange<double, QtDoublePropertyManagerPrivate, QtDoublePropertyManager, double>(this, d_ptr,
                &QtDoublePropertyManager::propertyChanged,
                &QtDoublePropertyManager::valueChanged,
                property, val, setSubPropertyValue);
}

/*!
    Sets the step value for the given \a property to \a step.

    The step is typically used to increment or decrement a property value while pressing an arrow key.

    \sa singleStep()
*/
void QtDoublePropertyManager::setSingleStep(QtProperty *property, double step)
{
    const QtDoublePropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    QtDoublePropertyManagerPrivate::Data data = it.value();

    if (step < 0)
        step = 0;

    if (floatEqual(data.singleStep, step))
        return;

    data.singleStep = step;

    it.value() = data;

    emit singleStepChanged(property, data.singleStep);
}

/*!
    \fn void QtDoublePropertyManager::setDecimals(QtProperty *property, int prec)

    Sets the precision of the given \a property to \a prec.

    The valid decimal range is 0-13. The default is 2.

    \sa decimals()
*/
void QtDoublePropertyManager::setDecimals(QtProperty *property, int prec)
{
    const QtDoublePropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    QtDoublePropertyManagerPrivate::Data data = it.value();

    if (prec > 13)
        prec = 13;
    else if (prec < 0)
        prec = 0;

    if (data.decimals == prec)
        return;

    data.decimals = prec;

    it.value() = data;

    emit decimalsChanged(property, data.decimals);
}

/*!
    Sets the minimum value for the given \a property to \a minVal.

    When setting the minimum value, the maximum and current values are
    adjusted if necessary (ensuring that the range remains valid and
    that the current value is within in the range).

    \sa minimum(), setRange(), rangeChanged()
*/
void QtDoublePropertyManager::setMinimum(QtProperty *property, double minVal)
{
    setMinimumValue<double, QtDoublePropertyManagerPrivate, QtDoublePropertyManager, double, QtDoublePropertyManagerPrivate::Data>(this, d_ptr,
                &QtDoublePropertyManager::propertyChanged,
                &QtDoublePropertyManager::valueChanged,
                &QtDoublePropertyManager::rangeChanged,
                property, minVal);
}

/*!
    Sets the maximum value for the given \a property to \a maxVal.

    When setting the maximum value, the minimum and current values are
    adjusted if necessary (ensuring that the range remains valid and
    that the current value is within in the range).

    \sa maximum(), setRange(), rangeChanged()
*/
void QtDoublePropertyManager::setMaximum(QtProperty *property, double maxVal)
{
    setMaximumValue<double, QtDoublePropertyManagerPrivate, QtDoublePropertyManager, double, QtDoublePropertyManagerPrivate::Data>(this, d_ptr,
                &QtDoublePropertyManager::propertyChanged,
                &QtDoublePropertyManager::valueChanged,
                &QtDoublePropertyManager::rangeChanged,
                property, maxVal);
}

/*!
    \fn void QtDoublePropertyManager::setRange(QtProperty *property, double minimum, double maximum)

    Sets the range of valid values.

    This is a convenience function defining the range of valid values
    in one go; setting the \a minimum and \a maximum values for the
    given \a property with a single function call.

    When setting a new range, the current value is adjusted if
    necessary (ensuring that the value remains within range).

    \sa setMinimum(), setMaximum(), rangeChanged()
*/
void QtDoublePropertyManager::setRange(QtProperty *property, double minVal, double maxVal)
{
    void (QtDoublePropertyManagerPrivate::*setSubPropertyRange)(QtProperty *, double, double, double) = nullptr;
    setBorderValues<double, QtDoublePropertyManagerPrivate, QtDoublePropertyManager, double>(this, d_ptr,
                &QtDoublePropertyManager::propertyChanged,
                &QtDoublePropertyManager::valueChanged,
                &QtDoublePropertyManager::rangeChanged,
                property, minVal, maxVal, setSubPropertyRange);
}

/*!
    \reimp
*/
void QtDoublePropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtDoublePropertyManagerPrivate::Data();
}

/*!
    \reimp
*/
void QtDoublePropertyManager::uninitializeProperty(QtProperty *property)
{
    d_ptr->m_values.remove(property);
}

#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif



#include "int_property.h"
#include "../qtpropertybrowserutils_p.h"
#include "common.h"

#include <QObject>


#if defined(Q_CC_MSVC)
#    pragma warning(disable: 4786) /* MS VS 6: truncating debug info after 255 characters */
#endif

#if QT_VERSION >= 0x040400
QT_BEGIN_NAMESPACE
#endif


// QtIntPropertyManager

class QtIntPropertyManagerPrivate
{
    QtIntPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtIntPropertyManager)
public:

    struct Data
    {
        Data() : val(0), minVal(-INT_MAX), maxVal(INT_MAX), singleStep(1) {}
        int val;
        int minVal;
        int maxVal;
        int singleStep;
        int minimumValue() const { return minVal; }
        int maximumValue() const { return maxVal; }
        void setMinimumValue(int newMinVal) { setSimpleMinimumData(this, newMinVal); }
        void setMaximumValue(int newMaxVal) { setSimpleMaximumData(this, newMaxVal); }
    };

    typedef QMap<const QtProperty *, Data> PropertyValueMap;
    PropertyValueMap m_values;
};

/*!
    \class QtIntPropertyManager

    \brief The QtIntPropertyManager provides and manages int properties.

    An int property has a current value, and a range specifying the
    valid values. The range is defined by a minimum and a maximum
    value.

    The property's value and range can be retrieved using the value(),
    minimum() and maximum() functions, and can be set using the
    setValue(), setMinimum() and setMaximum() slots. Alternatively,
    the range can be defined in one go using the setRange() slot.

    In addition, QtIntPropertyManager provides the valueChanged() signal which
    is emitted whenever a property created by this manager changes,
    and the rangeChanged() signal which is emitted whenever such a
    property changes its range of valid values.

    \sa QtAbstractPropertyManager, QtSpinBoxFactory, QtSliderFactory, QtScrollBarFactory
*/

/*!
    \fn void QtIntPropertyManager::valueChanged(QtProperty *property, int value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.

    \sa setValue()
*/

/*!
    \fn void QtIntPropertyManager::rangeChanged(QtProperty *property, int minimum, int maximum)

    This signal is emitted whenever a property created by this manager
    changes its range of valid values, passing a pointer to the
    \a property and the new \a minimum and \a maximum values.

    \sa setRange()
*/

/*!
    \fn void QtIntPropertyManager::singleStepChanged(QtProperty *property, int step)

    This signal is emitted whenever a property created by this manager
    changes its single step property, passing a pointer to the
    \a property and the new \a step value

    \sa setSingleStep()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtIntPropertyManager::QtIntPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtIntPropertyManagerPrivate;
    d_ptr->q_ptr = this;
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtIntPropertyManager::~QtIntPropertyManager()
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
int QtIntPropertyManager::value(const QtProperty *property) const
{
    return getValue<int>(d_ptr->m_values, property, 0);
}

/*!
    Returns the given \a property's minimum value.

    \sa setMinimum(), maximum(), setRange()
*/
int QtIntPropertyManager::minimum(const QtProperty *property) const
{
    return getMinimum<int>(d_ptr->m_values, property, 0);
}

/*!
    Returns the given \a property's maximum value.

    \sa setMaximum(), minimum(), setRange()
*/
int QtIntPropertyManager::maximum(const QtProperty *property) const
{
    return getMaximum<int>(d_ptr->m_values, property, 0);
}

/*!
    Returns the given \a property's step value.

    The step is typically used to increment or decrement a property value while pressing an arrow key.

    \sa setSingleStep()
*/
int QtIntPropertyManager::singleStep(const QtProperty *property) const
{
    return getData<int>(d_ptr->m_values, &QtIntPropertyManagerPrivate::Data::singleStep, property, 0);
}

/*!
    \reimp
*/
QString QtIntPropertyManager::valueText(const QtProperty *property) const
{
    const QtIntPropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QString();
    return QString::number(it.value().val);
}

/*!
    \fn void QtIntPropertyManager::setValue(QtProperty *property, int value)

    Sets the value of the given \a property to \a value.

    If the specified \a value is not valid according to the given \a
    property's range, the \a value is adjusted to the nearest valid
    value within the range.

    \sa value(), setRange(), valueChanged()
*/
void QtIntPropertyManager::setValue(QtProperty *property, int val)
{
    void (QtIntPropertyManagerPrivate::*setSubPropertyValue)(QtProperty *, int) = nullptr;
    setValueInRange<int, QtIntPropertyManagerPrivate, QtIntPropertyManager, int>(this, d_ptr,
                &QtIntPropertyManager::propertyChanged,
                &QtIntPropertyManager::valueChanged,
                property, val, setSubPropertyValue);
}

/*!
    Sets the minimum value for the given \a property to \a minVal.

    When setting the minimum value, the maximum and current values are
    adjusted if necessary (ensuring that the range remains valid and
    that the current value is within the range).

    \sa minimum(), setRange(), rangeChanged()
*/
void QtIntPropertyManager::setMinimum(QtProperty *property, int minVal)
{
    setMinimumValue<int, QtIntPropertyManagerPrivate, QtIntPropertyManager, int, QtIntPropertyManagerPrivate::Data>(this, d_ptr,
                &QtIntPropertyManager::propertyChanged,
                &QtIntPropertyManager::valueChanged,
                &QtIntPropertyManager::rangeChanged,
                property, minVal);
}

/*!
    Sets the maximum value for the given \a property to \a maxVal.

    When setting maximum value, the minimum and current values are
    adjusted if necessary (ensuring that the range remains valid and
    that the current value is within the range).

    \sa maximum(), setRange(), rangeChanged()
*/
void QtIntPropertyManager::setMaximum(QtProperty *property, int maxVal)
{
    setMaximumValue<int, QtIntPropertyManagerPrivate, QtIntPropertyManager, int, QtIntPropertyManagerPrivate::Data>(this, d_ptr,
                &QtIntPropertyManager::propertyChanged,
                &QtIntPropertyManager::valueChanged,
                &QtIntPropertyManager::rangeChanged,
                property, maxVal);
}

/*!
    \fn void QtIntPropertyManager::setRange(QtProperty *property, int minimum, int maximum)

    Sets the range of valid values.

    This is a convenience function defining the range of valid values
    in one go; setting the \a minimum and \a maximum values for the
    given \a property with a single function call.

    When setting a new range, the current value is adjusted if
    necessary (ensuring that the value remains within range).

    \sa setMinimum(), setMaximum(), rangeChanged()
*/
void QtIntPropertyManager::setRange(QtProperty *property, int minVal, int maxVal)
{
    void (QtIntPropertyManagerPrivate::*setSubPropertyRange)(QtProperty *, int, int, int) = nullptr;
    setBorderValues<int, QtIntPropertyManagerPrivate, QtIntPropertyManager, int>(this, d_ptr,
                &QtIntPropertyManager::propertyChanged,
                &QtIntPropertyManager::valueChanged,
                &QtIntPropertyManager::rangeChanged,
                property, minVal, maxVal, setSubPropertyRange);
}

/*!
    Sets the step value for the given \a property to \a step.

    The step is typically used to increment or decrement a property value while pressing an arrow key.

    \sa singleStep()
*/
void QtIntPropertyManager::setSingleStep(QtProperty *property, int step)
{
    const QtIntPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    QtIntPropertyManagerPrivate::Data data = it.value();

    if (step < 0)
        step = 0;

    if (data.singleStep == step)
        return;

    data.singleStep = step;

    it.value() = data;

    emit singleStepChanged(property, data.singleStep);
}

/*!
    \reimp
*/
void QtIntPropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtIntPropertyManagerPrivate::Data();
}

/*!
    \reimp
*/
void QtIntPropertyManager::uninitializeProperty(QtProperty *property)
{
    d_ptr->m_values.remove(property);
}

#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif



#include "string_property.h"
#include "../qtpropertybrowserutils_p.h"
#include "common.h"

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


// QtStringPropertyManager

class QtStringPropertyManagerPrivate
{
    QtStringPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtStringPropertyManager)
public:

    struct Data
    {
        Data() : regExp(QString(QLatin1Char('*')),  Qt::CaseSensitive, QRegExp::Wildcard), maxLen(-1)
        {
        }
        QString val;
        QRegExp regExp;
        int maxLen;
    };

    typedef QMap<const QtProperty *, Data> PropertyValueMap;
    QMap<const QtProperty *, Data> m_values;
};

/*!
    \class QtStringPropertyManager

    \brief The QtStringPropertyManager provides and manages QString properties.

    A string property's value can be retrieved using the value()
    function, and set using the setValue() slot.

    The current value can be checked against a regular expression. To
    set the regular expression use the setRegExp() slot, use the
    regExp() function to retrieve the currently set expression.

    In addition, QtStringPropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes, and the regExpChanged() signal which is emitted whenever
    such a property changes its currently set regular expression.

    \sa QtAbstractPropertyManager, QtLineEditFactory
*/

/*!
    \fn void QtStringPropertyManager::valueChanged(QtProperty *property, const QString &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the
    new \a value as parameters.

    \sa setValue()
*/

/*!
    \fn void QtStringPropertyManager::regExpChanged(QtProperty *property, const QRegExp &regExp)

    This signal is emitted whenever a property created by this manager
    changes its currenlty set regular expression, passing a pointer to
    the \a property and the new \a regExp as parameters.

    \sa setRegExp()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtStringPropertyManager::QtStringPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtStringPropertyManagerPrivate;
    d_ptr->q_ptr = this;
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtStringPropertyManager::~QtStringPropertyManager()
{
    clear();
    delete d_ptr;
}

/*!
    Returns the given \a property's value.

    If the given property is not managed by this manager, this
    function returns an empty string.

    \sa setValue()
*/
QString QtStringPropertyManager::value(const QtProperty *property) const
{
    return getValue<QString>(d_ptr->m_values, property);
}

/*!
    Returns the given \a property's currently set regular expression.

    If the given \a property is not managed by this manager, this
    function returns an empty expression.

    \sa setRegExp()
*/
QRegExp QtStringPropertyManager::regExp(const QtProperty *property) const
{
    return getData<QRegExp>(d_ptr->m_values, &QtStringPropertyManagerPrivate::Data::regExp, property, QRegExp());
}

/*!
    Returns the given \a property's currently set maximum length.

    If the given \a property is not managed by this manager, this
    function returns an empty expression.

    \sa setMaxLength()
*/
int QtStringPropertyManager::maxLength(const QtProperty *property) const
{
    return getData<int>(d_ptr->m_values, &QtStringPropertyManagerPrivate::Data::maxLen, property, -1);
}

/*!
    \reimp
*/
QString QtStringPropertyManager::valueText(const QtProperty *property) const
{
    const QtStringPropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QString();
    return it.value().val;
}

/*!
    \fn void QtStringPropertyManager::setValue(QtProperty *property, const QString &value)

    Sets the value of the given \a property to \a value.

    If the specified \a value doesn't match the given \a property's
    regular expression, this function does nothing.

    \sa value(), setRegExp(), valueChanged()
*/
void QtStringPropertyManager::setValue(QtProperty *property, const QString &val)
{
    const QtStringPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    QtStringPropertyManagerPrivate::Data data = it.value();

    if (data.val == val)
        return;

    if ((data.maxLen > 0) && (val.length() > data.maxLen))
        return;

    if (data.regExp.isValid() && !data.regExp.exactMatch(val))
        return;

    data.val = val;

    it.value() = data;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    Sets the regular expression of the given \a property to \a regExp.

    \sa regExp(), setValue(), regExpChanged()
*/
void QtStringPropertyManager::setRegExp(QtProperty *property, const QRegExp &regExp)
{
    const QtStringPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    QtStringPropertyManagerPrivate::Data data = it.value() ;

    if (data.regExp == regExp)
        return;

    data.regExp = regExp;

    it.value() = data;

    emit regExpChanged(property, data.regExp);
}

void QtStringPropertyManager::setMaxLength(QtProperty *property, int maxlen)
{
    const QtStringPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    QtStringPropertyManagerPrivate::Data data = it.value();

    if (data.maxLen == maxlen)
        return;

    if (maxlen < -1)
        return;

    data.maxLen = maxlen;

    it.value() = data;

    emit maxLenChanged(property, data.maxLen);
}

/*!
    \reimp
*/
void QtStringPropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtStringPropertyManagerPrivate::Data();
}

/*!
    \reimp
*/
void QtStringPropertyManager::uninitializeProperty(QtProperty *property)
{
    d_ptr->m_values.remove(property);
}


#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif



#include "group_property.h"

#include <QObject>


#if defined(Q_CC_MSVC)
#    pragma warning(disable: 4786) /* MS VS 6: truncating debug info after 255 characters */
#endif

#if QT_VERSION >= 0x040400
QT_BEGIN_NAMESPACE
#endif

// QtGroupPropertyManager

/*!
    \class QtGroupPropertyManager

    \brief The QtGroupPropertyManager provides and manages group properties.

    This class is intended to provide a grouping element without any value.

    \sa QtAbstractPropertyManager
*/

/*!
    Creates a manager with the given \a parent.
*/
QtGroupPropertyManager::QtGroupPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{

}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtGroupPropertyManager::~QtGroupPropertyManager()
{

}

/*!
    \reimp
*/
bool QtGroupPropertyManager::hasValue(const QtProperty *property) const
{
    Q_UNUSED(property)
    return false;
}

/*!
    \reimp
*/
void QtGroupPropertyManager::initializeProperty(QtProperty *property)
{
    Q_UNUSED(property)
}

/*!
    \reimp
*/
void QtGroupPropertyManager::uninitializeProperty(QtProperty *property)
{
    Q_UNUSED(property)
}


#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif



#include "bool_property.h"
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

// QtBoolPropertyManager

class QtBoolPropertyManagerPrivate
{
    QtBoolPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtBoolPropertyManager)
public:

    struct Data
    {
        Data() : val(false), textVisible(true) {}
        bool val;
        bool textVisible;
    };

    typedef QMap<const QtProperty *, Data> PropertyValueMap;
    PropertyValueMap m_values;
};

/*!
    \class QtBoolPropertyManager

    \brief The QtBoolPropertyManager class provides and manages boolean properties.

    The property's value can be retrieved using the value() function,
    and set using the setValue() slot.

    In addition, QtBoolPropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes.

    \sa QtAbstractPropertyManager, QtCheckBoxFactory
*/

/*!
    \fn void QtBoolPropertyManager::valueChanged(QtProperty *property, bool value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the
    new \a value as parameters.
*/

/*!
    Creates a manager with the given \a parent.
*/
QtBoolPropertyManager::QtBoolPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtBoolPropertyManagerPrivate;
    d_ptr->q_ptr = this;
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtBoolPropertyManager::~QtBoolPropertyManager()
{
    clear();
    delete d_ptr;
}

bool QtBoolPropertyManager::textVisible(const QtProperty *property) const
{
    return getData<bool>(d_ptr->m_values, &QtBoolPropertyManagerPrivate::Data::textVisible, property, true);
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by \e this manager, this
    function returns false.

    \sa setValue()
*/
bool QtBoolPropertyManager::value(const QtProperty *property) const
{
    return getValue<bool>(d_ptr->m_values, property, false);
}

void QtBoolPropertyManager::setTextVisible(QtProperty *property, bool textVisible)
{
    const QtBoolPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    QtBoolPropertyManagerPrivate::Data data = it.value();
    if (data.textVisible == textVisible)
        return;

    data.textVisible = textVisible;
    it.value() = data;
    emit textVisibleChanged(property, data.textVisible);
}

/*!
    \reimp
*/
QString QtBoolPropertyManager::valueText(const QtProperty *property) const
{
    const QtBoolPropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QString();

    static const QString trueText = tr("True");
    static const QString falseText = tr("False");
    return it.value().val ? trueText : falseText;
}

// Return an icon containing a check box indicator
static QIcon drawCheckBox(bool value)
{
    QStyleOptionButton opt;
    opt.state |= value ? QStyle::State_On : QStyle::State_Off;
    opt.state |= QStyle::State_Enabled;
    const QStyle *style = QApplication::style();
    // Figure out size of an indicator and make sure it is not scaled down in a list view item
    // by making the pixmap as big as a list view icon and centering the indicator in it.
    // (if it is smaller, it can't be helped)
    const int indicatorWidth = style->pixelMetric(QStyle::PM_IndicatorWidth, &opt);
    const int indicatorHeight = style->pixelMetric(QStyle::PM_IndicatorHeight, &opt);
    const int listViewIconSize = indicatorWidth;
    const int pixmapWidth = indicatorWidth;
    const int pixmapHeight = qMax(indicatorHeight, listViewIconSize);

    opt.rect = QRect(0, 0, indicatorWidth, indicatorHeight);
    QPixmap pixmap = QPixmap(pixmapWidth, pixmapHeight);
    pixmap.fill(Qt::transparent);
    {
        // Center?
        const int xoff = (pixmapWidth  > indicatorWidth)  ? (pixmapWidth  - indicatorWidth)  / 2 : 0;
        const int yoff = (pixmapHeight > indicatorHeight) ? (pixmapHeight - indicatorHeight) / 2 : 0;
        QPainter painter(&pixmap);
        painter.translate(xoff, yoff);
        QCheckBox cb;
        style->drawPrimitive(QStyle::PE_IndicatorCheckBox, &opt, &painter, &cb);
    }
    return QIcon(pixmap);
}

/*!
    \reimp
*/
QIcon QtBoolPropertyManager::valueIcon(const QtProperty *property) const
{
    const QtBoolPropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QIcon();

    static const QIcon checkedIcon = drawCheckBox(true);
    static const QIcon uncheckedIcon = drawCheckBox(false);
    return it.value().val ? checkedIcon : uncheckedIcon;
}

/*!
    \fn void QtBoolPropertyManager::setValue(QtProperty *property, bool value)

    Sets the value of the given \a property to \a value.

    \sa value()
*/
void QtBoolPropertyManager::setValue(QtProperty *property, bool val)
{
    QtBoolPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;
    QtBoolPropertyManagerPrivate::Data data = it.value();
    if (data.val == val)
        return;

    data.val = val;
    it.value() = data;
    emit valueChanged(property, data.val);
}

/*!
    \reimp
*/
void QtBoolPropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property].val = false;
}

/*!
    \reimp
*/
void QtBoolPropertyManager::uninitializeProperty(QtProperty *property)
{
    d_ptr->m_values.remove(property);
}

#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif


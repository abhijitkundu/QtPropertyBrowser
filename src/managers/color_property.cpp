
#include "color_property.h"
#include "../qtpropertybrowserutils_p.h"
#include "common.h"

#include <QStyleOption>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QColorDialog>
#include <QFontMetrics>
#include <QStyle>
#include <QPainter>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QCheckBox>
#include <QApplication>

#if defined(Q_CC_MSVC)
#    pragma warning(disable: 4786) /* MS VS 6: truncating debug info after 255 characters */
#endif

#if QT_VERSION >= 0x040400
QT_BEGIN_NAMESPACE
#endif


// QtColorPropertyManager

class QtColorPropertyManagerPrivate
{
    QtColorPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtColorPropertyManager)
public:

    void slotStringChanged(QtProperty *property, const QString &value);
//    void slotIntChanged(QtProperty *property, int value);
    void slotPropertyDestroyed(QtProperty *property);

    typedef QMap<const QtProperty *, QColor> PropertyValueMap;
    PropertyValueMap m_values;

//    QtIntPropertyManager *m_intPropertyManager;
    QtStringPropertyManager *m_stringPropertyManager;

//    QMap<const QtProperty *, QtProperty *> m_propertyToR;
//    QMap<const QtProperty *, QtProperty *> m_propertyToG;
//    QMap<const QtProperty *, QtProperty *> m_propertyToB;
//    QMap<const QtProperty *, QtProperty *> m_propertyToA;

//    QMap<const QtProperty *, QtProperty *> m_rToProperty;
//    QMap<const QtProperty *, QtProperty *> m_gToProperty;
//    QMap<const QtProperty *, QtProperty *> m_bToProperty;
//    QMap<const QtProperty *, QtProperty *> m_aToProperty;

    QMap<const QtProperty *, QtProperty *> m_propertyToRgba;
    QMap<const QtProperty *, QtProperty *> m_rgbaToProperty;
};

void QtColorPropertyManagerPrivate::slotStringChanged(QtProperty *property, const QString &value)
{
    if (QtProperty *prop = m_rgbaToProperty.value(property, nullptr)) {
        QColor c = m_values[prop];
        c.setNamedColor(value);
        q_ptr->setValue(prop, c);
    }
}

//void QtColorPropertyManagerPrivate::slotIntChanged(QtProperty *property, int value)
//{
//    if (QtProperty *prop = m_rToProperty.value(property, nullptr)) {
//        QColor c = m_values[prop];
//        c.setRed(value);
//        q_ptr->setValue(prop, c);
//    } else if (QtProperty *prop = m_gToProperty.value(property, nullptr)) {
//        QColor c = m_values[prop];
//        c.setGreen(value);
//        q_ptr->setValue(prop, c);
//    } else if (QtProperty *prop = m_bToProperty.value(property, nullptr)) {
//        QColor c = m_values[prop];
//        c.setBlue(value);
//        q_ptr->setValue(prop, c);
//    } else if (QtProperty *prop = m_aToProperty.value(property, nullptr)) {
//        QColor c = m_values[prop];
//        c.setAlpha(value);
//        q_ptr->setValue(prop, c);
//    }
//}

void QtColorPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (QtProperty *pointProp = m_rgbaToProperty.value(property, nullptr)) {
        m_propertyToRgba[pointProp] = nullptr;
        m_rgbaToProperty.remove(property);
    } /*else if (QtProperty *pointProp = m_rToProperty.value(property, nullptr)) {
        m_propertyToR[pointProp] = nullptr;
        m_rToProperty.remove(property);
    } else if (QtProperty *pointProp = m_gToProperty.value(property, nullptr)) {
        m_propertyToG[pointProp] = nullptr;
        m_gToProperty.remove(property);
    } else if (QtProperty *pointProp = m_bToProperty.value(property, nullptr)) {
        m_propertyToB[pointProp] = nullptr;
        m_bToProperty.remove(property);
    } else if (QtProperty *pointProp = m_aToProperty.value(property, nullptr)) {
        m_propertyToA[pointProp] = nullptr;
        m_aToProperty.remove(property);
    }*/
}

/*!
    \class QtColorPropertyManager

    \brief The QtColorPropertyManager provides and manages QColor properties.

    A color property has nested \e red, \e green and \e blue
    subproperties. The top-level property's value can be retrieved
    using the value() function, and set using the setValue() slot.

    The subproperties are created by a QtIntPropertyManager object. This
    manager can be retrieved using the subIntPropertyManager() function.  In
    order to provide editing widgets for the subproperties in a
    property browser widget, this manager must be associated with an
    editor factory.

    In addition, QtColorPropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes.

    \sa QtAbstractPropertyManager, QtAbstractPropertyBrowser, QtIntPropertyManager
*/

/*!
    \fn void QtColorPropertyManager::valueChanged(QtProperty *property, const QColor &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.

    \sa setValue()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtColorPropertyManager::QtColorPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtColorPropertyManagerPrivate;
    d_ptr->q_ptr = this;

//    d_ptr->m_intPropertyManager = new QtIntPropertyManager(this);
//    connect(d_ptr->m_intPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
//                this, SLOT(slotIntChanged(QtProperty *, int)));

//    connect(d_ptr->m_intPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
//                this, SLOT(slotPropertyDestroyed(QtProperty *)));

    d_ptr->m_stringPropertyManager = new QtStringPropertyManager(this);
    connect(d_ptr->m_stringPropertyManager, SIGNAL(valueChanged(QtProperty *, const QString &)),
                this, SLOT(slotStringChanged(QtProperty *, const QString &)));

    connect(d_ptr->m_stringPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtColorPropertyManager::~QtColorPropertyManager()
{
    clear();
    delete d_ptr;
}

/*!
    Returns the manager that produces the nested \e red, \e green and
    \e blue subproperties.

    In order to provide editing widgets for the subproperties in a
    property browser widget, this manager must be associated with an
    editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
//QtIntPropertyManager *QtColorPropertyManager::subIntPropertyManager() const
//{
//    return d_ptr->m_intPropertyManager;
//}

QtStringPropertyManager *QtColorPropertyManager::subStringPropertyManager() const
{
    return d_ptr->m_stringPropertyManager;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by \e this manager, this
    function returns an invalid color.

    \sa setValue()
*/
QColor QtColorPropertyManager::value(const QtProperty *property) const
{
    return d_ptr->m_values.value(property, QColor());
}

/*!
    \reimp
*/

QString QtColorPropertyManager::valueText(const QtProperty *property) const
{
    const QtColorPropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QString();

    return QtPropertyBrowserUtils::colorHexValueText(it.value());
}

/*!
    \reimp
*/

QIcon QtColorPropertyManager::valueIcon(const QtProperty *property) const
{
    const QtColorPropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QIcon();
    return QtPropertyBrowserUtils::brushValueIcon(QBrush(it.value()));
}

/*!
    \fn void QtColorPropertyManager::setValue(QtProperty *property, const QColor &value)

    Sets the value of the given \a property to \a value.  Nested
    properties are updated automatically.

    \sa value(), valueChanged()
*/
void QtColorPropertyManager::setValue(QtProperty *property, const QColor &val)
{
    const QtColorPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    if (it.value() == val)
        return;

    it.value() = val;

//    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToR[property], val.red());
//    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToG[property], val.green());
//    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToB[property], val.blue());
//    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToA[property], val.alpha());

    d_ptr->m_stringPropertyManager->blockSignals(true);
    d_ptr->m_stringPropertyManager->setValue(d_ptr->m_propertyToRgba[property], QtPropertyBrowserUtils::colorHexValueText(val));
    d_ptr->m_stringPropertyManager->blockSignals(false);

    emit propertyChanged(property);
    emit valueChanged(property, val);
}

/*!
    \reimp
*/
void QtColorPropertyManager::initializeProperty(QtProperty *property)
{
    QColor val;
    d_ptr->m_values[property] = val;

    QtProperty *rgbaProp = d_ptr->m_stringPropertyManager->addProperty();
    rgbaProp->setPropertyName("#");
    d_ptr->m_stringPropertyManager->setValue(rgbaProp, QtPropertyBrowserUtils::colorHexValueText(val));
    d_ptr->m_propertyToRgba[property] = rgbaProp;
    d_ptr->m_rgbaToProperty[rgbaProp] = property;
    property->addSubProperty(rgbaProp);

//    QtProperty *rProp = d_ptr->m_intPropertyManager->addProperty();
//    rProp->setPropertyName(tr("Red"));
//    d_ptr->m_intPropertyManager->setValue(rProp, val.red());
//    d_ptr->m_intPropertyManager->setRange(rProp, 0, 0xFF);
//    d_ptr->m_propertyToR[property] = rProp;
//    d_ptr->m_rToProperty[rProp] = property;
//    property->addSubProperty(rProp);

//    QtProperty *gProp = d_ptr->m_intPropertyManager->addProperty();
//    gProp->setPropertyName(tr("Green"));
//    d_ptr->m_intPropertyManager->setValue(gProp, val.green());
//    d_ptr->m_intPropertyManager->setRange(gProp, 0, 0xFF);
//    d_ptr->m_propertyToG[property] = gProp;
//    d_ptr->m_gToProperty[gProp] = property;
//    property->addSubProperty(gProp);

//    QtProperty *bProp = d_ptr->m_intPropertyManager->addProperty();
//    bProp->setPropertyName(tr("Blue"));
//    d_ptr->m_intPropertyManager->setValue(bProp, val.blue());
//    d_ptr->m_intPropertyManager->setRange(bProp, 0, 0xFF);
//    d_ptr->m_propertyToB[property] = bProp;
//    d_ptr->m_bToProperty[bProp] = property;
//    property->addSubProperty(bProp);

//    QtProperty *aProp = d_ptr->m_intPropertyManager->addProperty();
//    aProp->setPropertyName(tr("Alpha"));
//    d_ptr->m_intPropertyManager->setValue(aProp, val.alpha());
//    d_ptr->m_intPropertyManager->setRange(aProp, 0, 0xFF);
//    d_ptr->m_propertyToA[property] = aProp;
//    d_ptr->m_aToProperty[aProp] = property;
//    property->addSubProperty(aProp);
}

/*!
    \reimp
*/
void QtColorPropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *rgbaProp = d_ptr->m_propertyToRgba[property];
    if (rgbaProp) {
        d_ptr->m_rgbaToProperty.remove(rgbaProp);
        delete rgbaProp;
    }
    d_ptr->m_propertyToRgba.remove(property);

//    QtProperty *rProp = d_ptr->m_propertyToR[property];
//    if (rProp) {
//        d_ptr->m_rToProperty.remove(rProp);
//        delete rProp;
//    }
//    d_ptr->m_propertyToR.remove(property);

//    QtProperty *gProp = d_ptr->m_propertyToG[property];
//    if (gProp) {
//        d_ptr->m_gToProperty.remove(gProp);
//        delete gProp;
//    }
//    d_ptr->m_propertyToG.remove(property);

//    QtProperty *bProp = d_ptr->m_propertyToB[property];
//    if (bProp) {
//        d_ptr->m_bToProperty.remove(bProp);
//        delete bProp;
//    }
//    d_ptr->m_propertyToB.remove(property);

//    QtProperty *aProp = d_ptr->m_propertyToA[property];
//    if (aProp) {
//        d_ptr->m_aToProperty.remove(aProp);
//        delete aProp;
//    }
//    d_ptr->m_propertyToA.remove(property);

    d_ptr->m_values.remove(property);
}

void QtColorPropertyManager::slotStringChanged(QtProperty *p, const QString &val)
{
    d_ptr->slotStringChanged(p, val);
}

//void QtColorPropertyManager::slotIntChanged(QtProperty *p, int i)
//{
//    d_ptr->slotIntChanged(p, i);
//}

void QtColorPropertyManager::slotPropertyDestroyed(QtProperty *p)
{
    d_ptr->slotPropertyDestroyed(p);
}










QtColorEditWidget::QtColorEditWidget(QWidget *parent) :
    QWidget(parent),
    m_pixmapLabel(new QLabel),
    m_label(nullptr),
    m_lineEdit(new QLineEdit),
    m_button(new QToolButton)
{
    QHBoxLayout *lt = new QHBoxLayout(this);
    setupTreeViewEditorMargin(lt);
    lt->setSpacing(4);
    lt->setMargin(0);
    lt->addWidget(m_pixmapLabel);
    lt->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));
    lt->addWidget(m_lineEdit);
    m_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
    m_button->setFixedWidth(20);
    setFocusProxy(m_button);
    setFocusPolicy(m_button->focusPolicy());
    m_button->setText(tr("..."));
    m_button->installEventFilter(this);
    connect(m_button, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    connect(m_lineEdit, SIGNAL(editingFinished()), this, SLOT(editFinished()));
    lt->addWidget(m_button);
    m_lineEdit->setText(QtPropertyBrowserUtils::colorHexValueText(m_color));
    m_lineEdit->setMaxLength(9);
#ifdef _WIN32
    m_lineEdit->setFont(QFont("Courier New"));
#else
    m_lineEdit->setFont(QFont("Monospace"));
#endif
    QFontMetrics meter = QFontMetrics(m_lineEdit->font());
    m_lineEdit->setValidator(new QRegExpValidator(QRegExp("#[0-9a-fA-F]{6,8}"), m_lineEdit));
    m_pixmapLabel->setPixmap(QtPropertyBrowserUtils::brushValuePixmap(QBrush(m_color)));
    m_lineEdit->setMaximumWidth(meter.width("#ffffffff", 9) + 10);
}

void QtColorEditWidget::setValue(const QColor &c)
{
    if (m_color != c) {
        m_color = c;
        m_pixmapLabel->setPixmap(QtPropertyBrowserUtils::brushValuePixmap(QBrush(c), m_lineEdit->sizeHint()));
        m_lineEdit->setText(QtPropertyBrowserUtils::colorHexValueText(c));
    }
}

void QtColorEditWidget::buttonClicked()
{
    QColor &oldRgba = m_color;
    QColor newRgba = QColorDialog::getColor(oldRgba, this,
                                            QString(),
                                            QColorDialog::ShowAlphaChannel|
                                            QColorDialog::DontUseNativeDialog);
    if (newRgba.isValid() && newRgba != oldRgba) {
        setValue(newRgba);
        emit valueChanged(m_color);
    }
}

void QtColorEditWidget::editFinished()
{
    QColor &oldRgba = m_color;
    QColor newRgba = m_color;
    QString color = m_lineEdit->text();

    if(color.size() < 7)
    {
        m_lineEdit->setText(QtPropertyBrowserUtils::colorHexValueText(m_color));
        return;
    }

    color.remove(0, 1);

    if(color.size() >= 6)
    {
        newRgba.setRed(color.mid(0, 2).toInt(nullptr, 16));
        newRgba.setGreen(color.mid(2, 2).toInt(nullptr, 16));
        newRgba.setBlue(color.mid(4, 2).toInt(nullptr, 16));
    }
    if(color.size() == 8)
        newRgba.setAlpha(color.mid(6, 2).toInt(nullptr, 16));

    if (newRgba.isValid() && newRgba != oldRgba) {
        setValue(newRgba);
        emit valueChanged(m_color);
    }
}

bool QtColorEditWidget::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == m_button) {
        switch (ev->type()) {
        case QEvent::KeyPress:
        case QEvent::KeyRelease: { // Prevent the QToolButton from handling Enter/Escape meant control the delegate
            switch (static_cast<const QKeyEvent*>(ev)->key()) {
            case Qt::Key_Escape:
            case Qt::Key_Enter:
            case Qt::Key_Return:
                ev->ignore();
                return true;
            default:
                break;
            }
        }
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(obj, ev);
}

void QtColorEditWidget::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}





// QtColorEditorFactoryPrivate


void QtColorEditorFactoryPrivate::slotPropertyChanged(QtProperty *property,
                const QColor &value)
{
    const PropertyToEditorListMap::iterator it = m_createdEditors.find(property);
    if (it == m_createdEditors.end())
        return;
    QListIterator<QtColorEditWidget *> itEditor(it.value());

    while (itEditor.hasNext())
        itEditor.next()->setValue(value);
}

void QtColorEditorFactoryPrivate::slotSetValue(const QColor &value)
{
    QObject *object = q_ptr->sender();
    const EditorToPropertyMap::ConstIterator ecend = m_editorToProperty.constEnd();
    for (EditorToPropertyMap::ConstIterator itEditor = m_editorToProperty.constBegin(); itEditor != ecend; ++itEditor)
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtColorPropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
}

/*!
    \class QtColorEditorFactory

    \brief The QtColorEditorFactory class provides color editing  for
    properties created by QtColorPropertyManager objects.

    \sa QtAbstractEditorFactory, QtColorPropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtColorEditorFactory::QtColorEditorFactory(QObject *parent) :
    QtAbstractEditorFactory<QtColorPropertyManager>(parent),
    d_ptr(new QtColorEditorFactoryPrivate())
{
    d_ptr->q_ptr = this;
}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtColorEditorFactory::~QtColorEditorFactory()
{
    qDeleteAll(d_ptr->m_editorToProperty.keys());
    delete d_ptr;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtColorEditorFactory::connectPropertyManager(QtColorPropertyManager *manager)
{
    connect(manager, SIGNAL(valueChanged(QtProperty*,QColor)),
            this, SLOT(slotPropertyChanged(QtProperty*,QColor)));
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtColorEditorFactory::createEditor(QtColorPropertyManager *manager,
        QtProperty *property, QWidget *parent)
{
    QtColorEditWidget *editor = d_ptr->createEditor(property, parent);
    editor->setValue(manager->value(property));
    connect(editor, SIGNAL(valueChanged(QColor)), this, SLOT(slotSetValue(QColor)));
    connect(editor, SIGNAL(destroyed(QObject *)), this, SLOT(slotEditorDestroyed(QObject *)));
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtColorEditorFactory::disconnectPropertyManager(QtColorPropertyManager *manager)
{
    disconnect(manager, SIGNAL(valueChanged(QtProperty*,QColor)), this, SLOT(slotPropertyChanged(QtProperty*,QColor)));
}


#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif


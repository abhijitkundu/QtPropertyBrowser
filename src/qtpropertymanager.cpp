/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of a Qt Solutions component.
**
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
****************************************************************************/


#include "qtpropertymanager.h"
#include "qtpropertybrowserutils_p.h"
#include "managers/common.h"
#include <QtCore/QDateTime>
#include <QtCore/QLocale>
#include <QtCore/QMap>
#include <QtCore/QTimer>
#include <QtCore/QMetaEnum>
#include <QIcon>
#include <QFontDatabase>
#include <QStyleOption>
#include <QStyle>
#include <QPainter>
#include <QLabel>
#include <QCheckBox>
#include <QApplication>

#include <limits.h>
#include <float.h>

#if defined(Q_CC_MSVC)
#    pragma warning(disable: 4786) /* MS VS 6: truncating debug info after 255 characters */
#endif

#if QT_VERSION >= 0x040400
QT_BEGIN_NAMESPACE
#endif


class QtMetaEnumWrapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QSizePolicy::Policy policy READ policy)
public:
    QSizePolicy::Policy policy() const { return QSizePolicy::Ignored; }
private:
    QtMetaEnumWrapper(QObject *parent) : QObject(parent) {}
};

class QtMetaEnumProvider
{
public:
    QtMetaEnumProvider();

    QStringList policyEnumNames() const { return m_policyEnumNames; }
    QStringList languageEnumNames() const { return m_languageEnumNames; }
    QStringList countryEnumNames(QLocale::Language language) const { return m_countryEnumNames.value(language); }

    QSizePolicy::Policy indexToSizePolicy(int index) const;
    int sizePolicyToIndex(QSizePolicy::Policy policy) const;

    void indexToLocale(int languageIndex, int countryIndex, QLocale::Language *language, QLocale::Country *country) const;
    void localeToIndex(QLocale::Language language, QLocale::Country country, int *languageIndex, int *countryIndex) const;

private:
    void initLocale();

    QStringList m_policyEnumNames;
    QStringList m_languageEnumNames;
    QMap<QLocale::Language, QStringList> m_countryEnumNames;
    QMap<int, QLocale::Language> m_indexToLanguage;
    QMap<QLocale::Language, int> m_languageToIndex;
    QMap<int, QMap<int, QLocale::Country> > m_indexToCountry;
    QMap<QLocale::Language, QMap<QLocale::Country, int> > m_countryToIndex;
    QMetaEnum m_policyEnum;
};

#if QT_VERSION < 0x040300

static QList<QLocale::Country> countriesForLanguage(QLocale::Language language)
{
    QList<QLocale::Country> countries;
    QLocale::Country country = QLocale::AnyCountry;
    while (country <= QLocale::LastCountry) {
        QLocale locale(language, country);
        if (locale.language() == language && !countries.contains(locale.country()))
            countries << locale.country();
        country = (QLocale::Country)((uint)country + 1); // ++country
    }
    return countries;
}

#endif

static QList<QLocale::Country> sortCountries(const QList<QLocale::Country> &countries)
{
    QMultiMap<QString, QLocale::Country> nameToCountry;
    QListIterator<QLocale::Country> itCountry(countries);
    while (itCountry.hasNext()) {
        QLocale::Country country = itCountry.next();
        nameToCountry.insert(QLocale::countryToString(country), country);
    }
    return nameToCountry.values();
}

void QtMetaEnumProvider::initLocale()
{
    QMultiMap<QString, QLocale::Language> nameToLanguage;
    QLocale::Language language = QLocale::C;
    while (language <= QLocale::LastLanguage) {
        QLocale locale(language);
        if (locale.language() == language)
            nameToLanguage.insert(QLocale::languageToString(language), language);
        language = (QLocale::Language)((uint)language + 1); // ++language
    }

    const QLocale system = QLocale::system();
    if (!nameToLanguage.contains(QLocale::languageToString(system.language())))
        nameToLanguage.insert(QLocale::languageToString(system.language()), system.language());

    QList<QLocale::Language> languages = nameToLanguage.values();
    QListIterator<QLocale::Language> itLang(languages);
    while (itLang.hasNext()) {
        QLocale::Language language = itLang.next();
        QList<QLocale::Country> countries;
#if QT_VERSION < 0x040300
        countries = countriesForLanguage(language);
#else
        countries = QLocale::countriesForLanguage(language);
#endif
        if (countries.isEmpty() && language == system.language())
            countries << system.country();

        if (!countries.isEmpty() && !m_languageToIndex.contains(language)) {
            countries = sortCountries(countries);
            int langIdx = m_languageEnumNames.count();
            m_indexToLanguage[langIdx] = language;
            m_languageToIndex[language] = langIdx;
            QStringList countryNames;
            QListIterator<QLocale::Country> it(countries);
            int countryIdx = 0;
            while (it.hasNext()) {
                QLocale::Country country = it.next();
                countryNames << QLocale::countryToString(country);
                m_indexToCountry[langIdx][countryIdx] = country;
                m_countryToIndex[language][country] = countryIdx;
                ++countryIdx;
            }
            m_languageEnumNames << QLocale::languageToString(language);
            m_countryEnumNames[language] = countryNames;
        }
    }
}

QtMetaEnumProvider::QtMetaEnumProvider()
{
    QMetaProperty p;

    p = QtMetaEnumWrapper::staticMetaObject.property(
                QtMetaEnumWrapper::staticMetaObject.propertyOffset() + 0);
    m_policyEnum = p.enumerator();
    const int keyCount = m_policyEnum.keyCount();
    for (int i = 0; i < keyCount; i++)
        m_policyEnumNames << QLatin1String(m_policyEnum.key(i));

    initLocale();
}

QSizePolicy::Policy QtMetaEnumProvider::indexToSizePolicy(int index) const
{
    return static_cast<QSizePolicy::Policy>(m_policyEnum.value(index));
}

int QtMetaEnumProvider::sizePolicyToIndex(QSizePolicy::Policy policy) const
{
     const int keyCount = m_policyEnum.keyCount();
    for (int i = 0; i < keyCount; i++)
        if (indexToSizePolicy(i) == policy)
            return i;
    return -1;
}

void QtMetaEnumProvider::indexToLocale(int languageIndex, int countryIndex, QLocale::Language *language, QLocale::Country *country) const
{
    QLocale::Language l = QLocale::C;
    QLocale::Country c = QLocale::AnyCountry;
    if (m_indexToLanguage.contains(languageIndex)) {
        l = m_indexToLanguage[languageIndex];
        if (m_indexToCountry.contains(languageIndex) && m_indexToCountry[languageIndex].contains(countryIndex))
            c = m_indexToCountry[languageIndex][countryIndex];
    }
    if (language)
        *language = l;
    if (country)
        *country = c;
}

void QtMetaEnumProvider::localeToIndex(QLocale::Language language, QLocale::Country country, int *languageIndex, int *countryIndex) const
{
    int l = -1;
    int c = -1;
    if (m_languageToIndex.contains(language)) {
        l = m_languageToIndex[language];
        if (m_countryToIndex.contains(language) && m_countryToIndex[language].contains(country))
            c = m_countryToIndex[language][country];
    }

    if (languageIndex)
        *languageIndex = l;
    if (countryIndex)
        *countryIndex = c;
}

Q_GLOBAL_STATIC(QtMetaEnumProvider, metaEnumProvider)










// QtKeySequencePropertyManager

class QtKeySequencePropertyManagerPrivate
{
    QtKeySequencePropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtKeySequencePropertyManager)
public:

    QString m_format;

    typedef QMap<const QtProperty *, QKeySequence> PropertyValueMap;
    PropertyValueMap m_values;
};

/*! \class QtKeySequencePropertyManager

    \brief The QtKeySequencePropertyManager provides and manages QKeySequence properties.

    A key sequence's value can be retrieved using the value()
    function, and set using the setValue() slot.

    In addition, QtKeySequencePropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes.

    \sa QtAbstractPropertyManager
*/

/*!
    \fn void QtKeySequencePropertyManager::valueChanged(QtProperty *property, const QKeySequence &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.
*/

/*!
    Creates a manager with the given \a parent.
*/
QtKeySequencePropertyManager::QtKeySequencePropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtKeySequencePropertyManagerPrivate;
    d_ptr->q_ptr = this;
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtKeySequencePropertyManager::~QtKeySequencePropertyManager()
{
    clear();
    delete d_ptr;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by this manager, this
    function returns an empty QKeySequence object.

    \sa setValue()
*/
QKeySequence QtKeySequencePropertyManager::value(const QtProperty *property) const
{
    return d_ptr->m_values.value(property, QKeySequence());
}

/*!
    \reimp
*/
QString QtKeySequencePropertyManager::valueText(const QtProperty *property) const
{
    const QtKeySequencePropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QString();
    return it.value().toString(QKeySequence::NativeText);
}

/*!
    \fn void QtKeySequencePropertyManager::setValue(QtProperty *property, const QKeySequence &value)

    Sets the value of the given \a property to \a value.

    \sa value(), valueChanged()
*/
void QtKeySequencePropertyManager::setValue(QtProperty *property, const QKeySequence &val)
{
    setSimpleValue<const QKeySequence &, QKeySequence, QtKeySequencePropertyManager>(d_ptr->m_values, this,
                &QtKeySequencePropertyManager::propertyChanged,
                &QtKeySequencePropertyManager::valueChanged,
                property, val);
}

/*!
    \reimp
*/
void QtKeySequencePropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QKeySequence();
}

/*!
    \reimp
*/
void QtKeySequencePropertyManager::uninitializeProperty(QtProperty *property)
{
    d_ptr->m_values.remove(property);
}

// QtCharPropertyManager

class QtCharPropertyManagerPrivate
{
    QtCharPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtCharPropertyManager)
public:

    typedef QMap<const QtProperty *, QChar> PropertyValueMap;
    PropertyValueMap m_values;
};

/*! \class QtCharPropertyManager

    \brief The QtCharPropertyManager provides and manages QChar properties.

    A char's value can be retrieved using the value()
    function, and set using the setValue() slot.

    In addition, QtCharPropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes.

    \sa QtAbstractPropertyManager
*/

/*!
    \fn void QtCharPropertyManager::valueChanged(QtProperty *property, const QChar &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.
*/

/*!
    Creates a manager with the given \a parent.
*/
QtCharPropertyManager::QtCharPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtCharPropertyManagerPrivate;
    d_ptr->q_ptr = this;
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtCharPropertyManager::~QtCharPropertyManager()
{
    clear();
    delete d_ptr;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by this manager, this
    function returns an null QChar object.

    \sa setValue()
*/
QChar QtCharPropertyManager::value(const QtProperty *property) const
{
    return d_ptr->m_values.value(property, QChar());
}

/*!
    \reimp
*/
QString QtCharPropertyManager::valueText(const QtProperty *property) const
{
   const QtCharPropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QString();
    const QChar c = it.value();
    return c.isNull() ? QString() : QString(c);
}

/*!
    \fn void QtCharPropertyManager::setValue(QtProperty *property, const QChar &value)

    Sets the value of the given \a property to \a value.

    \sa value(), valueChanged()
*/
void QtCharPropertyManager::setValue(QtProperty *property, const QChar &val)
{
    setSimpleValue<const QChar &, QChar, QtCharPropertyManager>(d_ptr->m_values, this,
                &QtCharPropertyManager::propertyChanged,
                &QtCharPropertyManager::valueChanged,
                property, val);
}

/*!
    \reimp
*/
void QtCharPropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QChar();
}

/*!
    \reimp
*/
void QtCharPropertyManager::uninitializeProperty(QtProperty *property)
{
    d_ptr->m_values.remove(property);
}

// QtLocalePropertyManager

class QtLocalePropertyManagerPrivate
{
    QtLocalePropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtLocalePropertyManager)
public:

    QtLocalePropertyManagerPrivate();

    void slotEnumChanged(QtProperty *property, int value);
    void slotPropertyDestroyed(QtProperty *property);

    typedef QMap<const QtProperty *, QLocale> PropertyValueMap;
    PropertyValueMap m_values;

    QtEnumPropertyManager *m_enumPropertyManager;

    QMap<const QtProperty *, QtProperty *> m_propertyToLanguage;
    QMap<const QtProperty *, QtProperty *> m_propertyToCountry;

    QMap<const QtProperty *, QtProperty *> m_languageToProperty;
    QMap<const QtProperty *, QtProperty *> m_countryToProperty;
};

QtLocalePropertyManagerPrivate::QtLocalePropertyManagerPrivate()
{
}

void QtLocalePropertyManagerPrivate::slotEnumChanged(QtProperty *property, int value)
{
    if (QtProperty *prop = m_languageToProperty.value(property, nullptr)) {
        const QLocale loc = m_values[prop];
        QLocale::Language newLanguage = loc.language();
        QLocale::Country newCountry = loc.country();
        metaEnumProvider()->indexToLocale(value, 0, &newLanguage, nullptr);
        QLocale newLoc(newLanguage, newCountry);
        q_ptr->setValue(prop, newLoc);
    } else if (QtProperty *prop = m_countryToProperty.value(property, nullptr)) {
        const QLocale loc = m_values[prop];
        QLocale::Language newLanguage = loc.language();
        QLocale::Country newCountry = loc.country();
        metaEnumProvider()->indexToLocale(m_enumPropertyManager->value(m_propertyToLanguage.value(prop)), value, &newLanguage, &newCountry);
        QLocale newLoc(newLanguage, newCountry);
        q_ptr->setValue(prop, newLoc);
    }
}

void QtLocalePropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (QtProperty *subProp = m_languageToProperty.value(property, nullptr)) {
        m_propertyToLanguage[subProp] = nullptr;
        m_languageToProperty.remove(property);
    } else if (QtProperty *subProp = m_countryToProperty.value(property, nullptr)) {
        m_propertyToCountry[subProp] = nullptr;
        m_countryToProperty.remove(property);
    }
}

/*!
    \class QtLocalePropertyManager

    \brief The QtLocalePropertyManager provides and manages QLocale properties.

    A locale property has nested \e language and \e country
    subproperties. The top-level property's value can be retrieved
    using the value() function, and set using the setValue() slot.

    The subproperties are created by QtEnumPropertyManager object.
    These submanager can be retrieved using the subEnumPropertyManager()
    function. In order to provide editing widgets for the subproperties
    in a property browser widget, this manager must be associated with editor factory.

    In addition, QtLocalePropertyManager provides the valueChanged()
    signal which is emitted whenever a property created by this
    manager changes.

    \sa QtAbstractPropertyManager, QtEnumPropertyManager
*/

/*!
    \fn void QtLocalePropertyManager::valueChanged(QtProperty *property, const QLocale &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the
    new \a value as parameters.

    \sa setValue()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtLocalePropertyManager::QtLocalePropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtLocalePropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_enumPropertyManager = new QtEnumPropertyManager(this);
    connect(d_ptr->m_enumPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotEnumChanged(QtProperty *, int)));

    connect(d_ptr->m_enumPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtLocalePropertyManager::~QtLocalePropertyManager()
{
    clear();
    delete d_ptr;
}

/*!
    Returns the manager that creates the nested \e language
    and \e country subproperties.

    In order to provide editing widgets for the mentioned subproperties
    in a property browser widget, this manager must be associated with
    an editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtEnumPropertyManager *QtLocalePropertyManager::subEnumPropertyManager() const
{
    return d_ptr->m_enumPropertyManager;
}

/*!
    Returns the given \a property's value.

    If the given property is not managed by this manager, this
    function returns the default locale.

    \sa setValue()
*/
QLocale QtLocalePropertyManager::value(const QtProperty *property) const
{
    return d_ptr->m_values.value(property, QLocale());
}

/*!
    \reimp
*/
QString QtLocalePropertyManager::valueText(const QtProperty *property) const
{
    const QtLocalePropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QString();

    QLocale loc = it.value();

    int langIdx = 0;
    int countryIdx = 0;
    metaEnumProvider()->localeToIndex(loc.language(), loc.country(), &langIdx, &countryIdx);
    QString str = tr("%1, %2")
            .arg(metaEnumProvider()->languageEnumNames().at(langIdx))
            .arg(metaEnumProvider()->countryEnumNames(loc.language()).at(countryIdx));
    return str;
}

/*!
    \fn void QtLocalePropertyManager::setValue(QtProperty *property, const QLocale &value)

    Sets the value of the given \a property to \a value. Nested
    properties are updated automatically.

    \sa value(), valueChanged()
*/
void QtLocalePropertyManager::setValue(QtProperty *property, const QLocale &val)
{
    const QtLocalePropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    const QLocale loc = it.value();
    if (loc == val)
        return;

    it.value() = val;

    int langIdx = 0;
    int countryIdx = 0;
    metaEnumProvider()->localeToIndex(val.language(), val.country(), &langIdx, &countryIdx);
    if (loc.language() != val.language()) {
        d_ptr->m_enumPropertyManager->setValue(d_ptr->m_propertyToLanguage.value(property), langIdx);
        d_ptr->m_enumPropertyManager->setEnumNames(d_ptr->m_propertyToCountry.value(property),
                    metaEnumProvider()->countryEnumNames(val.language()));
    }
    d_ptr->m_enumPropertyManager->setValue(d_ptr->m_propertyToCountry.value(property), countryIdx);

    emit propertyChanged(property);
    emit valueChanged(property, val);
}

/*!
    \reimp
*/
void QtLocalePropertyManager::initializeProperty(QtProperty *property)
{
    QLocale val;
    d_ptr->m_values[property] = val;

    int langIdx = 0;
    int countryIdx = 0;
    metaEnumProvider()->localeToIndex(val.language(), val.country(), &langIdx, &countryIdx);

    QtProperty *languageProp = d_ptr->m_enumPropertyManager->addProperty();
    languageProp->setPropertyName(tr("Language"));
    d_ptr->m_enumPropertyManager->setEnumNames(languageProp, metaEnumProvider()->languageEnumNames());
    d_ptr->m_enumPropertyManager->setValue(languageProp, langIdx);
    d_ptr->m_propertyToLanguage[property] = languageProp;
    d_ptr->m_languageToProperty[languageProp] = property;
    property->addSubProperty(languageProp);

    QtProperty *countryProp = d_ptr->m_enumPropertyManager->addProperty();
    countryProp->setPropertyName(tr("Country"));
    d_ptr->m_enumPropertyManager->setEnumNames(countryProp, metaEnumProvider()->countryEnumNames(val.language()));
    d_ptr->m_enumPropertyManager->setValue(countryProp, countryIdx);
    d_ptr->m_propertyToCountry[property] = countryProp;
    d_ptr->m_countryToProperty[countryProp] = property;
    property->addSubProperty(countryProp);
}

/*!
    \reimp
*/
void QtLocalePropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *languageProp = d_ptr->m_propertyToLanguage[property];
    if (languageProp) {
        d_ptr->m_languageToProperty.remove(languageProp);
        delete languageProp;
    }
    d_ptr->m_propertyToLanguage.remove(property);

    QtProperty *countryProp = d_ptr->m_propertyToCountry[property];
    if (countryProp) {
        d_ptr->m_countryToProperty.remove(countryProp);
        delete countryProp;
    }
    d_ptr->m_propertyToCountry.remove(property);

    d_ptr->m_values.remove(property);
}

// QtPointPropertyManager

class QtPointPropertyManagerPrivate
{
    QtPointPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtPointPropertyManager)
public:

    void slotIntChanged(QtProperty *property, int value);
    void slotPropertyDestroyed(QtProperty *property);

    typedef QMap<const QtProperty *, QPoint> PropertyValueMap;
    PropertyValueMap m_values;

    QtIntPropertyManager *m_intPropertyManager;

    QMap<const QtProperty *, QtProperty *> m_propertyToX;
    QMap<const QtProperty *, QtProperty *> m_propertyToY;

    QMap<const QtProperty *, QtProperty *> m_xToProperty;
    QMap<const QtProperty *, QtProperty *> m_yToProperty;
};

void QtPointPropertyManagerPrivate::slotIntChanged(QtProperty *property, int value)
{
    if (QtProperty *xprop = m_xToProperty.value(property, nullptr)) {
        QPoint p = m_values[xprop];
        p.setX(value);
        q_ptr->setValue(xprop, p);
    } else if (QtProperty *yprop = m_yToProperty.value(property, nullptr)) {
        QPoint p = m_values[yprop];
        p.setY(value);
        q_ptr->setValue(yprop, p);
    }
}

void QtPointPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (QtProperty *pointProp = m_xToProperty.value(property, nullptr)) {
        m_propertyToX[pointProp] = nullptr;
        m_xToProperty.remove(property);
    } else if (QtProperty *pointProp = m_yToProperty.value(property, nullptr)) {
        m_propertyToY[pointProp] = nullptr;
        m_yToProperty.remove(property);
    }
}

/*! \class QtPointPropertyManager

    \brief The QtPointPropertyManager provides and manages QPoint properties.

    A point property has nested \e x and \e y subproperties. The
    top-level property's value can be retrieved using the value()
    function, and set using the setValue() slot.

    The subproperties are created by a QtIntPropertyManager object. This
    manager can be retrieved using the subIntPropertyManager() function. In
    order to provide editing widgets for the subproperties in a
    property browser widget, this manager must be associated with an
    editor factory.

    In addition, QtPointPropertyManager provides the valueChanged() signal which
    is emitted whenever a property created by this manager changes.

    \sa QtAbstractPropertyManager, QtIntPropertyManager, QtPointFPropertyManager
*/

/*!
    \fn void QtPointPropertyManager::valueChanged(QtProperty *property, const QPoint &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the
    new \a value as parameters.

    \sa setValue()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtPointPropertyManager::QtPointPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtPointPropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_intPropertyManager = new QtIntPropertyManager(this);
    connect(d_ptr->m_intPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotIntChanged(QtProperty *, int)));
    connect(d_ptr->m_intPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtPointPropertyManager::~QtPointPropertyManager()
{
    clear();
    delete d_ptr;
}

/*!
    Returns the manager that creates the nested \e x and \e y
    subproperties.

    In order to provide editing widgets for the subproperties in a
    property browser widget, this manager must be associated with an
    editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtIntPropertyManager *QtPointPropertyManager::subIntPropertyManager() const
{
    return d_ptr->m_intPropertyManager;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by this manager, this
    function returns a point with coordinates (0, 0).

    \sa setValue()
*/
QPoint QtPointPropertyManager::value(const QtProperty *property) const
{
    return d_ptr->m_values.value(property, QPoint());
}

/*!
    \reimp
*/
QString QtPointPropertyManager::valueText(const QtProperty *property) const
{
    const QtPointPropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QString();
    const QPoint v = it.value();
    return QString(tr("(%1, %2)").arg(QString::number(v.x()))
                                 .arg(QString::number(v.y())));
}

/*!
    \fn void QtPointPropertyManager::setValue(QtProperty *property, const QPoint &value)

    Sets the value of the given \a property to \a value. Nested
    properties are updated automatically.

    \sa value(), valueChanged()
*/
void QtPointPropertyManager::setValue(QtProperty *property, const QPoint &val)
{
    const QtPointPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    if (it.value() == val)
        return;

    it.value() = val;
    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToX[property], val.x());
    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToY[property], val.y());

    emit propertyChanged(property);
    emit valueChanged(property, val);
}

/*!
    \reimp
*/
void QtPointPropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QPoint(0, 0);

    QtProperty *xProp = d_ptr->m_intPropertyManager->addProperty();
    xProp->setPropertyName(tr("X"));
    d_ptr->m_intPropertyManager->setValue(xProp, 0);
    d_ptr->m_propertyToX[property] = xProp;
    d_ptr->m_xToProperty[xProp] = property;
    property->addSubProperty(xProp);

    QtProperty *yProp = d_ptr->m_intPropertyManager->addProperty();
    yProp->setPropertyName(tr("Y"));
    d_ptr->m_intPropertyManager->setValue(yProp, 0);
    d_ptr->m_propertyToY[property] = yProp;
    d_ptr->m_yToProperty[yProp] = property;
    property->addSubProperty(yProp);
}

/*!
    \reimp
*/
void QtPointPropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *xProp = d_ptr->m_propertyToX[property];
    if (xProp) {
        d_ptr->m_xToProperty.remove(xProp);
        delete xProp;
    }
    d_ptr->m_propertyToX.remove(property);

    QtProperty *yProp = d_ptr->m_propertyToY[property];
    if (yProp) {
        d_ptr->m_yToProperty.remove(yProp);
        delete yProp;
    }
    d_ptr->m_propertyToY.remove(property);

    d_ptr->m_values.remove(property);
}

// QtPointFPropertyManager

class QtPointFPropertyManagerPrivate
{
    QtPointFPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtPointFPropertyManager)
public:

    struct Data
    {
        Data() : decimals(2) {}
        QPointF val;
        int decimals;
    };

    void slotDoubleChanged(QtProperty *property, double value);
    void slotPropertyDestroyed(QtProperty *property);

    typedef QMap<const QtProperty *, Data> PropertyValueMap;
    PropertyValueMap m_values;

    QtDoublePropertyManager *m_doublePropertyManager;

    QMap<const QtProperty *, QtProperty *> m_propertyToX;
    QMap<const QtProperty *, QtProperty *> m_propertyToY;

    QMap<const QtProperty *, QtProperty *> m_xToProperty;
    QMap<const QtProperty *, QtProperty *> m_yToProperty;
};

void QtPointFPropertyManagerPrivate::slotDoubleChanged(QtProperty *property, double value)
{
    if (QtProperty *prop = m_xToProperty.value(property, nullptr)) {
        QPointF p = m_values[prop].val;
        p.setX(value);
        q_ptr->setValue(prop, p);
    } else if (QtProperty *prop = m_yToProperty.value(property, nullptr)) {
        QPointF p = m_values[prop].val;
        p.setY(value);
        q_ptr->setValue(prop, p);
    }
}

void QtPointFPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (QtProperty *pointProp  = m_xToProperty.value(property, nullptr)) {
        m_propertyToX[pointProp] = nullptr;
        m_xToProperty.remove(property);
    } else if (QtProperty *pointProp = m_yToProperty.value(property, nullptr)) {
        m_propertyToY[pointProp] = nullptr;
        m_yToProperty.remove(property);
    }
}

/*! \class QtPointFPropertyManager

    \brief The QtPointFPropertyManager provides and manages QPointF properties.

    A point property has nested \e x and \e y subproperties. The
    top-level property's value can be retrieved using the value()
    function, and set using the setValue() slot.

    The subproperties are created by a QtDoublePropertyManager object. This
    manager can be retrieved using the subDoublePropertyManager() function. In
    order to provide editing widgets for the subproperties in a
    property browser widget, this manager must be associated with an
    editor factory.

    In addition, QtPointFPropertyManager provides the valueChanged() signal which
    is emitted whenever a property created by this manager changes.

    \sa QtAbstractPropertyManager, QtDoublePropertyManager, QtPointPropertyManager
*/

/*!
    \fn void QtPointFPropertyManager::valueChanged(QtProperty *property, const QPointF &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the
    new \a value as parameters.

    \sa setValue()
*/

/*!
    \fn void QtPointFPropertyManager::decimalsChanged(QtProperty *property, int prec)

    This signal is emitted whenever a property created by this manager
    changes its precision of value, passing a pointer to the
    \a property and the new \a prec value

    \sa setDecimals()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtPointFPropertyManager::QtPointFPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtPointFPropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_doublePropertyManager = new QtDoublePropertyManager(this);
    connect(d_ptr->m_doublePropertyManager, SIGNAL(valueChanged(QtProperty *, double)),
                this, SLOT(slotDoubleChanged(QtProperty *, double)));
    connect(d_ptr->m_doublePropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtPointFPropertyManager::~QtPointFPropertyManager()
{
    clear();
    delete d_ptr;
}

/*!
    Returns the manager that creates the nested \e x and \e y
    subproperties.

    In order to provide editing widgets for the subproperties in a
    property browser widget, this manager must be associated with an
    editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtDoublePropertyManager *QtPointFPropertyManager::subDoublePropertyManager() const
{
    return d_ptr->m_doublePropertyManager;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by this manager, this
    function returns a point with coordinates (0, 0).

    \sa setValue()
*/
QPointF QtPointFPropertyManager::value(const QtProperty *property) const
{
    return getValue<QPointF>(d_ptr->m_values, property);
}

/*!
    Returns the given \a property's precision, in decimals.

    \sa setDecimals()
*/
int QtPointFPropertyManager::decimals(const QtProperty *property) const
{
    return getData<int>(d_ptr->m_values, &QtPointFPropertyManagerPrivate::Data::decimals, property, 0);
}

/*!
    \reimp
*/
QString QtPointFPropertyManager::valueText(const QtProperty *property) const
{
    const QtPointFPropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QString();
    const QPointF v = it.value().val;
    const int dec =  it.value().decimals;
    return QString(tr("(%1, %2)").arg(QString::number(v.x(), 'f', dec))
                                 .arg(QString::number(v.y(), 'f', dec)));
}

/*!
    \fn void QtPointFPropertyManager::setValue(QtProperty *property, const QPointF &value)

    Sets the value of the given \a property to \a value. Nested
    properties are updated automatically.

    \sa value(), valueChanged()
*/
void QtPointFPropertyManager::setValue(QtProperty *property, const QPointF &val)
{
    const QtPointFPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    if (it.value().val == val)
        return;

    it.value().val = val;
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToX[property], val.x());
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToY[property], val.y());

    emit propertyChanged(property);
    emit valueChanged(property, val);
}

/*!
    \fn void QtPointFPropertyManager::setDecimals(QtProperty *property, int prec)

    Sets the precision of the given \a property to \a prec.

    The valid decimal range is 0-13. The default is 2.

    \sa decimals()
*/
void QtPointFPropertyManager::setDecimals(QtProperty *property, int prec)
{
    const QtPointFPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    QtPointFPropertyManagerPrivate::Data data = it.value();

    if (prec > 13)
        prec = 13;
    else if (prec < 0)
        prec = 0;

    if (data.decimals == prec)
        return;

    data.decimals = prec;
    d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToX[property], prec);
    d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToY[property], prec);

    it.value() = data;

    emit decimalsChanged(property, data.decimals);
}

/*!
    \reimp
*/
void QtPointFPropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtPointFPropertyManagerPrivate::Data();

    QtProperty *xProp = d_ptr->m_doublePropertyManager->addProperty();
    xProp->setPropertyName(tr("X"));
    d_ptr->m_doublePropertyManager->setDecimals(xProp, decimals(property));
    d_ptr->m_doublePropertyManager->setValue(xProp, 0);
    d_ptr->m_propertyToX[property] = xProp;
    d_ptr->m_xToProperty[xProp] = property;
    property->addSubProperty(xProp);

    QtProperty *yProp = d_ptr->m_doublePropertyManager->addProperty();
    yProp->setPropertyName(tr("Y"));
    d_ptr->m_doublePropertyManager->setDecimals(yProp, decimals(property));
    d_ptr->m_doublePropertyManager->setValue(yProp, 0);
    d_ptr->m_propertyToY[property] = yProp;
    d_ptr->m_yToProperty[yProp] = property;
    property->addSubProperty(yProp);
}

/*!
    \reimp
*/
void QtPointFPropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *xProp = d_ptr->m_propertyToX[property];
    if (xProp) {
        d_ptr->m_xToProperty.remove(xProp);
        delete xProp;
    }
    d_ptr->m_propertyToX.remove(property);

    QtProperty *yProp = d_ptr->m_propertyToY[property];
    if (yProp) {
        d_ptr->m_yToProperty.remove(yProp);
        delete yProp;
    }
    d_ptr->m_propertyToY.remove(property);

    d_ptr->m_values.remove(property);
}

// QtSizePropertyManager

class QtSizePropertyManagerPrivate
{
    QtSizePropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtSizePropertyManager)
public:

    void slotIntChanged(QtProperty *property, int value);
    void slotPropertyDestroyed(QtProperty *property);
    void setValue(QtProperty *property, const QSize &val);
    void setRange(QtProperty *property,
                const QSize &minVal, const QSize &maxVal, const QSize &val);

    struct Data
    {
        Data() : val(QSize(0, 0)), minVal(QSize(0, 0)), maxVal(QSize(INT_MAX, INT_MAX)) {}
        QSize val;
        QSize minVal;
        QSize maxVal;
        QSize minimumValue() const { return minVal; }
        QSize maximumValue() const { return maxVal; }
        void setMinimumValue(const QSize &newMinVal) { setSizeMinimumData(this, newMinVal); }
        void setMaximumValue(const QSize &newMaxVal) { setSizeMaximumData(this, newMaxVal); }
    };

    typedef QMap<const QtProperty *, Data> PropertyValueMap;
    PropertyValueMap m_values;

    QtIntPropertyManager *m_intPropertyManager;

    QMap<const QtProperty *, QtProperty *> m_propertyToW;
    QMap<const QtProperty *, QtProperty *> m_propertyToH;

    QMap<const QtProperty *, QtProperty *> m_wToProperty;
    QMap<const QtProperty *, QtProperty *> m_hToProperty;
};

void QtSizePropertyManagerPrivate::slotIntChanged(QtProperty *property, int value)
{
    if (QtProperty *prop = m_wToProperty.value(property, nullptr)) {
        QSize s = m_values[prop].val;
        s.setWidth(value);
        q_ptr->setValue(prop, s);
    } else if (QtProperty *prop = m_hToProperty.value(property, nullptr)) {
        QSize s = m_values[prop].val;
        s.setHeight(value);
        q_ptr->setValue(prop, s);
    }
}

void QtSizePropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (QtProperty *pointProp = m_wToProperty.value(property, nullptr)) {
        m_propertyToW[pointProp] = nullptr;
        m_wToProperty.remove(property);
    } else if (QtProperty *pointProp = m_hToProperty.value(property, nullptr)) {
        m_propertyToH[pointProp] = nullptr;
        m_hToProperty.remove(property);
    }
}

void QtSizePropertyManagerPrivate::setValue(QtProperty *property, const QSize &val)
{
    m_intPropertyManager->setValue(m_propertyToW.value(property), val.width());
    m_intPropertyManager->setValue(m_propertyToH.value(property), val.height());
}

void QtSizePropertyManagerPrivate::setRange(QtProperty *property,
                const QSize &minVal, const QSize &maxVal, const QSize &val)
{
    QtProperty *wProperty = m_propertyToW.value(property);
    QtProperty *hProperty = m_propertyToH.value(property);
    m_intPropertyManager->setRange(wProperty, minVal.width(), maxVal.width());
    m_intPropertyManager->setValue(wProperty, val.width());
    m_intPropertyManager->setRange(hProperty, minVal.height(), maxVal.height());
    m_intPropertyManager->setValue(hProperty, val.height());
}

/*!
    \class QtSizePropertyManager

    \brief The QtSizePropertyManager provides and manages QSize properties.

    A size property has nested \e width and \e height
    subproperties. The top-level property's value can be retrieved
    using the value() function, and set using the setValue() slot.

    The subproperties are created by a QtIntPropertyManager object. This
    manager can be retrieved using the subIntPropertyManager() function. In
    order to provide editing widgets for the subproperties in a
    property browser widget, this manager must be associated with an
    editor factory.

    A size property also has a range of valid values defined by a
    minimum size and a maximum size. These sizes can be retrieved
    using the minimum() and the maximum() functions, and set using the
    setMinimum() and setMaximum() slots. Alternatively, the range can
    be defined in one go using the setRange() slot.

    In addition, QtSizePropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes, and the rangeChanged() signal which is emitted whenever
    such a property changes its range of valid sizes.

    \sa QtAbstractPropertyManager, QtIntPropertyManager, QtSizeFPropertyManager
*/

/*!
    \fn void QtSizePropertyManager::valueChanged(QtProperty *property, const QSize &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.

    \sa setValue()
*/

/*!
    \fn void QtSizePropertyManager::rangeChanged(QtProperty *property, const QSize &minimum, const QSize &maximum)

    This signal is emitted whenever a property created by this manager
    changes its range of valid sizes, passing a pointer to the \a
    property and the new \a minimum and \a maximum sizes.

    \sa setRange()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtSizePropertyManager::QtSizePropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtSizePropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_intPropertyManager = new QtIntPropertyManager(this);
    connect(d_ptr->m_intPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotIntChanged(QtProperty *, int)));
    connect(d_ptr->m_intPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtSizePropertyManager::~QtSizePropertyManager()
{
    clear();
    delete d_ptr;
}

/*!
    Returns the manager that creates the nested \e width and \e height
    subproperties.

    In order to provide editing widgets for the \e width and \e height
    properties in a property browser widget, this manager must be
    associated with an editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtIntPropertyManager *QtSizePropertyManager::subIntPropertyManager() const
{
    return d_ptr->m_intPropertyManager;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by this manager, this
    function returns an invalid size

    \sa setValue()
*/
QSize QtSizePropertyManager::value(const QtProperty *property) const
{
    return getValue<QSize>(d_ptr->m_values, property);
}

/*!
    Returns the given \a property's minimum size value.

    \sa setMinimum(), maximum(), setRange()
*/
QSize QtSizePropertyManager::minimum(const QtProperty *property) const
{
    return getMinimum<QSize>(d_ptr->m_values, property);
}

/*!
    Returns the given \a property's maximum size value.

    \sa setMaximum(), minimum(), setRange()
*/
QSize QtSizePropertyManager::maximum(const QtProperty *property) const
{
    return getMaximum<QSize>(d_ptr->m_values, property);
}

/*!
    \reimp
*/
QString QtSizePropertyManager::valueText(const QtProperty *property) const
{
    const QtSizePropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QString();
    const QSize v = it.value().val;
    return QString(tr("%1 x %2").arg(QString::number(v.width()))
                                .arg(QString::number(v.height())));
}

/*!
    \fn void QtSizePropertyManager::setValue(QtProperty *property, const QSize &value)

    Sets the value of the given \a property to \a value.

    If the specified \a value is not valid according to the given \a
    property's size range, the \a value is adjusted to the nearest
    valid value within the size range.

    \sa value(), setRange(), valueChanged()
*/
void QtSizePropertyManager::setValue(QtProperty *property, const QSize &val)
{
    setValueInRange<const QSize &, QtSizePropertyManagerPrivate, QtSizePropertyManager, const QSize>(this, d_ptr,
                &QtSizePropertyManager::propertyChanged,
                &QtSizePropertyManager::valueChanged,
                property, val, &QtSizePropertyManagerPrivate::setValue);
}

/*!
    Sets the minimum size value for the given \a property to \a minVal.

    When setting the minimum size value, the maximum and current
    values are adjusted if necessary (ensuring that the size range
    remains valid and that the current value is within the range).

    \sa minimum(), setRange(), rangeChanged()
*/
void QtSizePropertyManager::setMinimum(QtProperty *property, const QSize &minVal)
{
    setBorderValue<const QSize &, QtSizePropertyManagerPrivate, QtSizePropertyManager, QSize, QtSizePropertyManagerPrivate::Data>(this, d_ptr,
                &QtSizePropertyManager::propertyChanged,
                &QtSizePropertyManager::valueChanged,
                &QtSizePropertyManager::rangeChanged,
                property,
                &QtSizePropertyManagerPrivate::Data::minimumValue,
                &QtSizePropertyManagerPrivate::Data::setMinimumValue,
                minVal, &QtSizePropertyManagerPrivate::setRange);
}

/*!
    Sets the maximum size value for the given \a property to \a maxVal.

    When setting the maximum size value, the minimum and current
    values are adjusted if necessary (ensuring that the size range
    remains valid and that the current value is within the range).

    \sa maximum(), setRange(), rangeChanged()
*/
void QtSizePropertyManager::setMaximum(QtProperty *property, const QSize &maxVal)
{
    setBorderValue<const QSize &, QtSizePropertyManagerPrivate, QtSizePropertyManager, QSize, QtSizePropertyManagerPrivate::Data>(this, d_ptr,
                &QtSizePropertyManager::propertyChanged,
                &QtSizePropertyManager::valueChanged,
                &QtSizePropertyManager::rangeChanged,
                property,
                &QtSizePropertyManagerPrivate::Data::maximumValue,
                &QtSizePropertyManagerPrivate::Data::setMaximumValue,
                maxVal, &QtSizePropertyManagerPrivate::setRange);
}

/*!
    \fn void QtSizePropertyManager::setRange(QtProperty *property, const QSize &minimum, const QSize &maximum)

    Sets the range of valid values.

    This is a convenience function defining the range of valid values
    in one go; setting the \a minimum and \a maximum values for the
    given \a property with a single function call.

    When setting a new range, the current value is adjusted if
    necessary (ensuring that the value remains within the range).

    \sa  setMinimum(), setMaximum(), rangeChanged()
*/
void QtSizePropertyManager::setRange(QtProperty *property, const QSize &minVal, const QSize &maxVal)
{
    setBorderValues<const QSize &, QtSizePropertyManagerPrivate, QtSizePropertyManager, QSize>(this, d_ptr,
                &QtSizePropertyManager::propertyChanged,
                &QtSizePropertyManager::valueChanged,
                &QtSizePropertyManager::rangeChanged,
                property, minVal, maxVal, &QtSizePropertyManagerPrivate::setRange);
}

/*!
    \reimp
*/
void QtSizePropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtSizePropertyManagerPrivate::Data();

    QtProperty *wProp = d_ptr->m_intPropertyManager->addProperty();
    wProp->setPropertyName(tr("Width"));
    d_ptr->m_intPropertyManager->setValue(wProp, 0);
    d_ptr->m_intPropertyManager->setMinimum(wProp, 0);
    d_ptr->m_propertyToW[property] = wProp;
    d_ptr->m_wToProperty[wProp] = property;
    property->addSubProperty(wProp);

    QtProperty *hProp = d_ptr->m_intPropertyManager->addProperty();
    hProp->setPropertyName(tr("Height"));
    d_ptr->m_intPropertyManager->setValue(hProp, 0);
    d_ptr->m_intPropertyManager->setMinimum(hProp, 0);
    d_ptr->m_propertyToH[property] = hProp;
    d_ptr->m_hToProperty[hProp] = property;
    property->addSubProperty(hProp);
}

/*!
    \reimp
*/
void QtSizePropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *wProp = d_ptr->m_propertyToW[property];
    if (wProp) {
        d_ptr->m_wToProperty.remove(wProp);
        delete wProp;
    }
    d_ptr->m_propertyToW.remove(property);

    QtProperty *hProp = d_ptr->m_propertyToH[property];
    if (hProp) {
        d_ptr->m_hToProperty.remove(hProp);
        delete hProp;
    }
    d_ptr->m_propertyToH.remove(property);

    d_ptr->m_values.remove(property);
}

// QtSizeFPropertyManager

class QtSizeFPropertyManagerPrivate
{
    QtSizeFPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtSizeFPropertyManager)
public:

    void slotDoubleChanged(QtProperty *property, double value);
    void slotPropertyDestroyed(QtProperty *property);
    void setValue(QtProperty *property, const QSizeF &val);
    void setRange(QtProperty *property,
                const QSizeF &minVal, const QSizeF &maxVal, const QSizeF &val);

    struct Data
    {
        Data() : val(QSizeF(0, 0)), minVal(QSizeF(0, 0)), maxVal(QSizeF(INT_MAX, INT_MAX)), decimals(2) {}
        QSizeF val;
        QSizeF minVal;
        QSizeF maxVal;
        int decimals;
        QSizeF minimumValue() const { return minVal; }
        QSizeF maximumValue() const { return maxVal; }
        void setMinimumValue(const QSizeF &newMinVal) { setSizeMinimumData(this, newMinVal); }
        void setMaximumValue(const QSizeF &newMaxVal) { setSizeMaximumData(this, newMaxVal); }
    };

    typedef QMap<const QtProperty *, Data> PropertyValueMap;
    PropertyValueMap m_values;

    QtDoublePropertyManager *m_doublePropertyManager;

    QMap<const QtProperty *, QtProperty *> m_propertyToW;
    QMap<const QtProperty *, QtProperty *> m_propertyToH;

    QMap<const QtProperty *, QtProperty *> m_wToProperty;
    QMap<const QtProperty *, QtProperty *> m_hToProperty;
};

void QtSizeFPropertyManagerPrivate::slotDoubleChanged(QtProperty *property, double value)
{
    if (QtProperty *prop = m_wToProperty.value(property, nullptr)) {
        QSizeF s = m_values[prop].val;
        s.setWidth(value);
        q_ptr->setValue(prop, s);
    } else if (QtProperty *prop = m_hToProperty.value(property, nullptr)) {
        QSizeF s = m_values[prop].val;
        s.setHeight(value);
        q_ptr->setValue(prop, s);
    }
}

void QtSizeFPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (QtProperty *pointProp = m_wToProperty.value(property, nullptr)) {
        m_propertyToW[pointProp] = nullptr;
        m_wToProperty.remove(property);
    } else if (QtProperty *pointProp = m_hToProperty.value(property, nullptr)) {
        m_propertyToH[pointProp] = nullptr;
        m_hToProperty.remove(property);
    }
}

void QtSizeFPropertyManagerPrivate::setValue(QtProperty *property, const QSizeF &val)
{
    m_doublePropertyManager->setValue(m_propertyToW.value(property), val.width());
    m_doublePropertyManager->setValue(m_propertyToH.value(property), val.height());
}

void QtSizeFPropertyManagerPrivate::setRange(QtProperty *property,
                const QSizeF &minVal, const QSizeF &maxVal, const QSizeF &val)
{
    m_doublePropertyManager->setRange(m_propertyToW[property], minVal.width(), maxVal.width());
    m_doublePropertyManager->setValue(m_propertyToW[property], val.width());
    m_doublePropertyManager->setRange(m_propertyToH[property], minVal.height(), maxVal.height());
    m_doublePropertyManager->setValue(m_propertyToH[property], val.height());
}

/*!
    \class QtSizeFPropertyManager

    \brief The QtSizeFPropertyManager provides and manages QSizeF properties.

    A size property has nested \e width and \e height
    subproperties. The top-level property's value can be retrieved
    using the value() function, and set using the setValue() slot.

    The subproperties are created by a QtDoublePropertyManager object. This
    manager can be retrieved using the subDoublePropertyManager() function. In
    order to provide editing widgets for the subproperties in a
    property browser widget, this manager must be associated with an
    editor factory.

    A size property also has a range of valid values defined by a
    minimum size and a maximum size. These sizes can be retrieved
    using the minimum() and the maximum() functions, and set using the
    setMinimum() and setMaximum() slots. Alternatively, the range can
    be defined in one go using the setRange() slot.

    In addition, QtSizeFPropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes, and the rangeChanged() signal which is emitted whenever
    such a property changes its range of valid sizes.

    \sa QtAbstractPropertyManager, QtDoublePropertyManager, QtSizePropertyManager
*/

/*!
    \fn void QtSizeFPropertyManager::valueChanged(QtProperty *property, const QSizeF &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.

    \sa setValue()
*/

/*!
    \fn void QtSizeFPropertyManager::rangeChanged(QtProperty *property, const QSizeF &minimum, const QSizeF &maximum)

    This signal is emitted whenever a property created by this manager
    changes its range of valid sizes, passing a pointer to the \a
    property and the new \a minimum and \a maximum sizes.

    \sa setRange()
*/

/*!
    \fn void QtSizeFPropertyManager::decimalsChanged(QtProperty *property, int prec)

    This signal is emitted whenever a property created by this manager
    changes its precision of value, passing a pointer to the
    \a property and the new \a prec value

    \sa setDecimals()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtSizeFPropertyManager::QtSizeFPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtSizeFPropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_doublePropertyManager = new QtDoublePropertyManager(this);
    connect(d_ptr->m_doublePropertyManager, SIGNAL(valueChanged(QtProperty *, double)),
                this, SLOT(slotDoubleChanged(QtProperty *, double)));
    connect(d_ptr->m_doublePropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtSizeFPropertyManager::~QtSizeFPropertyManager()
{
    clear();
    delete d_ptr;
}

/*!
    Returns the manager that creates the nested \e width and \e height
    subproperties.

    In order to provide editing widgets for the \e width and \e height
    properties in a property browser widget, this manager must be
    associated with an editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtDoublePropertyManager *QtSizeFPropertyManager::subDoublePropertyManager() const
{
    return d_ptr->m_doublePropertyManager;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by this manager, this
    function returns an invalid size

    \sa setValue()
*/
QSizeF QtSizeFPropertyManager::value(const QtProperty *property) const
{
    return getValue<QSizeF>(d_ptr->m_values, property);
}

/*!
    Returns the given \a property's precision, in decimals.

    \sa setDecimals()
*/
int QtSizeFPropertyManager::decimals(const QtProperty *property) const
{
    return getData<int>(d_ptr->m_values, &QtSizeFPropertyManagerPrivate::Data::decimals, property, 0);
}

/*!
    Returns the given \a property's minimum size value.

    \sa setMinimum(), maximum(), setRange()
*/
QSizeF QtSizeFPropertyManager::minimum(const QtProperty *property) const
{
    return getMinimum<QSizeF>(d_ptr->m_values, property);
}

/*!
    Returns the given \a property's maximum size value.

    \sa setMaximum(), minimum(), setRange()
*/
QSizeF QtSizeFPropertyManager::maximum(const QtProperty *property) const
{
    return getMaximum<QSizeF>(d_ptr->m_values, property);
}

/*!
    \reimp
*/
QString QtSizeFPropertyManager::valueText(const QtProperty *property) const
{
    const QtSizeFPropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QString();
    const QSizeF v = it.value().val;
    const int dec = it.value().decimals;
    return QString(tr("%1 x %2").arg(QString::number(v.width(), 'f', dec))
                                .arg(QString::number(v.height(), 'f', dec)));
}

/*!
    \fn void QtSizeFPropertyManager::setValue(QtProperty *property, const QSizeF &value)

    Sets the value of the given \a property to \a value.

    If the specified \a value is not valid according to the given \a
    property's size range, the \a value is adjusted to the nearest
    valid value within the size range.

    \sa value(), setRange(), valueChanged()
*/
void QtSizeFPropertyManager::setValue(QtProperty *property, const QSizeF &val)
{
    setValueInRange<const QSizeF &, QtSizeFPropertyManagerPrivate, QtSizeFPropertyManager, QSizeF>(this, d_ptr,
                &QtSizeFPropertyManager::propertyChanged,
                &QtSizeFPropertyManager::valueChanged,
                property, val, &QtSizeFPropertyManagerPrivate::setValue);
}

/*!
    \fn void QtSizeFPropertyManager::setDecimals(QtProperty *property, int prec)

    Sets the precision of the given \a property to \a prec.

    The valid decimal range is 0-13. The default is 2.

    \sa decimals()
*/
void QtSizeFPropertyManager::setDecimals(QtProperty *property, int prec)
{
    const QtSizeFPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    QtSizeFPropertyManagerPrivate::Data data = it.value();

    if (prec > 13)
        prec = 13;
    else if (prec < 0)
        prec = 0;

    if (data.decimals == prec)
        return;

    data.decimals = prec;
    d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToW[property], prec);
    d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToH[property], prec);

    it.value() = data;

    emit decimalsChanged(property, data.decimals);
}

/*!
    Sets the minimum size value for the given \a property to \a minVal.

    When setting the minimum size value, the maximum and current
    values are adjusted if necessary (ensuring that the size range
    remains valid and that the current value is within the range).

    \sa minimum(), setRange(), rangeChanged()
*/
void QtSizeFPropertyManager::setMinimum(QtProperty *property, const QSizeF &minVal)
{
    setBorderValue<const QSizeF &, QtSizeFPropertyManagerPrivate, QtSizeFPropertyManager, QSizeF, QtSizeFPropertyManagerPrivate::Data>(this, d_ptr,
                &QtSizeFPropertyManager::propertyChanged,
                &QtSizeFPropertyManager::valueChanged,
                &QtSizeFPropertyManager::rangeChanged,
                property,
                &QtSizeFPropertyManagerPrivate::Data::minimumValue,
                &QtSizeFPropertyManagerPrivate::Data::setMinimumValue,
                minVal, &QtSizeFPropertyManagerPrivate::setRange);
}

/*!
    Sets the maximum size value for the given \a property to \a maxVal.

    When setting the maximum size value, the minimum and current
    values are adjusted if necessary (ensuring that the size range
    remains valid and that the current value is within the range).

    \sa maximum(), setRange(), rangeChanged()
*/
void QtSizeFPropertyManager::setMaximum(QtProperty *property, const QSizeF &maxVal)
{
    setBorderValue<const QSizeF &, QtSizeFPropertyManagerPrivate, QtSizeFPropertyManager, QSizeF, QtSizeFPropertyManagerPrivate::Data>(this, d_ptr,
                &QtSizeFPropertyManager::propertyChanged,
                &QtSizeFPropertyManager::valueChanged,
                &QtSizeFPropertyManager::rangeChanged,
                property,
                &QtSizeFPropertyManagerPrivate::Data::maximumValue,
                &QtSizeFPropertyManagerPrivate::Data::setMaximumValue,
                maxVal, &QtSizeFPropertyManagerPrivate::setRange);
}

/*!
    \fn void QtSizeFPropertyManager::setRange(QtProperty *property, const QSizeF &minimum, const QSizeF &maximum)

    Sets the range of valid values.

    This is a convenience function defining the range of valid values
    in one go; setting the \a minimum and \a maximum values for the
    given \a property with a single function call.

    When setting a new range, the current value is adjusted if
    necessary (ensuring that the value remains within the range).

    \sa  setMinimum(), setMaximum(), rangeChanged()
*/
void QtSizeFPropertyManager::setRange(QtProperty *property, const QSizeF &minVal, const QSizeF &maxVal)
{
    setBorderValues<const QSizeF &, QtSizeFPropertyManagerPrivate, QtSizeFPropertyManager, QSizeF>(this, d_ptr,
                &QtSizeFPropertyManager::propertyChanged,
                &QtSizeFPropertyManager::valueChanged,
                &QtSizeFPropertyManager::rangeChanged,
                property, minVal, maxVal, &QtSizeFPropertyManagerPrivate::setRange);
}

/*!
    \reimp
*/
void QtSizeFPropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtSizeFPropertyManagerPrivate::Data();

    QtProperty *wProp = d_ptr->m_doublePropertyManager->addProperty();
    wProp->setPropertyName(tr("Width"));
    d_ptr->m_doublePropertyManager->setDecimals(wProp, decimals(property));
    d_ptr->m_doublePropertyManager->setValue(wProp, 0);
    d_ptr->m_doublePropertyManager->setMinimum(wProp, 0);
    d_ptr->m_propertyToW[property] = wProp;
    d_ptr->m_wToProperty[wProp] = property;
    property->addSubProperty(wProp);

    QtProperty *hProp = d_ptr->m_doublePropertyManager->addProperty();
    hProp->setPropertyName(tr("Height"));
    d_ptr->m_doublePropertyManager->setDecimals(hProp, decimals(property));
    d_ptr->m_doublePropertyManager->setValue(hProp, 0);
    d_ptr->m_doublePropertyManager->setMinimum(hProp, 0);
    d_ptr->m_propertyToH[property] = hProp;
    d_ptr->m_hToProperty[hProp] = property;
    property->addSubProperty(hProp);
}

/*!
    \reimp
*/
void QtSizeFPropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *wProp = d_ptr->m_propertyToW[property];
    if (wProp) {
        d_ptr->m_wToProperty.remove(wProp);
        delete wProp;
    }
    d_ptr->m_propertyToW.remove(property);

    QtProperty *hProp = d_ptr->m_propertyToH[property];
    if (hProp) {
        d_ptr->m_hToProperty.remove(hProp);
        delete hProp;
    }
    d_ptr->m_propertyToH.remove(property);

    d_ptr->m_values.remove(property);
}

// QtRectPropertyManager

class QtRectPropertyManagerPrivate
{
    QtRectPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtRectPropertyManager)
public:

    void slotIntChanged(QtProperty *property, int value);
    void slotPropertyDestroyed(QtProperty *property);
    void setConstraint(QtProperty *property, const QRect &constraint, const QRect &val);

    struct Data
    {
        Data() : val(0, 0, 0, 0) {}
        QRect val;
        QRect constraint;
    };

    typedef QMap<const QtProperty *, Data> PropertyValueMap;
    PropertyValueMap m_values;

    QtIntPropertyManager *m_intPropertyManager;

    QMap<const QtProperty *, QtProperty *> m_propertyToX;
    QMap<const QtProperty *, QtProperty *> m_propertyToY;
    QMap<const QtProperty *, QtProperty *> m_propertyToW;
    QMap<const QtProperty *, QtProperty *> m_propertyToH;

    QMap<const QtProperty *, QtProperty *> m_xToProperty;
    QMap<const QtProperty *, QtProperty *> m_yToProperty;
    QMap<const QtProperty *, QtProperty *> m_wToProperty;
    QMap<const QtProperty *, QtProperty *> m_hToProperty;
};

void QtRectPropertyManagerPrivate::slotIntChanged(QtProperty *property, int value)
{
    if (QtProperty *prop = m_xToProperty.value(property, nullptr)) {
        QRect r = m_values[prop].val;
        r.moveLeft(value);
        q_ptr->setValue(prop, r);
    } else if (QtProperty *prop = m_yToProperty.value(property)) {
        QRect r = m_values[prop].val;
        r.moveTop(value);
        q_ptr->setValue(prop, r);
    } else if (QtProperty *prop = m_wToProperty.value(property, nullptr)) {
        Data data = m_values[prop];
        QRect r = data.val;
        r.setWidth(value);
        if (!data.constraint.isNull() && data.constraint.x() + data.constraint.width() < r.x() + r.width()) {
            r.moveLeft(data.constraint.left() + data.constraint.width() - r.width());
        }
        q_ptr->setValue(prop, r);
    } else if (QtProperty *prop = m_hToProperty.value(property, nullptr)) {
        Data data = m_values[prop];
        QRect r = data.val;
        r.setHeight(value);
        if (!data.constraint.isNull() && data.constraint.y() + data.constraint.height() < r.y() + r.height()) {
            r.moveTop(data.constraint.top() + data.constraint.height() - r.height());
        }
        q_ptr->setValue(prop, r);
    }
}

void QtRectPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (QtProperty *pointProp = m_xToProperty.value(property, nullptr)) {
        m_propertyToX[pointProp] = nullptr;
        m_xToProperty.remove(property);
    } else if (QtProperty *pointProp = m_yToProperty.value(property, nullptr)) {
        m_propertyToY[pointProp] = nullptr;
        m_yToProperty.remove(property);
    } else if (QtProperty *pointProp = m_wToProperty.value(property, nullptr)) {
        m_propertyToW[pointProp] = nullptr;
        m_wToProperty.remove(property);
    } else if (QtProperty *pointProp = m_hToProperty.value(property, nullptr)) {
        m_propertyToH[pointProp] = nullptr;
        m_hToProperty.remove(property);
    }
}

void QtRectPropertyManagerPrivate::setConstraint(QtProperty *property,
            const QRect &constraint, const QRect &val)
{
    const bool isNull = constraint.isNull();
    const int left   = isNull ? INT_MIN : constraint.left();
    const int right  = isNull ? INT_MAX : constraint.left() + constraint.width();
    const int top    = isNull ? INT_MIN : constraint.top();
    const int bottom = isNull ? INT_MAX : constraint.top() + constraint.height();
    const int width  = isNull ? INT_MAX : constraint.width();
    const int height = isNull ? INT_MAX : constraint.height();

    m_intPropertyManager->setRange(m_propertyToX[property], left, right);
    m_intPropertyManager->setRange(m_propertyToY[property], top, bottom);
    m_intPropertyManager->setRange(m_propertyToW[property], 0, width);
    m_intPropertyManager->setRange(m_propertyToH[property], 0, height);

    m_intPropertyManager->setValue(m_propertyToX[property], val.x());
    m_intPropertyManager->setValue(m_propertyToY[property], val.y());
    m_intPropertyManager->setValue(m_propertyToW[property], val.width());
    m_intPropertyManager->setValue(m_propertyToH[property], val.height());
}

/*!
    \class QtRectPropertyManager

    \brief The QtRectPropertyManager provides and manages QRect properties.

    A rectangle property has nested \e x, \e y, \e width and \e height
    subproperties. The top-level property's value can be retrieved
    using the value() function, and set using the setValue() slot.

    The subproperties are created by a QtIntPropertyManager object. This
    manager can be retrieved using the subIntPropertyManager() function. In
    order to provide editing widgets for the subproperties in a
    property browser widget, this manager must be associated with an
    editor factory.

    A rectangle property also has a constraint rectangle which can be
    retrieved using the constraint() function, and set using the
    setConstraint() slot.

    In addition, QtRectPropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes, and the constraintChanged() signal which is emitted
    whenever such a property changes its constraint rectangle.

    \sa QtAbstractPropertyManager, QtIntPropertyManager, QtRectFPropertyManager
*/

/*!
    \fn void QtRectPropertyManager::valueChanged(QtProperty *property, const QRect &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.

    \sa setValue()
*/

/*!
    \fn void QtRectPropertyManager::constraintChanged(QtProperty *property, const QRect &constraint)

    This signal is emitted whenever property changes its constraint
    rectangle, passing a pointer to the \a property and the new \a
    constraint rectangle as parameters.

    \sa setConstraint()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtRectPropertyManager::QtRectPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtRectPropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_intPropertyManager = new QtIntPropertyManager(this);
    connect(d_ptr->m_intPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotIntChanged(QtProperty *, int)));
    connect(d_ptr->m_intPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtRectPropertyManager::~QtRectPropertyManager()
{
    clear();
    delete d_ptr;
}

/*!
    Returns the manager that creates the nested \e x, \e y, \e width
    and \e height subproperties.

    In order to provide editing widgets for the mentioned
    subproperties in a property browser widget, this manager must be
    associated with an editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtIntPropertyManager *QtRectPropertyManager::subIntPropertyManager() const
{
    return d_ptr->m_intPropertyManager;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by this manager, this
    function returns an invalid rectangle.

    \sa setValue(), constraint()
*/
QRect QtRectPropertyManager::value(const QtProperty *property) const
{
    return getValue<QRect>(d_ptr->m_values, property);
}

/*!
    Returns the given \a property's constraining rectangle. If returned value is null QRect it means there is no constraint applied.

    \sa value(), setConstraint()
*/
QRect QtRectPropertyManager::constraint(const QtProperty *property) const
{
    return getData<QRect>(d_ptr->m_values, &QtRectPropertyManagerPrivate::Data::constraint, property, QRect());
}

/*!
    \reimp
*/
QString QtRectPropertyManager::valueText(const QtProperty *property) const
{
    const QtRectPropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QString();
    const QRect v = it.value().val;
    return QString(tr("[(%1, %2), %3 x %4]").arg(QString::number(v.x()))
                                .arg(QString::number(v.y()))
                                .arg(QString::number(v.width()))
                                .arg(QString::number(v.height())));
}

/*!
    \fn void QtRectPropertyManager::setValue(QtProperty *property, const QRect &value)

    Sets the value of the given \a property to \a value. Nested
    properties are updated automatically.

    If the specified \a value is not inside the given \a property's
    constraining rectangle, the value is adjusted accordingly to fit
    within the constraint.

    \sa value(), setConstraint(), valueChanged()
*/
void QtRectPropertyManager::setValue(QtProperty *property, const QRect &val)
{
    const QtRectPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    QtRectPropertyManagerPrivate::Data data = it.value();

    QRect newRect = val.normalized();
    if (!data.constraint.isNull() && !data.constraint.contains(newRect)) {
        const QRect r1 = data.constraint;
        const QRect r2 = newRect;
        newRect.setLeft(qMax(r1.left(), r2.left()));
        newRect.setRight(qMin(r1.right(), r2.right()));
        newRect.setTop(qMax(r1.top(), r2.top()));
        newRect.setBottom(qMin(r1.bottom(), r2.bottom()));
        if (newRect.width() < 0 || newRect.height() < 0)
            return;
    }

    if (data.val == newRect)
        return;

    data.val = newRect;

    it.value() = data;
    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToX[property], newRect.x());
    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToY[property], newRect.y());
    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToW[property], newRect.width());
    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToH[property], newRect.height());

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    Sets the given \a property's constraining rectangle to \a
    constraint.

    When setting the constraint, the current value is adjusted if
    necessary (ensuring that the current rectangle value is inside the
    constraint). In order to reset the constraint pass a null QRect value.

    \sa setValue(), constraint(), constraintChanged()
*/
void QtRectPropertyManager::setConstraint(QtProperty *property, const QRect &constraint)
{
    const QtRectPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    QtRectPropertyManagerPrivate::Data data = it.value();

    QRect newConstraint = constraint.normalized();
    if (data.constraint == newConstraint)
        return;

    const QRect oldVal = data.val;

    data.constraint = newConstraint;

    if (!data.constraint.isNull() && !data.constraint.contains(oldVal)) {
        QRect r1 = data.constraint;
        QRect r2 = data.val;

        if (r2.width() > r1.width())
            r2.setWidth(r1.width());
        if (r2.height() > r1.height())
            r2.setHeight(r1.height());
        if (r2.left() < r1.left())
            r2.moveLeft(r1.left());
        else if (r2.right() > r1.right())
            r2.moveRight(r1.right());
        if (r2.top() < r1.top())
            r2.moveTop(r1.top());
        else if (r2.bottom() > r1.bottom())
            r2.moveBottom(r1.bottom());

        data.val = r2;
    }

    it.value() = data;

    emit constraintChanged(property, data.constraint);

    d_ptr->setConstraint(property, data.constraint, data.val);

    if (data.val == oldVal)
        return;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    \reimp
*/
void QtRectPropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtRectPropertyManagerPrivate::Data();

    QtProperty *xProp = d_ptr->m_intPropertyManager->addProperty();
    xProp->setPropertyName(tr("X"));
    d_ptr->m_intPropertyManager->setValue(xProp, 0);
    d_ptr->m_propertyToX[property] = xProp;
    d_ptr->m_xToProperty[xProp] = property;
    property->addSubProperty(xProp);

    QtProperty *yProp = d_ptr->m_intPropertyManager->addProperty();
    yProp->setPropertyName(tr("Y"));
    d_ptr->m_intPropertyManager->setValue(yProp, 0);
    d_ptr->m_propertyToY[property] = yProp;
    d_ptr->m_yToProperty[yProp] = property;
    property->addSubProperty(yProp);

    QtProperty *wProp = d_ptr->m_intPropertyManager->addProperty();
    wProp->setPropertyName(tr("Width"));
    d_ptr->m_intPropertyManager->setValue(wProp, 0);
    d_ptr->m_intPropertyManager->setMinimum(wProp, 0);
    d_ptr->m_propertyToW[property] = wProp;
    d_ptr->m_wToProperty[wProp] = property;
    property->addSubProperty(wProp);

    QtProperty *hProp = d_ptr->m_intPropertyManager->addProperty();
    hProp->setPropertyName(tr("Height"));
    d_ptr->m_intPropertyManager->setValue(hProp, 0);
    d_ptr->m_intPropertyManager->setMinimum(hProp, 0);
    d_ptr->m_propertyToH[property] = hProp;
    d_ptr->m_hToProperty[hProp] = property;
    property->addSubProperty(hProp);
}

/*!
    \reimp
*/
void QtRectPropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *xProp = d_ptr->m_propertyToX[property];
    if (xProp) {
        d_ptr->m_xToProperty.remove(xProp);
        delete xProp;
    }
    d_ptr->m_propertyToX.remove(property);

    QtProperty *yProp = d_ptr->m_propertyToY[property];
    if (yProp) {
        d_ptr->m_yToProperty.remove(yProp);
        delete yProp;
    }
    d_ptr->m_propertyToY.remove(property);

    QtProperty *wProp = d_ptr->m_propertyToW[property];
    if (wProp) {
        d_ptr->m_wToProperty.remove(wProp);
        delete wProp;
    }
    d_ptr->m_propertyToW.remove(property);

    QtProperty *hProp = d_ptr->m_propertyToH[property];
    if (hProp) {
        d_ptr->m_hToProperty.remove(hProp);
        delete hProp;
    }
    d_ptr->m_propertyToH.remove(property);

    d_ptr->m_values.remove(property);
}

// QtRectFPropertyManager

class QtRectFPropertyManagerPrivate
{
    QtRectFPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtRectFPropertyManager)
public:

    void slotDoubleChanged(QtProperty *property, double value);
    void slotPropertyDestroyed(QtProperty *property);
    void setConstraint(QtProperty *property, const QRectF &constraint, const QRectF &val);

    struct Data
    {
        Data() : val(0, 0, 0, 0), decimals(2) {}
        QRectF val;
        QRectF constraint;
        int decimals;
    };

    typedef QMap<const QtProperty *, Data> PropertyValueMap;
    PropertyValueMap m_values;

    QtDoublePropertyManager *m_doublePropertyManager;

    QMap<const QtProperty *, QtProperty *> m_propertyToX;
    QMap<const QtProperty *, QtProperty *> m_propertyToY;
    QMap<const QtProperty *, QtProperty *> m_propertyToW;
    QMap<const QtProperty *, QtProperty *> m_propertyToH;

    QMap<const QtProperty *, QtProperty *> m_xToProperty;
    QMap<const QtProperty *, QtProperty *> m_yToProperty;
    QMap<const QtProperty *, QtProperty *> m_wToProperty;
    QMap<const QtProperty *, QtProperty *> m_hToProperty;
};

void QtRectFPropertyManagerPrivate::slotDoubleChanged(QtProperty *property, double value)
{
    if (QtProperty *prop = m_xToProperty.value(property, nullptr)) {
        QRectF r = m_values[prop].val;
        r.moveLeft(value);
        q_ptr->setValue(prop, r);
    } else if (QtProperty *prop = m_yToProperty.value(property, nullptr)) {
        QRectF r = m_values[prop].val;
        r.moveTop(value);
        q_ptr->setValue(prop, r);
    } else if (QtProperty *prop = m_wToProperty.value(property, nullptr)) {
        Data data = m_values[prop];
        QRectF r = data.val;
        r.setWidth(value);
        if (!data.constraint.isNull() && data.constraint.x() + data.constraint.width() < r.x() + r.width()) {
            r.moveLeft(data.constraint.left() + data.constraint.width() - r.width());
        }
        q_ptr->setValue(prop, r);
    } else if (QtProperty *prop = m_hToProperty.value(property, nullptr)) {
        Data data = m_values[prop];
        QRectF r = data.val;
        r.setHeight(value);
        if (!data.constraint.isNull() && data.constraint.y() + data.constraint.height() < r.y() + r.height()) {
            r.moveTop(data.constraint.top() + data.constraint.height() - r.height());
        }
        q_ptr->setValue(prop, r);
    }
}

void QtRectFPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (QtProperty *pointProp = m_xToProperty.value(property, nullptr)) {
        m_propertyToX[pointProp] = nullptr;
        m_xToProperty.remove(property);
    } else if (QtProperty *pointProp = m_yToProperty.value(property, nullptr)) {
        m_propertyToY[pointProp] = nullptr;
        m_yToProperty.remove(property);
    } else if (QtProperty *pointProp = m_wToProperty.value(property, nullptr)) {
        m_propertyToW[pointProp] = nullptr;
        m_wToProperty.remove(property);
    } else if (QtProperty *pointProp = m_hToProperty.value(property, nullptr)) {
        m_propertyToH[pointProp] = nullptr;
        m_hToProperty.remove(property);
    }
}

void QtRectFPropertyManagerPrivate::setConstraint(QtProperty *property,
            const QRectF &constraint, const QRectF &val)
{
    const bool isNull = constraint.isNull();
    const float left   = isNull ? FLT_MIN : float(constraint.left());
    const float right  = isNull ? FLT_MAX : float(constraint.left() + constraint.width());
    const float top    = isNull ? FLT_MIN : float(constraint.top());
    const float bottom = isNull ? FLT_MAX : float(constraint.top() + constraint.height());
    const float width  = isNull ? FLT_MAX : float(constraint.width());
    const float height = isNull ? FLT_MAX : float(constraint.height());

    m_doublePropertyManager->setRange(m_propertyToX[property], double(left), double(right));
    m_doublePropertyManager->setRange(m_propertyToY[property], double(top), double(bottom));
    m_doublePropertyManager->setRange(m_propertyToW[property], 0, double(width));
    m_doublePropertyManager->setRange(m_propertyToH[property], 0, double(height));

    m_doublePropertyManager->setValue(m_propertyToX[property], val.x());
    m_doublePropertyManager->setValue(m_propertyToY[property], val.y());
    m_doublePropertyManager->setValue(m_propertyToW[property], val.width());
    m_doublePropertyManager->setValue(m_propertyToH[property], val.height());
}

/*!
    \class QtRectFPropertyManager

    \brief The QtRectFPropertyManager provides and manages QRectF properties.

    A rectangle property has nested \e x, \e y, \e width and \e height
    subproperties. The top-level property's value can be retrieved
    using the value() function, and set using the setValue() slot.

    The subproperties are created by a QtDoublePropertyManager object. This
    manager can be retrieved using the subDoublePropertyManager() function. In
    order to provide editing widgets for the subproperties in a
    property browser widget, this manager must be associated with an
    editor factory.

    A rectangle property also has a constraint rectangle which can be
    retrieved using the constraint() function, and set using the
    setConstraint() slot.

    In addition, QtRectFPropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes, and the constraintChanged() signal which is emitted
    whenever such a property changes its constraint rectangle.

    \sa QtAbstractPropertyManager, QtDoublePropertyManager, QtRectPropertyManager
*/

/*!
    \fn void QtRectFPropertyManager::valueChanged(QtProperty *property, const QRectF &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.

    \sa setValue()
*/

/*!
    \fn void QtRectFPropertyManager::constraintChanged(QtProperty *property, const QRectF &constraint)

    This signal is emitted whenever property changes its constraint
    rectangle, passing a pointer to the \a property and the new \a
    constraint rectangle as parameters.

    \sa setConstraint()
*/

/*!
    \fn void QtRectFPropertyManager::decimalsChanged(QtProperty *property, int prec)

    This signal is emitted whenever a property created by this manager
    changes its precision of value, passing a pointer to the
    \a property and the new \a prec value

    \sa setDecimals()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtRectFPropertyManager::QtRectFPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtRectFPropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_doublePropertyManager = new QtDoublePropertyManager(this);
    connect(d_ptr->m_doublePropertyManager, SIGNAL(valueChanged(QtProperty *, double)),
                this, SLOT(slotDoubleChanged(QtProperty *, double)));
    connect(d_ptr->m_doublePropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtRectFPropertyManager::~QtRectFPropertyManager()
{
    clear();
    delete d_ptr;
}

/*!
    Returns the manager that creates the nested \e x, \e y, \e width
    and \e height subproperties.

    In order to provide editing widgets for the mentioned
    subproperties in a property browser widget, this manager must be
    associated with an editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtDoublePropertyManager *QtRectFPropertyManager::subDoublePropertyManager() const
{
    return d_ptr->m_doublePropertyManager;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by this manager, this
    function returns an invalid rectangle.

    \sa setValue(), constraint()
*/
QRectF QtRectFPropertyManager::value(const QtProperty *property) const
{
    return getValue<QRectF>(d_ptr->m_values, property);
}

/*!
    Returns the given \a property's precision, in decimals.

    \sa setDecimals()
*/
int QtRectFPropertyManager::decimals(const QtProperty *property) const
{
    return getData<int>(d_ptr->m_values, &QtRectFPropertyManagerPrivate::Data::decimals, property, 0);
}

/*!
    Returns the given \a property's constraining rectangle. If returned value is null QRectF it means there is no constraint applied.

    \sa value(), setConstraint()
*/
QRectF QtRectFPropertyManager::constraint(const QtProperty *property) const
{
    return getData<QRectF>(d_ptr->m_values, &QtRectFPropertyManagerPrivate::Data::constraint, property, QRect());
}

/*!
    \reimp
*/
QString QtRectFPropertyManager::valueText(const QtProperty *property) const
{
    const QtRectFPropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QString();
    const QRectF v = it.value().val;
    const int dec = it.value().decimals;
    return QString(tr("[(%1, %2), %3 x %4]").arg(QString::number(v.x(), 'f', dec))
                                .arg(QString::number(v.y(), 'f', dec))
                                .arg(QString::number(v.width(), 'f', dec))
                                .arg(QString::number(v.height(), 'f', dec)));
}

/*!
    \fn void QtRectFPropertyManager::setValue(QtProperty *property, const QRectF &value)

    Sets the value of the given \a property to \a value. Nested
    properties are updated automatically.

    If the specified \a value is not inside the given \a property's
    constraining rectangle, the value is adjusted accordingly to fit
    within the constraint.

    \sa value(), setConstraint(), valueChanged()
*/
void QtRectFPropertyManager::setValue(QtProperty *property, const QRectF &val)
{
    const QtRectFPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    QtRectFPropertyManagerPrivate::Data data = it.value();

    QRectF newRect = val.normalized();
    if (!data.constraint.isNull() && !data.constraint.contains(newRect)) {
        const QRectF r1 = data.constraint;
        const QRectF r2 = newRect;
        newRect.setLeft(qMax(r1.left(), r2.left()));
        newRect.setRight(qMin(r1.right(), r2.right()));
        newRect.setTop(qMax(r1.top(), r2.top()));
        newRect.setBottom(qMin(r1.bottom(), r2.bottom()));
        if (newRect.width() < 0 || newRect.height() < 0)
            return;
    }

    if (data.val == newRect)
        return;

    data.val = newRect;

    it.value() = data;
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToX[property], newRect.x());
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToY[property], newRect.y());
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToW[property], newRect.width());
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToH[property], newRect.height());

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    Sets the given \a property's constraining rectangle to \a
    constraint.

    When setting the constraint, the current value is adjusted if
    necessary (ensuring that the current rectangle value is inside the
    constraint). In order to reset the constraint pass a null QRectF value.

    \sa setValue(), constraint(), constraintChanged()
*/
void QtRectFPropertyManager::setConstraint(QtProperty *property, const QRectF &constraint)
{
    const QtRectFPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    QtRectFPropertyManagerPrivate::Data data = it.value();

    QRectF newConstraint = constraint.normalized();
    if (data.constraint == newConstraint)
        return;

    const QRectF oldVal = data.val;

    data.constraint = newConstraint;

    if (!data.constraint.isNull() && !data.constraint.contains(oldVal)) {
        QRectF r1 = data.constraint;
        QRectF r2 = data.val;

        if (r2.width() > r1.width())
            r2.setWidth(r1.width());
        if (r2.height() > r1.height())
            r2.setHeight(r1.height());
        if (r2.left() < r1.left())
            r2.moveLeft(r1.left());
        else if (r2.right() > r1.right())
            r2.moveRight(r1.right());
        if (r2.top() < r1.top())
            r2.moveTop(r1.top());
        else if (r2.bottom() > r1.bottom())
            r2.moveBottom(r1.bottom());

        data.val = r2;
    }

    it.value() = data;

    emit constraintChanged(property, data.constraint);

    d_ptr->setConstraint(property, data.constraint, data.val);

    if (data.val == oldVal)
        return;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    \fn void QtRectFPropertyManager::setDecimals(QtProperty *property, int prec)

    Sets the precision of the given \a property to \a prec.

    The valid decimal range is 0-13. The default is 2.

    \sa decimals()
*/
void QtRectFPropertyManager::setDecimals(QtProperty *property, int prec)
{
    const QtRectFPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    QtRectFPropertyManagerPrivate::Data data = it.value();

    if (prec > 13)
        prec = 13;
    else if (prec < 0)
        prec = 0;

    if (data.decimals == prec)
        return;

    data.decimals = prec;
    d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToX[property], prec);
    d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToY[property], prec);
    d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToW[property], prec);
    d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToH[property], prec);

    it.value() = data;

    emit decimalsChanged(property, data.decimals);
}

/*!
    \reimp
*/
void QtRectFPropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtRectFPropertyManagerPrivate::Data();

    QtProperty *xProp = d_ptr->m_doublePropertyManager->addProperty();
    xProp->setPropertyName(tr("X"));
    d_ptr->m_doublePropertyManager->setDecimals(xProp, decimals(property));
    d_ptr->m_doublePropertyManager->setValue(xProp, 0);
    d_ptr->m_propertyToX[property] = xProp;
    d_ptr->m_xToProperty[xProp] = property;
    property->addSubProperty(xProp);

    QtProperty *yProp = d_ptr->m_doublePropertyManager->addProperty();
    yProp->setPropertyName(tr("Y"));
    d_ptr->m_doublePropertyManager->setDecimals(yProp, decimals(property));
    d_ptr->m_doublePropertyManager->setValue(yProp, 0);
    d_ptr->m_propertyToY[property] = yProp;
    d_ptr->m_yToProperty[yProp] = property;
    property->addSubProperty(yProp);

    QtProperty *wProp = d_ptr->m_doublePropertyManager->addProperty();
    wProp->setPropertyName(tr("Width"));
    d_ptr->m_doublePropertyManager->setDecimals(wProp, decimals(property));
    d_ptr->m_doublePropertyManager->setValue(wProp, 0);
    d_ptr->m_doublePropertyManager->setMinimum(wProp, 0);
    d_ptr->m_propertyToW[property] = wProp;
    d_ptr->m_wToProperty[wProp] = property;
    property->addSubProperty(wProp);

    QtProperty *hProp = d_ptr->m_doublePropertyManager->addProperty();
    hProp->setPropertyName(tr("Height"));
    d_ptr->m_doublePropertyManager->setDecimals(hProp, decimals(property));
    d_ptr->m_doublePropertyManager->setValue(hProp, 0);
    d_ptr->m_doublePropertyManager->setMinimum(hProp, 0);
    d_ptr->m_propertyToH[property] = hProp;
    d_ptr->m_hToProperty[hProp] = property;
    property->addSubProperty(hProp);
}

/*!
    \reimp
*/
void QtRectFPropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *xProp = d_ptr->m_propertyToX[property];
    if (xProp) {
        d_ptr->m_xToProperty.remove(xProp);
        delete xProp;
    }
    d_ptr->m_propertyToX.remove(property);

    QtProperty *yProp = d_ptr->m_propertyToY[property];
    if (yProp) {
        d_ptr->m_yToProperty.remove(yProp);
        delete yProp;
    }
    d_ptr->m_propertyToY.remove(property);

    QtProperty *wProp = d_ptr->m_propertyToW[property];
    if (wProp) {
        d_ptr->m_wToProperty.remove(wProp);
        delete wProp;
    }
    d_ptr->m_propertyToW.remove(property);

    QtProperty *hProp = d_ptr->m_propertyToH[property];
    if (hProp) {
        d_ptr->m_hToProperty.remove(hProp);
        delete hProp;
    }
    d_ptr->m_propertyToH.remove(property);

    d_ptr->m_values.remove(property);
}

// QtEnumPropertyManager

class QtEnumPropertyManagerPrivate
{
    QtEnumPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtEnumPropertyManager)
public:

    struct Data
    {
        Data() : val(-1) {}
        int val;
        QStringList enumNames;
        QMap<int, QIcon> enumIcons;
    };

    typedef QMap<const QtProperty *, Data> PropertyValueMap;
    PropertyValueMap m_values;
};

/*!
    \class QtEnumPropertyManager

    \brief The QtEnumPropertyManager provides and manages enum properties.

    Each enum property has an associated list of enum names which can
    be retrieved using the enumNames() function, and set using the
    corresponding setEnumNames() function. An enum property's value is
    represented by an index in this list, and can be retrieved and set
    using the value() and setValue() slots respectively.

    Each enum value can also have an associated icon. The mapping from
    values to icons can be set using the setEnumIcons() function and
    queried with the enumIcons() function.

    In addition, QtEnumPropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes. The enumNamesChanged() or enumIconsChanged() signal is emitted
    whenever the list of enum names or icons is altered.

    \sa QtAbstractPropertyManager, QtEnumEditorFactory
*/

/*!
    \fn void QtEnumPropertyManager::valueChanged(QtProperty *property, int value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.

    \sa setValue()
*/

/*!
    \fn void QtEnumPropertyManager::enumNamesChanged(QtProperty *property, const QStringList &names)

    This signal is emitted whenever a property created by this manager
    changes its enum names, passing a pointer to the \a property and
    the new \a names as parameters.

    \sa setEnumNames()
*/

/*!
    \fn void QtEnumPropertyManager::enumIconsChanged(QtProperty *property, const QMap<int, QIcon> &icons)

    This signal is emitted whenever a property created by this manager
    changes its enum icons, passing a pointer to the \a property and
    the new mapping of values to \a icons as parameters.

    \sa setEnumIcons()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtEnumPropertyManager::QtEnumPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtEnumPropertyManagerPrivate;
    d_ptr->q_ptr = this;
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtEnumPropertyManager::~QtEnumPropertyManager()
{
    clear();
    delete d_ptr;
}

/*!
    Returns the given \a property's value which is an index in the
    list returned by enumNames()

    If the given property is not managed by this manager, this
    function returns -1.

    \sa enumNames(), setValue()
*/
int QtEnumPropertyManager::value(const QtProperty *property) const
{
    return getValue<int>(d_ptr->m_values, property, -1);
}

/*!
    Returns the given \a property's list of enum names.

    \sa value(), setEnumNames()
*/
QStringList QtEnumPropertyManager::enumNames(const QtProperty *property) const
{
    return getData<QStringList>(d_ptr->m_values, &QtEnumPropertyManagerPrivate::Data::enumNames, property, QStringList());
}

/*!
    Returns the given \a property's map of enum values to their icons.

    \sa value(), setEnumIcons()
*/
QMap<int, QIcon> QtEnumPropertyManager::enumIcons(const QtProperty *property) const
{
    return getData<QMap<int, QIcon> >(d_ptr->m_values, &QtEnumPropertyManagerPrivate::Data::enumIcons, property, QMap<int, QIcon>());
}

/*!
    \reimp
*/
QString QtEnumPropertyManager::valueText(const QtProperty *property) const
{
    const QtEnumPropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QString();

    const QtEnumPropertyManagerPrivate::Data &data = it.value();

    const int v = data.val;
    if (v >= 0 && v < data.enumNames.count())
        return data.enumNames.at(v);
    return QString();
}

/*!
    \reimp
*/
QIcon QtEnumPropertyManager::valueIcon(const QtProperty *property) const
{
    const QtEnumPropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QIcon();

    const QtEnumPropertyManagerPrivate::Data &data = it.value();

    const int v = data.val;
    return data.enumIcons.value(v);
}

/*!
    \fn void QtEnumPropertyManager::setValue(QtProperty *property, int value)

    Sets the value of the given  \a property to \a value.

    The specified \a value must be less than the size of the given \a
    property's enumNames() list, and larger than (or equal to) 0.

    \sa value(), valueChanged()
*/
void QtEnumPropertyManager::setValue(QtProperty *property, int val)
{
    const QtEnumPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    QtEnumPropertyManagerPrivate::Data data = it.value();

    if (val >= data.enumNames.count())
        return;

    if (val < 0 && data.enumNames.count() > 0)
        return;

    if (val < 0)
        val = -1;

    if (data.val == val)
        return;

    data.val = val;

    it.value() = data;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    Sets the given \a property's list of enum names to \a
    enumNames. The \a property's current value is reset to 0
    indicating the first item of the list.

    If the specified \a enumNames list is empty, the \a property's
    current value is set to -1.

    \sa enumNames(), enumNamesChanged()
*/
void QtEnumPropertyManager::setEnumNames(QtProperty *property, const QStringList &enumNames)
{
    const QtEnumPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    QtEnumPropertyManagerPrivate::Data data = it.value();

    if (data.enumNames == enumNames)
        return;

    data.enumNames = enumNames;

    data.val = -1;

    if (enumNames.count() > 0)
        data.val = 0;

    it.value() = data;

    emit enumNamesChanged(property, data.enumNames);

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    Sets the given \a property's map of enum values to their icons to \a
    enumIcons.

    Each enum value can have associated icon. This association is represented with passed \a enumIcons map.

    \sa enumNames(), enumNamesChanged()
*/
void QtEnumPropertyManager::setEnumIcons(QtProperty *property, const QMap<int, QIcon> &enumIcons)
{
    const QtEnumPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    it.value().enumIcons = enumIcons;

    emit enumIconsChanged(property, it.value().enumIcons);

    emit propertyChanged(property);
}

/*!
    \reimp
*/
void QtEnumPropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtEnumPropertyManagerPrivate::Data();
}

/*!
    \reimp
*/
void QtEnumPropertyManager::uninitializeProperty(QtProperty *property)
{
    d_ptr->m_values.remove(property);
}

// QtFlagPropertyManager

class QtFlagPropertyManagerPrivate
{
    QtFlagPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtFlagPropertyManager)
public:

    void slotBoolChanged(QtProperty *property, bool value);
    void slotPropertyDestroyed(QtProperty *property);

    struct Data
    {
        Data() : val(-1) {}
        int val;
        QStringList flagNames;
    };

    typedef QMap<const QtProperty *, Data> PropertyValueMap;
    PropertyValueMap m_values;

    QtBoolPropertyManager *m_boolPropertyManager;

    QMap<const QtProperty *, QList<QtProperty *> > m_propertyToFlags;

    QMap<const QtProperty *, QtProperty *> m_flagToProperty;
};

void QtFlagPropertyManagerPrivate::slotBoolChanged(QtProperty *property, bool value)
{
    QtProperty *prop = m_flagToProperty.value(property, nullptr);
    if (prop == nullptr)
        return;

    QListIterator<QtProperty *> itProp(m_propertyToFlags[prop]);
    int level = 0;
    while (itProp.hasNext()) {
        QtProperty *p = itProp.next();
        if (p == property) {
            int v = m_values[prop].val;
            if (value) {
                v |= (1 << level);
            } else {
                v &= ~(1 << level);
            }
            q_ptr->setValue(prop, v);
            return;
        }
        level++;
    }
}

void QtFlagPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    QtProperty *flagProperty = m_flagToProperty.value(property, nullptr);
    if (flagProperty == nullptr)
        return;

    m_propertyToFlags[flagProperty].replace(m_propertyToFlags[flagProperty].indexOf(property), nullptr);
    m_flagToProperty.remove(property);
}

/*!
    \class QtFlagPropertyManager

    \brief The QtFlagPropertyManager provides and manages flag properties.

    Each flag property has an associated list of flag names which can
    be retrieved using the flagNames() function, and set using the
    corresponding setFlagNames() function.

    The flag manager provides properties with nested boolean
    subproperties representing each flag, i.e. a flag property's value
    is the binary combination of the subproperties' values. A
    property's value can be retrieved and set using the value() and
    setValue() slots respectively. The combination of flags is represented
    by single int value - that's why it's possible to store up to
    32 independent flags in one flag property.

    The subproperties are created by a QtBoolPropertyManager object. This
    manager can be retrieved using the subBoolPropertyManager() function. In
    order to provide editing widgets for the subproperties in a
    property browser widget, this manager must be associated with an
    editor factory.

    In addition, QtFlagPropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes, and the flagNamesChanged() signal which is emitted
    whenever the list of flag names is altered.

    \sa QtAbstractPropertyManager, QtBoolPropertyManager
*/

/*!
    \fn void QtFlagPropertyManager::valueChanged(QtProperty *property, int value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a  property and the new
    \a value as parameters.

    \sa setValue()
*/

/*!
    \fn void QtFlagPropertyManager::flagNamesChanged(QtProperty *property, const QStringList &names)

    This signal is emitted whenever a property created by this manager
    changes its flag names, passing a pointer to the \a property and the
    new \a names as parameters.

    \sa setFlagNames()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtFlagPropertyManager::QtFlagPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtFlagPropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_boolPropertyManager = new QtBoolPropertyManager(this);
    connect(d_ptr->m_boolPropertyManager, SIGNAL(valueChanged(QtProperty *, bool)),
                this, SLOT(slotBoolChanged(QtProperty *, bool)));
    connect(d_ptr->m_boolPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtFlagPropertyManager::~QtFlagPropertyManager()
{
    clear();
    delete d_ptr;
}

/*!
    Returns the manager that produces the nested boolean subproperties
    representing each flag.

    In order to provide editing widgets for the subproperties in a
    property browser widget, this manager must be associated with an
    editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtBoolPropertyManager *QtFlagPropertyManager::subBoolPropertyManager() const
{
    return d_ptr->m_boolPropertyManager;
}

/*!
    Returns the given \a property's value.

    If the given property is not managed by this manager, this
    function returns 0.

    \sa flagNames(), setValue()
*/
int QtFlagPropertyManager::value(const QtProperty *property) const
{
    return getValue<int>(d_ptr->m_values, property, 0);
}

/*!
    Returns the given \a property's list of flag names.

    \sa value(), setFlagNames()
*/
QStringList QtFlagPropertyManager::flagNames(const QtProperty *property) const
{
    return getData<QStringList>(d_ptr->m_values, &QtFlagPropertyManagerPrivate::Data::flagNames, property, QStringList());
}

/*!
    \reimp
*/
QString QtFlagPropertyManager::valueText(const QtProperty *property) const
{
    const QtFlagPropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QString();

    const QtFlagPropertyManagerPrivate::Data &data = it.value();

    QString str;
    int level = 0;
    const QChar bar = QLatin1Char('|');
    const QStringList::const_iterator fncend = data.flagNames.constEnd();
    for (QStringList::const_iterator it =  data.flagNames.constBegin(); it != fncend; ++it) {
        if (data.val & (1 << level)) {
            if (!str.isEmpty())
                str += bar;
            str += *it;
        }

        level++;
    }
    return str;
}

/*!
    \fn void QtFlagPropertyManager::setValue(QtProperty *property, int value)

    Sets the value of the given \a property to \a value. Nested
    properties are updated automatically.

    The specified \a value must be less than the binary combination of
    the property's flagNames() list size (i.e. less than 2\sup n,
    where \c n is the size of the list) and larger than (or equal to)
    0.

    \sa value(), valueChanged()
*/
void QtFlagPropertyManager::setValue(QtProperty *property, int val)
{
    const QtFlagPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    QtFlagPropertyManagerPrivate::Data data = it.value();

    if (data.val == val)
        return;

    if (val > (1 << data.flagNames.count()) - 1)
        return;

    if (val < 0)
        return;

    data.val = val;

    it.value() = data;

    QListIterator<QtProperty *> itProp(d_ptr->m_propertyToFlags[property]);
    int level = 0;
    while (itProp.hasNext()) {
        QtProperty *prop = itProp.next();
        if (prop)
            d_ptr->m_boolPropertyManager->setValue(prop, val & (1 << level));
        level++;
    }

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    Sets the given \a property's list of flag names to \a flagNames. The
    property's current value is reset to 0 indicating the first item
    of the list.

    \sa flagNames(), flagNamesChanged()
*/
void QtFlagPropertyManager::setFlagNames(QtProperty *property, const QStringList &flagNames)
{
    const QtFlagPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    QtFlagPropertyManagerPrivate::Data data = it.value();

    if (data.flagNames == flagNames)
        return;

    data.flagNames = flagNames;
    data.val = 0;

    it.value() = data;

    QListIterator<QtProperty *> itProp(d_ptr->m_propertyToFlags[property]);
    while (itProp.hasNext()) {
        QtProperty *prop = itProp.next();
        if (prop) {
            delete prop;
            d_ptr->m_flagToProperty.remove(prop);
        }
    }
    d_ptr->m_propertyToFlags[property].clear();

    QStringListIterator itFlag(flagNames);
    while (itFlag.hasNext()) {
        const QString flagName = itFlag.next();
        QtProperty *prop = d_ptr->m_boolPropertyManager->addProperty();
        prop->setPropertyName(flagName);
        property->addSubProperty(prop);
        d_ptr->m_propertyToFlags[property].append(prop);
        d_ptr->m_flagToProperty[prop] = property;
    }

    emit flagNamesChanged(property, data.flagNames);

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    \reimp
*/
void QtFlagPropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtFlagPropertyManagerPrivate::Data();

    d_ptr->m_propertyToFlags[property] = QList<QtProperty *>();
}

/*!
    \reimp
*/
void QtFlagPropertyManager::uninitializeProperty(QtProperty *property)
{
    QListIterator<QtProperty *> itProp(d_ptr->m_propertyToFlags[property]);
    while (itProp.hasNext()) {
        QtProperty *prop = itProp.next();
        if (prop) {
            delete prop;
            d_ptr->m_flagToProperty.remove(prop);
        }
    }
    d_ptr->m_propertyToFlags.remove(property);

    d_ptr->m_values.remove(property);
}

// QtSizePolicyPropertyManager

class QtSizePolicyPropertyManagerPrivate
{
    QtSizePolicyPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtSizePolicyPropertyManager)
public:

    QtSizePolicyPropertyManagerPrivate();

    void slotIntChanged(QtProperty *property, int value);
    void slotEnumChanged(QtProperty *property, int value);
    void slotPropertyDestroyed(QtProperty *property);

    typedef QMap<const QtProperty *, QSizePolicy> PropertyValueMap;
    PropertyValueMap m_values;

    QtIntPropertyManager *m_intPropertyManager;
    QtEnumPropertyManager *m_enumPropertyManager;

    QMap<const QtProperty *, QtProperty *> m_propertyToHPolicy;
    QMap<const QtProperty *, QtProperty *> m_propertyToVPolicy;
    QMap<const QtProperty *, QtProperty *> m_propertyToHStretch;
    QMap<const QtProperty *, QtProperty *> m_propertyToVStretch;

    QMap<const QtProperty *, QtProperty *> m_hPolicyToProperty;
    QMap<const QtProperty *, QtProperty *> m_vPolicyToProperty;
    QMap<const QtProperty *, QtProperty *> m_hStretchToProperty;
    QMap<const QtProperty *, QtProperty *> m_vStretchToProperty;
};

QtSizePolicyPropertyManagerPrivate::QtSizePolicyPropertyManagerPrivate()
{
}

void QtSizePolicyPropertyManagerPrivate::slotIntChanged(QtProperty *property, int value)
{
    if (QtProperty *prop = m_hStretchToProperty.value(property, nullptr)) {
        QSizePolicy sp = m_values[prop];
        sp.setHorizontalStretch(value);
        q_ptr->setValue(prop, sp);
    } else if (QtProperty *prop = m_vStretchToProperty.value(property, nullptr)) {
        QSizePolicy sp = m_values[prop];
        sp.setVerticalStretch(value);
        q_ptr->setValue(prop, sp);
    }
}

void QtSizePolicyPropertyManagerPrivate::slotEnumChanged(QtProperty *property, int value)
{
    if (QtProperty *prop = m_hPolicyToProperty.value(property, nullptr)) {
        QSizePolicy sp = m_values[prop];
        sp.setHorizontalPolicy(metaEnumProvider()->indexToSizePolicy(value));
        q_ptr->setValue(prop, sp);
    } else if (QtProperty *prop = m_vPolicyToProperty.value(property, nullptr)) {
        QSizePolicy sp = m_values[prop];
        sp.setVerticalPolicy(metaEnumProvider()->indexToSizePolicy(value));
        q_ptr->setValue(prop, sp);
    }
}

void QtSizePolicyPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (QtProperty *pointProp = m_hStretchToProperty.value(property, nullptr)) {
        m_propertyToHStretch[pointProp] = nullptr;
        m_hStretchToProperty.remove(property);
    } else if (QtProperty *pointProp = m_vStretchToProperty.value(property, nullptr)) {
        m_propertyToVStretch[pointProp] = nullptr;
        m_vStretchToProperty.remove(property);
    } else if (QtProperty *pointProp = m_hPolicyToProperty.value(property, nullptr)) {
        m_propertyToHPolicy[pointProp] = nullptr;
        m_hPolicyToProperty.remove(property);
    } else if (QtProperty *pointProp = m_vPolicyToProperty.value(property, nullptr)) {
        m_propertyToVPolicy[pointProp] = nullptr;
        m_vPolicyToProperty.remove(property);
    }
}

/*!
    \class QtSizePolicyPropertyManager

    \brief The QtSizePolicyPropertyManager provides and manages QSizePolicy properties.

    A size policy property has nested \e horizontalPolicy, \e
    verticalPolicy, \e horizontalStretch and \e verticalStretch
    subproperties. The top-level property's value can be retrieved
    using the value() function, and set using the setValue() slot.

    The subproperties are created by QtIntPropertyManager and QtEnumPropertyManager
    objects. These managers can be retrieved using the subIntPropertyManager()
    and subEnumPropertyManager() functions respectively. In order to provide
    editing widgets for the subproperties in a property browser widget,
    these managers must be associated with editor factories.

    In addition, QtSizePolicyPropertyManager provides the valueChanged()
    signal which is emitted whenever a property created by this
    manager changes.

    \sa QtAbstractPropertyManager, QtIntPropertyManager, QtEnumPropertyManager
*/

/*!
    \fn void QtSizePolicyPropertyManager::valueChanged(QtProperty *property, const QSizePolicy &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the
    new \a value as parameters.

    \sa setValue()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtSizePolicyPropertyManager::QtSizePolicyPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtSizePolicyPropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_intPropertyManager = new QtIntPropertyManager(this);
    connect(d_ptr->m_intPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotIntChanged(QtProperty *, int)));
    d_ptr->m_enumPropertyManager = new QtEnumPropertyManager(this);
    connect(d_ptr->m_enumPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotEnumChanged(QtProperty *, int)));

    connect(d_ptr->m_intPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
    connect(d_ptr->m_enumPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtSizePolicyPropertyManager::~QtSizePolicyPropertyManager()
{
    clear();
    delete d_ptr;
}

/*!
    Returns the manager that creates the nested \e horizontalStretch
    and \e verticalStretch subproperties.

    In order to provide editing widgets for the mentioned subproperties
    in a property browser widget, this manager must be associated with
    an editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtIntPropertyManager *QtSizePolicyPropertyManager::subIntPropertyManager() const
{
    return d_ptr->m_intPropertyManager;
}

/*!
    Returns the manager that creates the nested \e horizontalPolicy
    and \e verticalPolicy subproperties.

    In order to provide editing widgets for the mentioned subproperties
    in a property browser widget, this manager must be associated with
    an editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtEnumPropertyManager *QtSizePolicyPropertyManager::subEnumPropertyManager() const
{
    return d_ptr->m_enumPropertyManager;
}

/*!
    Returns the given \a property's value.

    If the given property is not managed by this manager, this
    function returns the default size policy.

    \sa setValue()
*/
QSizePolicy QtSizePolicyPropertyManager::value(const QtProperty *property) const
{
    return d_ptr->m_values.value(property, QSizePolicy());
}

/*!
    \reimp
*/
QString QtSizePolicyPropertyManager::valueText(const QtProperty *property) const
{
    const QtSizePolicyPropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QString();

    const QSizePolicy sp = it.value();
    const QtMetaEnumProvider *mep = metaEnumProvider();
    const int hIndex = mep->sizePolicyToIndex(sp.horizontalPolicy());
    const int vIndex = mep->sizePolicyToIndex(sp.verticalPolicy());
    //! Unknown size policy on reading invalid uic3 files
    const QString hPolicy = hIndex != -1 ? mep->policyEnumNames().at(hIndex) : tr("<Invalid>");
    const QString vPolicy = vIndex != -1 ? mep->policyEnumNames().at(vIndex) : tr("<Invalid>");
    const QString str = tr("[%1, %2, %3, %4]").arg(hPolicy, vPolicy).arg(sp.horizontalStretch()).arg(sp.verticalStretch());
    return str;
}

/*!
    \fn void QtSizePolicyPropertyManager::setValue(QtProperty *property, const QSizePolicy &value)

    Sets the value of the given \a property to \a value. Nested
    properties are updated automatically.

    \sa value(), valueChanged()
*/
void QtSizePolicyPropertyManager::setValue(QtProperty *property, const QSizePolicy &val)
{
    const QtSizePolicyPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    if (it.value() == val)
        return;

    it.value() = val;

    d_ptr->m_enumPropertyManager->setValue(d_ptr->m_propertyToHPolicy[property],
                metaEnumProvider()->sizePolicyToIndex(val.horizontalPolicy()));
    d_ptr->m_enumPropertyManager->setValue(d_ptr->m_propertyToVPolicy[property],
                metaEnumProvider()->sizePolicyToIndex(val.verticalPolicy()));
    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToHStretch[property],
                val.horizontalStretch());
    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToVStretch[property],
                val.verticalStretch());

    emit propertyChanged(property);
    emit valueChanged(property, val);
}

/*!
    \reimp
*/
void QtSizePolicyPropertyManager::initializeProperty(QtProperty *property)
{
    QSizePolicy val;
    d_ptr->m_values[property] = val;

    QtProperty *hPolicyProp = d_ptr->m_enumPropertyManager->addProperty();
    hPolicyProp->setPropertyName(tr("Horizontal Policy"));
    d_ptr->m_enumPropertyManager->setEnumNames(hPolicyProp, metaEnumProvider()->policyEnumNames());
    d_ptr->m_enumPropertyManager->setValue(hPolicyProp,
                metaEnumProvider()->sizePolicyToIndex(val.horizontalPolicy()));
    d_ptr->m_propertyToHPolicy[property] = hPolicyProp;
    d_ptr->m_hPolicyToProperty[hPolicyProp] = property;
    property->addSubProperty(hPolicyProp);

    QtProperty *vPolicyProp = d_ptr->m_enumPropertyManager->addProperty();
    vPolicyProp->setPropertyName(tr("Vertical Policy"));
    d_ptr->m_enumPropertyManager->setEnumNames(vPolicyProp, metaEnumProvider()->policyEnumNames());
    d_ptr->m_enumPropertyManager->setValue(vPolicyProp,
                metaEnumProvider()->sizePolicyToIndex(val.verticalPolicy()));
    d_ptr->m_propertyToVPolicy[property] = vPolicyProp;
    d_ptr->m_vPolicyToProperty[vPolicyProp] = property;
    property->addSubProperty(vPolicyProp);

    QtProperty *hStretchProp = d_ptr->m_intPropertyManager->addProperty();
    hStretchProp->setPropertyName(tr("Horizontal Stretch"));
    d_ptr->m_intPropertyManager->setValue(hStretchProp, val.horizontalStretch());
    d_ptr->m_intPropertyManager->setRange(hStretchProp, 0, 0xff);
    d_ptr->m_propertyToHStretch[property] = hStretchProp;
    d_ptr->m_hStretchToProperty[hStretchProp] = property;
    property->addSubProperty(hStretchProp);

    QtProperty *vStretchProp = d_ptr->m_intPropertyManager->addProperty();
    vStretchProp->setPropertyName(tr("Vertical Stretch"));
    d_ptr->m_intPropertyManager->setValue(vStretchProp, val.verticalStretch());
    d_ptr->m_intPropertyManager->setRange(vStretchProp, 0, 0xff);
    d_ptr->m_propertyToVStretch[property] = vStretchProp;
    d_ptr->m_vStretchToProperty[vStretchProp] = property;
    property->addSubProperty(vStretchProp);

}

/*!
    \reimp
*/
void QtSizePolicyPropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *hPolicyProp = d_ptr->m_propertyToHPolicy[property];
    if (hPolicyProp) {
        d_ptr->m_hPolicyToProperty.remove(hPolicyProp);
        delete hPolicyProp;
    }
    d_ptr->m_propertyToHPolicy.remove(property);

    QtProperty *vPolicyProp = d_ptr->m_propertyToVPolicy[property];
    if (vPolicyProp) {
        d_ptr->m_vPolicyToProperty.remove(vPolicyProp);
        delete vPolicyProp;
    }
    d_ptr->m_propertyToVPolicy.remove(property);

    QtProperty *hStretchProp = d_ptr->m_propertyToHStretch[property];
    if (hStretchProp) {
        d_ptr->m_hStretchToProperty.remove(hStretchProp);
        delete hStretchProp;
    }
    d_ptr->m_propertyToHStretch.remove(property);

    QtProperty *vStretchProp = d_ptr->m_propertyToVStretch[property];
    if (vStretchProp) {
        d_ptr->m_vStretchToProperty.remove(vStretchProp);
        delete vStretchProp;
    }
    d_ptr->m_propertyToVStretch.remove(property);

    d_ptr->m_values.remove(property);
}

// QtFontPropertyManager:
// QtFontPropertyManagerPrivate has a mechanism for reacting
// to QApplication::fontDatabaseChanged() [4.5], which is emitted
// when someone loads an application font. The signals are compressed
// using a timer with interval 0, which then causes the family
// enumeration manager to re-set its strings and index values
// for each property.

Q_GLOBAL_STATIC(QFontDatabase, fontDatabase)

class QtFontPropertyManagerPrivate
{
    QtFontPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtFontPropertyManager)
public:

    QtFontPropertyManagerPrivate();

    void slotIntChanged(QtProperty *property, int value);
    void slotEnumChanged(QtProperty *property, int value);
    void slotBoolChanged(QtProperty *property, bool value);
    void slotPropertyDestroyed(QtProperty *property);
    void slotFontDatabaseChanged();
    void slotFontDatabaseDelayedChange();

    QStringList m_familyNames;

    typedef QMap<const QtProperty *, QFont> PropertyValueMap;
    PropertyValueMap m_values;

    QtIntPropertyManager *m_intPropertyManager;
    QtEnumPropertyManager *m_enumPropertyManager;
    QtBoolPropertyManager *m_boolPropertyManager;

    QMap<const QtProperty *, QtProperty *> m_propertyToFamily;
    QMap<const QtProperty *, QtProperty *> m_propertyToPointSize;
    QMap<const QtProperty *, QtProperty *> m_propertyToBold;
    QMap<const QtProperty *, QtProperty *> m_propertyToItalic;
    QMap<const QtProperty *, QtProperty *> m_propertyToUnderline;
    QMap<const QtProperty *, QtProperty *> m_propertyToStrikeOut;
    QMap<const QtProperty *, QtProperty *> m_propertyToKerning;

    QMap<const QtProperty *, QtProperty *> m_familyToProperty;
    QMap<const QtProperty *, QtProperty *> m_pointSizeToProperty;
    QMap<const QtProperty *, QtProperty *> m_boldToProperty;
    QMap<const QtProperty *, QtProperty *> m_italicToProperty;
    QMap<const QtProperty *, QtProperty *> m_underlineToProperty;
    QMap<const QtProperty *, QtProperty *> m_strikeOutToProperty;
    QMap<const QtProperty *, QtProperty *> m_kerningToProperty;

    bool m_settingValue;
    QTimer *m_fontDatabaseChangeTimer;
};

QtFontPropertyManagerPrivate::QtFontPropertyManagerPrivate() :
    m_settingValue(false),
    m_fontDatabaseChangeTimer(nullptr)
{
}

void QtFontPropertyManagerPrivate::slotIntChanged(QtProperty *property, int value)
{
    if (m_settingValue)
        return;
    if (QtProperty *prop = m_pointSizeToProperty.value(property, nullptr)) {
        QFont f = m_values[prop];
        f.setPointSize(value);
        q_ptr->setValue(prop, f);
    }
}

void QtFontPropertyManagerPrivate::slotEnumChanged(QtProperty *property, int value)
{
    if (m_settingValue)
        return;
    if (QtProperty *prop = m_familyToProperty.value(property, nullptr)) {
        QFont f = m_values[prop];
        f.setFamily(m_familyNames.at(value));
        q_ptr->setValue(prop, f);
    }
}

void QtFontPropertyManagerPrivate::slotBoolChanged(QtProperty *property, bool value)
{
    if (m_settingValue)
        return;
    if (QtProperty *prop = m_boldToProperty.value(property, nullptr)) {
        QFont f = m_values[prop];
        f.setBold(value);
        q_ptr->setValue(prop, f);
    } else if (QtProperty *prop = m_italicToProperty.value(property, nullptr)) {
        QFont f = m_values[prop];
        f.setItalic(value);
        q_ptr->setValue(prop, f);
    } else if (QtProperty *prop = m_underlineToProperty.value(property, nullptr)) {
        QFont f = m_values[prop];
        f.setUnderline(value);
        q_ptr->setValue(prop, f);
    } else if (QtProperty *prop = m_strikeOutToProperty.value(property, nullptr)) {
        QFont f = m_values[prop];
        f.setStrikeOut(value);
        q_ptr->setValue(prop, f);
    } else if (QtProperty *prop = m_kerningToProperty.value(property, nullptr)) {
        QFont f = m_values[prop];
        f.setKerning(value);
        q_ptr->setValue(prop, f);
    }
}

void QtFontPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (QtProperty *pointProp = m_pointSizeToProperty.value(property, nullptr)) {
        m_propertyToPointSize[pointProp] = nullptr;
        m_pointSizeToProperty.remove(property);
    } else if (QtProperty *pointProp = m_familyToProperty.value(property, nullptr)) {
        m_propertyToFamily[pointProp] = nullptr;
        m_familyToProperty.remove(property);
    } else if (QtProperty *pointProp = m_boldToProperty.value(property, nullptr)) {
        m_propertyToBold[pointProp] = nullptr;
        m_boldToProperty.remove(property);
    } else if (QtProperty *pointProp = m_italicToProperty.value(property, nullptr)) {
        m_propertyToItalic[pointProp] = nullptr;
        m_italicToProperty.remove(property);
    } else if (QtProperty *pointProp = m_underlineToProperty.value(property, nullptr)) {
        m_propertyToUnderline[pointProp] = nullptr;
        m_underlineToProperty.remove(property);
    } else if (QtProperty *pointProp = m_strikeOutToProperty.value(property, nullptr)) {
        m_propertyToStrikeOut[pointProp] = nullptr;
        m_strikeOutToProperty.remove(property);
    } else if (QtProperty *pointProp = m_kerningToProperty.value(property, nullptr)) {
        m_propertyToKerning[pointProp] = nullptr;
        m_kerningToProperty.remove(property);
    }
}

void  QtFontPropertyManagerPrivate::slotFontDatabaseChanged()
{
    if (!m_fontDatabaseChangeTimer) {
        m_fontDatabaseChangeTimer = new QTimer(q_ptr);
        m_fontDatabaseChangeTimer->setInterval(0);
        m_fontDatabaseChangeTimer->setSingleShot(true);
        QObject::connect(m_fontDatabaseChangeTimer, SIGNAL(timeout()), q_ptr, SLOT(slotFontDatabaseDelayedChange()));
    }
    if (!m_fontDatabaseChangeTimer->isActive())
        m_fontDatabaseChangeTimer->start();
}

void QtFontPropertyManagerPrivate::slotFontDatabaseDelayedChange()
{
    typedef QMap<const QtProperty *, QtProperty *> PropertyPropertyMap;
    // rescan available font names
    const QStringList oldFamilies = m_familyNames;
    m_familyNames = fontDatabase()->families();

    // Adapt all existing properties
    if (!m_propertyToFamily.empty()) {
        PropertyPropertyMap::const_iterator cend = m_propertyToFamily.constEnd();
        for (PropertyPropertyMap::const_iterator it = m_propertyToFamily.constBegin(); it != cend; ++it) {
            QtProperty *familyProp = it.value();
            const int oldIdx = m_enumPropertyManager->value(familyProp);
            int newIdx = m_familyNames.indexOf(oldFamilies.at(oldIdx));
            if (newIdx < 0)
                newIdx = 0;
            m_enumPropertyManager->setEnumNames(familyProp, m_familyNames);
            m_enumPropertyManager->setValue(familyProp, newIdx);
        }
    }
}

/*!
    \class QtFontPropertyManager

    \brief The QtFontPropertyManager provides and manages QFont properties.

    A font property has nested \e family, \e pointSize, \e bold, \e
    italic, \e underline, \e strikeOut and \e kerning subproperties. The top-level
    property's value can be retrieved using the value() function, and
    set using the setValue() slot.

    The subproperties are created by QtIntPropertyManager, QtEnumPropertyManager and
    QtBoolPropertyManager objects. These managers can be retrieved using the
    corresponding subIntPropertyManager(), subEnumPropertyManager() and
    subBoolPropertyManager() functions. In order to provide editing widgets
    for the subproperties in a property browser widget, these managers
    must be associated with editor factories.

    In addition, QtFontPropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes.

    \sa QtAbstractPropertyManager, QtEnumPropertyManager, QtIntPropertyManager, QtBoolPropertyManager
*/

/*!
    \fn void QtFontPropertyManager::valueChanged(QtProperty *property, const QFont &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the
    new \a value as parameters.

    \sa setValue()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtFontPropertyManager::QtFontPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtFontPropertyManagerPrivate;
    d_ptr->q_ptr = this;
#if QT_VERSION >= 0x040500
    QObject::connect(qApp, SIGNAL(fontDatabaseChanged()), this, SLOT(slotFontDatabaseChanged()));
#endif

    d_ptr->m_intPropertyManager = new QtIntPropertyManager(this);
    connect(d_ptr->m_intPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotIntChanged(QtProperty *, int)));
    d_ptr->m_enumPropertyManager = new QtEnumPropertyManager(this);
    connect(d_ptr->m_enumPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotEnumChanged(QtProperty *, int)));
    d_ptr->m_boolPropertyManager = new QtBoolPropertyManager(this);
    connect(d_ptr->m_boolPropertyManager, SIGNAL(valueChanged(QtProperty *, bool)),
                this, SLOT(slotBoolChanged(QtProperty *, bool)));

    connect(d_ptr->m_intPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
    connect(d_ptr->m_enumPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
    connect(d_ptr->m_boolPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtFontPropertyManager::~QtFontPropertyManager()
{
    clear();
    delete d_ptr;
}

/*!
    Returns the manager that creates the \e pointSize subproperty.

    In order to provide editing widgets for the \e pointSize property
    in a property browser widget, this manager must be associated
    with an editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtIntPropertyManager *QtFontPropertyManager::subIntPropertyManager() const
{
    return d_ptr->m_intPropertyManager;
}

/*!
    Returns the manager that create the \e family subproperty.

    In order to provide editing widgets for the \e family property
    in a property browser widget, this manager must be associated
    with an editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtEnumPropertyManager *QtFontPropertyManager::subEnumPropertyManager() const
{
    return d_ptr->m_enumPropertyManager;
}

/*!
    Returns the manager that creates the  \e bold, \e italic, \e underline,
    \e strikeOut and \e kerning subproperties.

    In order to provide editing widgets for the mentioned properties
    in a property browser widget, this manager must be associated with
    an editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtBoolPropertyManager *QtFontPropertyManager::subBoolPropertyManager() const
{
    return d_ptr->m_boolPropertyManager;
}

/*!
    Returns the given \a property's value.

    If the given property is not managed by this manager, this
    function returns a font object that uses the application's default
    font.

    \sa setValue()
*/
QFont QtFontPropertyManager::value(const QtProperty *property) const
{
    return d_ptr->m_values.value(property, QFont());
}

/*!
    \reimp
*/
QString QtFontPropertyManager::valueText(const QtProperty *property) const
{
    const QtFontPropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QString();

    return QtPropertyBrowserUtils::fontValueText(it.value());
}

/*!
    \reimp
*/
QIcon QtFontPropertyManager::valueIcon(const QtProperty *property) const
{
    const QtFontPropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QIcon();

    return QtPropertyBrowserUtils::fontValueIcon(it.value());
}

/*!
    \fn void QtFontPropertyManager::setValue(QtProperty *property, const QFont &value)

    Sets the value of the given \a property to \a value. Nested
    properties are updated automatically.

    \sa value(), valueChanged()
*/
void QtFontPropertyManager::setValue(QtProperty *property, const QFont &val)
{
    const QtFontPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    const QFont oldVal = it.value();
    if (oldVal == val && oldVal.resolve() == val.resolve())
        return;

    it.value() = val;

    int idx = d_ptr->m_familyNames.indexOf(val.family());
    if (idx == -1)
        idx = 0;
    bool settingValue = d_ptr->m_settingValue;
    d_ptr->m_settingValue = true;
    d_ptr->m_enumPropertyManager->setValue(d_ptr->m_propertyToFamily[property], idx);
    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToPointSize[property], val.pointSize());
    d_ptr->m_boolPropertyManager->setValue(d_ptr->m_propertyToBold[property], val.bold());
    d_ptr->m_boolPropertyManager->setValue(d_ptr->m_propertyToItalic[property], val.italic());
    d_ptr->m_boolPropertyManager->setValue(d_ptr->m_propertyToUnderline[property], val.underline());
    d_ptr->m_boolPropertyManager->setValue(d_ptr->m_propertyToStrikeOut[property], val.strikeOut());
    d_ptr->m_boolPropertyManager->setValue(d_ptr->m_propertyToKerning[property], val.kerning());
    d_ptr->m_settingValue = settingValue;

    emit propertyChanged(property);
    emit valueChanged(property, val);
}

/*!
    \reimp
*/
void QtFontPropertyManager::initializeProperty(QtProperty *property)
{
    QFont val;
    d_ptr->m_values[property] = val;

    QtProperty *familyProp = d_ptr->m_enumPropertyManager->addProperty();
    familyProp->setPropertyName(tr("Family"));
    if (d_ptr->m_familyNames.empty())
        d_ptr->m_familyNames = fontDatabase()->families();
    d_ptr->m_enumPropertyManager->setEnumNames(familyProp, d_ptr->m_familyNames);
    int idx = d_ptr->m_familyNames.indexOf(val.family());
    if (idx == -1)
        idx = 0;
    d_ptr->m_enumPropertyManager->setValue(familyProp, idx);
    d_ptr->m_propertyToFamily[property] = familyProp;
    d_ptr->m_familyToProperty[familyProp] = property;
    property->addSubProperty(familyProp);

    QtProperty *pointSizeProp = d_ptr->m_intPropertyManager->addProperty();
    pointSizeProp->setPropertyName(tr("Point Size"));
    d_ptr->m_intPropertyManager->setValue(pointSizeProp, val.pointSize());
    d_ptr->m_intPropertyManager->setMinimum(pointSizeProp, 1);
    d_ptr->m_propertyToPointSize[property] = pointSizeProp;
    d_ptr->m_pointSizeToProperty[pointSizeProp] = property;
    property->addSubProperty(pointSizeProp);

    QtProperty *boldProp = d_ptr->m_boolPropertyManager->addProperty();
    boldProp->setPropertyName(tr("Bold"));
    d_ptr->m_boolPropertyManager->setValue(boldProp, val.bold());
    d_ptr->m_propertyToBold[property] = boldProp;
    d_ptr->m_boldToProperty[boldProp] = property;
    property->addSubProperty(boldProp);

    QtProperty *italicProp = d_ptr->m_boolPropertyManager->addProperty();
    italicProp->setPropertyName(tr("Italic"));
    d_ptr->m_boolPropertyManager->setValue(italicProp, val.italic());
    d_ptr->m_propertyToItalic[property] = italicProp;
    d_ptr->m_italicToProperty[italicProp] = property;
    property->addSubProperty(italicProp);

    QtProperty *underlineProp = d_ptr->m_boolPropertyManager->addProperty();
    underlineProp->setPropertyName(tr("Underline"));
    d_ptr->m_boolPropertyManager->setValue(underlineProp, val.underline());
    d_ptr->m_propertyToUnderline[property] = underlineProp;
    d_ptr->m_underlineToProperty[underlineProp] = property;
    property->addSubProperty(underlineProp);

    QtProperty *strikeOutProp = d_ptr->m_boolPropertyManager->addProperty();
    strikeOutProp->setPropertyName(tr("Strikeout"));
    d_ptr->m_boolPropertyManager->setValue(strikeOutProp, val.strikeOut());
    d_ptr->m_propertyToStrikeOut[property] = strikeOutProp;
    d_ptr->m_strikeOutToProperty[strikeOutProp] = property;
    property->addSubProperty(strikeOutProp);

    QtProperty *kerningProp = d_ptr->m_boolPropertyManager->addProperty();
    kerningProp->setPropertyName(tr("Kerning"));
    d_ptr->m_boolPropertyManager->setValue(kerningProp, val.kerning());
    d_ptr->m_propertyToKerning[property] = kerningProp;
    d_ptr->m_kerningToProperty[kerningProp] = property;
    property->addSubProperty(kerningProp);
}

/*!
    \reimp
*/
void QtFontPropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *familyProp = d_ptr->m_propertyToFamily[property];
    if (familyProp) {
        d_ptr->m_familyToProperty.remove(familyProp);
        delete familyProp;
    }
    d_ptr->m_propertyToFamily.remove(property);

    QtProperty *pointSizeProp = d_ptr->m_propertyToPointSize[property];
    if (pointSizeProp) {
        d_ptr->m_pointSizeToProperty.remove(pointSizeProp);
        delete pointSizeProp;
    }
    d_ptr->m_propertyToPointSize.remove(property);

    QtProperty *boldProp = d_ptr->m_propertyToBold[property];
    if (boldProp) {
        d_ptr->m_boldToProperty.remove(boldProp);
        delete boldProp;
    }
    d_ptr->m_propertyToBold.remove(property);

    QtProperty *italicProp = d_ptr->m_propertyToItalic[property];
    if (italicProp) {
        d_ptr->m_italicToProperty.remove(italicProp);
        delete italicProp;
    }
    d_ptr->m_propertyToItalic.remove(property);

    QtProperty *underlineProp = d_ptr->m_propertyToUnderline[property];
    if (underlineProp) {
        d_ptr->m_underlineToProperty.remove(underlineProp);
        delete underlineProp;
    }
    d_ptr->m_propertyToUnderline.remove(property);

    QtProperty *strikeOutProp = d_ptr->m_propertyToStrikeOut[property];
    if (strikeOutProp) {
        d_ptr->m_strikeOutToProperty.remove(strikeOutProp);
        delete strikeOutProp;
    }
    d_ptr->m_propertyToStrikeOut.remove(property);

    QtProperty *kerningProp = d_ptr->m_propertyToKerning[property];
    if (kerningProp) {
        d_ptr->m_kerningToProperty.remove(kerningProp);
        delete kerningProp;
    }
    d_ptr->m_propertyToKerning.remove(property);

    d_ptr->m_values.remove(property);
}







// QtCursorPropertyManager

Q_GLOBAL_STATIC(QtCursorDatabase, cursorDatabase)

class QtCursorPropertyManagerPrivate
{
    QtCursorPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtCursorPropertyManager)
public:
    typedef QMap<const QtProperty *, QCursor> PropertyValueMap;
    PropertyValueMap m_values;
};

/*!
    \class QtCursorPropertyManager

    \brief The QtCursorPropertyManager provides and manages QCursor properties.

    A cursor property has a current value which can be
    retrieved using the value() function, and set using the setValue()
    slot. In addition, QtCursorPropertyManager provides the
    valueChanged() signal which is emitted whenever a property created
    by this manager changes.

    \sa QtAbstractPropertyManager
*/

/*!
    \fn void QtCursorPropertyManager::valueChanged(QtProperty *property, const QCursor &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.

    \sa setValue()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtCursorPropertyManager::QtCursorPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtCursorPropertyManagerPrivate;
    d_ptr->q_ptr = this;
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtCursorPropertyManager::~QtCursorPropertyManager()
{
    clear();
    delete d_ptr;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by this manager, this
    function returns a default QCursor object.

    \sa setValue()
*/
#ifndef QT_NO_CURSOR
QCursor QtCursorPropertyManager::value(const QtProperty *property) const
{
    return d_ptr->m_values.value(property, QCursor());
}
#endif

/*!
    \reimp
*/
QString QtCursorPropertyManager::valueText(const QtProperty *property) const
{
   const QtCursorPropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QString();

    return cursorDatabase()->cursorToShapeName(it.value());
}

/*!
    \reimp
*/
QIcon QtCursorPropertyManager::valueIcon(const QtProperty *property) const
{
    const QtCursorPropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QIcon();

    return cursorDatabase()->cursorToShapeIcon(it.value());
}

/*!
    \fn void QtCursorPropertyManager::setValue(QtProperty *property, const QCursor &value)

    Sets the value of the given \a property to \a value.

    \sa value(), valueChanged()
*/
void QtCursorPropertyManager::setValue(QtProperty *property, const QCursor &value)
{
#ifndef QT_NO_CURSOR
    const QtCursorPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    if (it.value().shape() == value.shape() && value.shape() != Qt::BitmapCursor)
        return;

    it.value() = value;

    emit propertyChanged(property);
    emit valueChanged(property, value);
#endif
}

/*!
    \reimp
*/
void QtCursorPropertyManager::initializeProperty(QtProperty *property)
{
#ifndef QT_NO_CURSOR
    d_ptr->m_values[property] = QCursor();
#endif
}

/*!
    \reimp
*/
void QtCursorPropertyManager::uninitializeProperty(QtProperty *property)
{
    d_ptr->m_values.remove(property);
}

#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif

#include "moc_qtpropertymanager.cpp"
#include "qtpropertymanager.moc"

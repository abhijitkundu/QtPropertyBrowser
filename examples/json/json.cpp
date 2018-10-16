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

#include "json.h"

#include <QApplication>
#include <QDate>
#include <QLocale>
#include <QFile>
#include <QStack>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "qtpropertymanager.h"
#include "qtvariantproperty.h"
#include "qtgroupboxpropertybrowser.h"
#include "qtbuttonpropertybrowser.h"
#include "qttreepropertybrowser.h"

#include <QMessageBox>

#include <QtDebug>

const char *sample_config = "sample.json";
const char *sample_settings = "setup.json";


PropJsonIO::PropJsonIO(QObject *parent) :
    QObject(parent)
{}

PropJsonIO::~PropJsonIO() {}

void PropJsonIO::currentItemChanged(QtBrowserItem *item)
{}

struct SetupStack
{
    QStack<QString> m_setupTree;
    QJsonDocument m_setupCache;

    SetupStack()
    {}

    QString getPropertyId(const QString &name)
    {
        QString outPr;
        for(const QString & t : m_setupTree)
        {
            outPr.append(t);
            outPr.append('/');
        }
        outPr.append(name);
        return outPr;
    }

    void loadSetup(const QString &path)
    {
        m_setupTree.clear();
        QFile f;
        f.setFileName(path);
        if(!f.open(QIODevice::ReadOnly))
        {
            QMessageBox::warning(nullptr, "oops", QString("Oops: %1").arg(f.errorString()));
            m_setupCache = QJsonDocument();
        }

        QByteArray layoutJson = f.readAll();
        f.close();
        QJsonParseError errCode;
        m_setupCache = QJsonDocument::fromJson(layoutJson, &errCode);
        if(errCode.error != QJsonParseError::NoError)
            m_setupCache = QJsonDocument();
    }
};

static QVariant retrieve_property(const SetupStack &setupTree, QString prop, const QVariant &defaultValue)
{
    const QJsonDocument d = setupTree.m_setupCache;
    QJsonObject o = d.object();
    QVariant out;
    QString outPr;

    for(const QString & t : setupTree.m_setupTree)
    {
        outPr.append(t);
        outPr.append(" << ");
        if(!o.contains(t))
        {
            qDebug() << outPr << prop << defaultValue << "DEFAULT-TREE";
            return defaultValue;
        }
        o = o[t].toObject();
        out = o[t].toVariant();
    }

    if(!o.contains(prop))
    {
        qDebug() << outPr << prop << defaultValue << "DEFAULT-PROP";
        return defaultValue;
    }
    out = o[prop].toVariant();

    qDebug() << outPr << prop << out << "INPUT";

    return out;
}

static QHash<QString, bool> loadPropertiesLoayout_requiredTypes =
{
    {"group", false},
    {"checkbox", false},
    {"spinbox", true},
    {"lineedit", false},
};

static bool loadPropertiesLoayout_hasType(const QString &type)
{
    QString l = type.toLower();
    if(!loadPropertiesLoayout_requiredTypes.contains(l))
        return false;
    return loadPropertiesLoayout_requiredTypes[l];
}

static void loadPropertiesLoayout_fillElements(SetupStack setupTree, const QJsonArray &elements, QtVariantPropertyManager *manager, QtProperty *target, QWidget *parent = nullptr, QString *err = nullptr)
{
    for(const QJsonValue &o : elements)
    {
        QString type = o["type"].toString("invalid");
        QString name = o["name"].toString(type);
        QString title = o["title"].toString(name);
        QString control = o["control"].toString();
        QtVariantProperty *item = nullptr;

        if(control.isEmpty())
            continue;//invalid
        if(loadPropertiesLoayout_hasType(control) && (type.isEmpty() || type == "invalid"))
            continue;
        if(name.isEmpty())
            continue;//invalid

        qDebug() << "property" << setupTree.getPropertyId(name);

        if(!control.compare("spinBox", Qt::CaseInsensitive))
        {
            if(!type.compare("int", Qt::CaseInsensitive))
            {
                int valueDefault = o["value-default"].toInt();
                int valueMin = o["value-min"].toInt(0);
                int valueMax = o["value-max"].toInt(100);
                int singleStep = o["single-step"].toInt(1);
                item = manager->addProperty(QVariant::Int, title);
                item->setValue(retrieve_property(setupTree, name, valueDefault));
                item->setAttribute(QLatin1String("minimum"), valueMin);
                item->setAttribute(QLatin1String("maximum"), valueMax);
                item->setAttribute(QLatin1String("singleStep"), singleStep);
                item->setPropertyId(setupTree.getPropertyId(name));
                target->addSubProperty(item);
            }
            else if(!type.compare("double", Qt::CaseInsensitive) || !type.compare("float", Qt::CaseInsensitive))
            {
                double valueDefault = o["value-default"].toDouble();
                double valueMin = o["value-min"].toDouble(0.0);
                double valueMax = o["value-max"].toDouble(100.0);
                double singleStep = o["single-step"].toDouble(1);
                int decimals = o["decimals"].toInt(1);
                item = manager->addProperty(QVariant::Double, title);
                item->setValue(retrieve_property(setupTree, name, valueDefault));
                item->setAttribute(QLatin1String("minimum"), valueMin);
                item->setAttribute(QLatin1String("maximum"), valueMax);
                item->setAttribute(QLatin1String("singleStep"), singleStep);
                item->setAttribute(QLatin1String("decimals"), decimals);
                item->setPropertyId(setupTree.getPropertyId(name));
                target->addSubProperty(item);
            }
        }
        else if(!control.compare("checkBox", Qt::CaseInsensitive))
        {
            bool valueDefault = o["value-default"].toBool();
            item = manager->addProperty(QVariant::Bool, title);
            item->setValue(retrieve_property(setupTree, name, valueDefault));
            target->addSubProperty(item);
        }
        else if(!control.compare("lineEdit", Qt::CaseInsensitive))
        {
            int maxLength = o["max-length"].toInt(-1);
            QString valueDefault = o["value-default"].toString();
            QString validator = o["validator"].toString();
            item = manager->addProperty(QVariant::String, title);
            item->setValue(retrieve_property(setupTree, name, valueDefault));
            item->setAttribute(QLatin1String("maxlength"), maxLength);
            if(!validator.isEmpty())
                item->setAttribute(QLatin1String("regExp"), QRegExp(validator));
            item->setPropertyId(setupTree.getPropertyId(name));
            target->addSubProperty(item);
        }
        else if(!control.compare("comboBox", Qt::CaseInsensitive))
        {
            item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), title);
            QStringList enumList;
            QVariantList children = o["elements"].toArray().toVariantList();
            for(QVariant &j : children)
                enumList.push_back(j.toString());
            int valueDefault = o["value-default"].toInt();
            item->setAttribute(QLatin1String("enumNames"), enumList);
            item->setValue(retrieve_property(setupTree, name, valueDefault));
            item->setPropertyId(setupTree.getPropertyId(name));
            target->addSubProperty(item);
        }
        else if(!control.compare("flagBox", Qt::CaseInsensitive))
        {
            item = manager->addProperty(QtVariantPropertyManager::flagTypeId(), title);
            QStringList enumList;
            QVariantList children = o["elements"].toArray().toVariantList();
            for(QVariant &j : children)
                enumList.push_back(j.toString());
            int valueDefault = o["value-default"].toInt();
            item->setAttribute(QLatin1String("flagNames"), enumList);
            item->setValue(retrieve_property(setupTree, name, valueDefault));
            item->setPropertyId(setupTree.getPropertyId(name));
            target->addSubProperty(item);
        }
        else if(!control.compare("sizeBox", Qt::CaseInsensitive))
        {
            item = manager->addProperty(QVariant::Size, title);
            QJsonArray defArr = o["value-default"].toArray();
            QJsonArray defMin = o["value-min"].toArray();
            QJsonArray defMax = o["value-max"].toArray();
            QSize valueDefault = QSize(defArr[0].toInt(), defArr[1].toInt());
            QSize valueMin = QSize(defMin[0].toInt(), defMin[1].toInt());
            QSize valueMax = QSize(defMax[0].toInt(), defMax[1].toInt());
            item->setValue(retrieve_property(setupTree, name, valueDefault));
            item->setAttribute(QLatin1String("minimum"), valueMin);
            item->setAttribute(QLatin1String("maximum"), valueMax);
            item->setPropertyId(setupTree.getPropertyId(name));
            target->addSubProperty(item);
        }
        else if(!control.compare("pointbox", Qt::CaseInsensitive))
        {
            QJsonArray defArr = o["value-default"].toArray();
            QPoint valueDefault = QPoint(defArr[0].toInt(), defArr[1].toInt());
            item = manager->addProperty(QVariant::Point, title);
            item->setValue(retrieve_property(setupTree, name, valueDefault));
            //TODO: Feed them with QPoint
            //item->setAttribute(QLatin1String("minimum"), valueMin);
            //item->setAttribute(QLatin1String("maximum"), valueMax);
            item->setPropertyId(setupTree.getPropertyId(name));
            target->addSubProperty(item);
        }
        else if(!control.compare("rectbox", Qt::CaseInsensitive))
        {
            QJsonArray defArr = o["value-default"].toArray();
            QRect valueDefault = QRect(defArr[0].toInt(), defArr[1].toInt(),
                                       defArr[2].toInt(), defArr[3].toInt());
            item = manager->addProperty(QVariant::Rect, title);
            item->setValue(retrieve_property(setupTree, name, valueDefault));
            //TODO: Feed them with QRect
            //item->setAttribute(QLatin1String("minimum"), valueMin);
            //item->setAttribute(QLatin1String("maximum"), valueMax);
            item->setPropertyId(setupTree.getPropertyId(name));
            target->addSubProperty(item);
        }
        /* TODO:
         * - QSizeF
         * - QRectF
         * - QPointF
         */
        else if(!control.compare("group", Qt::CaseInsensitive))
        {
            QJsonArray children = o["children"].toArray();
            if(!children.isEmpty())
            {
                QtProperty *subGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), title);
                setupTree.m_setupTree.push(name);
                loadPropertiesLoayout_fillElements(setupTree, children, manager, subGroup, parent, err);
                setupTree.m_setupTree.pop();
                target->addSubProperty(subGroup);
            }
        }
    }
}

static QtAbstractPropertyBrowser *loadPropertiesLoayout(const SetupStack &stack, const QByteArray &json, QWidget *parent = nullptr, QString *err = nullptr)
{
    QtAbstractPropertyBrowser *gui = nullptr;
    QString style;
    QString title;

    QJsonParseError errCode;
    QJsonDocument layout = QJsonDocument::fromJson(json, &errCode);


    if(errCode.error != QJsonParseError::NoError)
    {
        if(err)
            *err = errCode.errorString();
        return nullptr;
    }

    QJsonObject layoutData = layout.object();

    style = layoutData["style"].toString();
    title = layoutData["title"].toString();
    if(style == "groupbox")
        gui = new QtGroupBoxPropertyBrowser(parent);
    else if(style == "button")
        gui = new QtButtonPropertyBrowser(parent);
    else // "tree" is default
    {
        QtTreePropertyBrowser *gui_t = new QtTreePropertyBrowser(parent);
        gui_t->setPropertiesWithoutValueMarked(true);
        gui = gui_t;
    }

    QtVariantPropertyManager *variantManager = new QtVariantPropertyManager(gui);

    QtProperty *topItem = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), title);

    QJsonArray layoutEntries = layoutData["layout"].toArray();
    loadPropertiesLoayout_fillElements(stack, layoutEntries, variantManager, topItem, parent, err);

    variantManager->connect(variantManager, &QtVariantPropertyManager::valueChanged,
        [](QtProperty *p,QVariant v)
        {
            qDebug() << "changed:" << p->propertyId() << v;
        }
    );

    QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(gui);

    gui->setFactoryForManager(variantManager, variantFactory);
    gui->addProperty(topItem);

    return gui;
}


int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QFile f;
    f.setFileName(sample_config);
    if(!f.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(nullptr, "oops", QString("Oops: %1").arg(f.errorString()));
        return 1;
    }

    SetupStack stack;
    stack.loadSetup(sample_settings);

    QByteArray layoutJson = f.readAll();
    f.close();

//    QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory();

    QString error;
    //QtTreePropertyBrowser *variantEditor = new QtTreePropertyBrowser();
    QtAbstractPropertyBrowser *variantEditor = loadPropertiesLoayout(stack, layoutJson, nullptr, &error);
    if(!variantEditor)
    {
        QMessageBox::warning(nullptr, "oops", QString("Oops: %1").arg(error));
        return 1;
    }

//    variantEditor->setFactoryForManager(variantManager, variantFactory);
//    variantEditor->addProperty(topItem);

    variantEditor->show();

    int ret = app.exec();

    // delete variantManager;
    // delete variantFactory;
    delete variantEditor;

    return ret;
}

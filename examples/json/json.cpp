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

static QJsonObject rectToArray(QVariant r)
{
    QRect rekt = r.toRect();
    QJsonObject a;
    a["x"] = QJsonValue::fromVariant(rekt.x());
    a["y"] = QJsonValue::fromVariant(rekt.y());
    a["w"] = QJsonValue::fromVariant(rekt.width());
    a["h"] = QJsonValue::fromVariant(rekt.height());
    return a;
}

static QJsonObject sizeToArray(QVariant r)
{
    QSize rekt = r.toSize();
    QJsonObject a;
    a["w"] = QJsonValue::fromVariant(rekt.width());
    a["h"] = QJsonValue::fromVariant(rekt.height());
    return a;
}

static QJsonObject pointToArray(QVariant r)
{
    QPoint rekt = r.toPoint();
    QJsonObject a;
    a["x"] = QJsonValue::fromVariant(rekt.x());
    a["y"] = QJsonValue::fromVariant(rekt.y());
    return a;
}

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
            return;
        }

        QByteArray layoutJson = f.readAll();
        f.close();
        QJsonParseError errCode;
        m_setupCache = QJsonDocument::fromJson(layoutJson, &errCode);
        if(errCode.error != QJsonParseError::NoError)
            m_setupCache = QJsonDocument();
    }

    void saveSetup(const QString &path)
    {
        QFile f;
        f.setFileName(path);
        if(!f.open(QIODevice::WriteOnly))
        {
            QMessageBox::warning(nullptr, "oops", QString("Oops: %1").arg(f.errorString()));
            return;
        }
        f.write(m_setupCache.toJson());
        f.close();
    }

    void setValue(const QString &propertyId, QVariant value)
    {
        QStringList stack = propertyId.split("/");
        QJsonObject o = m_setupCache.object();
        QStack<QJsonObject> stack_o;
        QString top;
        for(int i = 0; i < stack.size(); i++)
        {
            QString &t = stack[i];
            top = t;
            if(i == stack.size() - 1)
                break;
            stack_o.push(o);
            o = o[t].toObject();
        }

        switch(value.type())
        {
        case QVariant::Rect:
            o[top] = rectToArray(value);
            break;
        case QVariant::Point:
            o[top] = pointToArray(value);
            break;
        case QVariant::Size:
            o[top] = sizeToArray(value);
            break;
        default:
            o[top] = QJsonValue::fromVariant(value);
            break;
        }

        for(int i = stack.size() - 2; i >= 0; i--)
        {
            QString &s = stack[i];
            QJsonObject oo = stack_o.pop();
            oo[s] = o;
            o = oo;
        }

        m_setupCache.setObject(o);
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
    switch(defaultValue.type())
    {
    case QVariant::Size:
        {
            QJsonObject sz = o[prop].toObject();
            out = QSize(sz["w"].toInt(), sz["h"].toInt());
        }
        break;
    case QVariant::Point:
        {
            QJsonObject sz = o[prop].toObject();
            out = QPoint(sz["x"].toInt(), sz["y"].toInt());
        }
        break;
    case QVariant::Rect:
        {
            QJsonObject sz = o[prop].toObject();
            out = QRect(sz["x"].toInt(), sz["y"].toInt(), sz["w"].toInt(), sz["h"].toInt());
        }
        break;
    default:
        out = o[prop].toVariant();
        break;
    }

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
            item->setPropertyId(setupTree.getPropertyId(name));
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
            QJsonObject defArr = o["value-default"].toObject();
            QJsonObject defMin = o["value-min"].toObject();
            QJsonObject defMax = o["value-max"].toObject();
            QSize valueDefault = QSize(defArr["w"].toInt(), defArr["h"].toInt());
            QSize valueMin = QSize(defMin["w"].toInt(), defMin["h"].toInt());
            QSize valueMax = QSize(defMax["w"].toInt(), defMax["h"].toInt());
            item->setValue(retrieve_property(setupTree, name, valueDefault));
            item->setAttribute(QLatin1String("minimum"), valueMin);
            item->setAttribute(QLatin1String("maximum"), valueMax);
            item->setPropertyId(setupTree.getPropertyId(name));
            target->addSubProperty(item);
        }
        else if(!control.compare("pointbox", Qt::CaseInsensitive))
        {
            QJsonObject defArr = o["value-default"].toObject();
            QJsonObject defMin = o["value-min"].toObject();
            QJsonObject defMax = o["value-max"].toObject();
            QPoint valueDefault = QPoint(defArr["x"].toInt(), defArr["y"].toInt());
            QPoint valueMin = QPoint(defMin["x"].toInt(), defMin["y"].toInt());
            QPoint valueMax = QPoint(defMax["x"].toInt(), defMax["y"].toInt());
            item = manager->addProperty(QVariant::Point, title);
            item->setValue(retrieve_property(setupTree, name, valueDefault));
            item->setAttribute(QLatin1String("minimum"), valueMin);
            item->setAttribute(QLatin1String("maximum"), valueMax);
            item->setPropertyId(setupTree.getPropertyId(name));
            target->addSubProperty(item);
        }
        else if(!control.compare("rectbox", Qt::CaseInsensitive))
        {
            QJsonObject defArr = o["value-default"].toObject();
            QJsonObject defMin = o["value-min"].toObject();
            QJsonObject defMax = o["value-max"].toObject();
            QRect valueDefault = QRect(defArr["x"].toInt(), defArr["y"].toInt(),
                                       defArr["w"].toInt(), defArr["h"].toInt());
            QRect valueMin     = QRect(defMin["x"].toInt(), defMin["y"].toInt(),
                                       defMin["w"].toInt(), defMin["h"].toInt());
            QRect valueMax     = QRect(defMax["x"].toInt(), defMax["y"].toInt(),
                                       defMax["w"].toInt(), defMax["h"].toInt());
            item = manager->addProperty(QVariant::Rect, title);
            item->setValue(retrieve_property(setupTree, name, valueDefault));
            item->setAttribute(QLatin1String("minimum"), valueMin);
            item->setAttribute(QLatin1String("maximum"), valueMax);
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

static QtAbstractPropertyBrowser *loadPropertiesLoayout(SetupStack &stack, const QByteArray &json, QWidget *parent = nullptr, QString *err = nullptr)
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
        [&stack](QtProperty *p,QVariant v)
        {
            QString pid = p->propertyId();
            qDebug() << "changed:" << pid << v;
            if(!pid.isEmpty())
                stack.setValue(p->propertyId(), v);
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

    stack.saveSetup(sample_settings);

    // delete variantManager;
    // delete variantFactory;
    delete variantEditor;

    return ret;
}

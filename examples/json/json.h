#ifndef JSONNNNNNN
#define JSONNNNNNN

#include <QObject>
#include <QtProperty>
#include <QJsonDocument>

class PropJsonIO : public QObject
{
    Q_OBJECT

    QJsonDocument m_doc;
public:
    explicit PropJsonIO(QObject *parent = nullptr);
    virtual ~PropJsonIO();

public slots:
    void currentItemChanged(QtBrowserItem *item);
};

#endif//JSONNNNNNN

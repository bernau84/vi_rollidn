#ifndef I_PROC_STAGE_H
#define I_PROC_STAGE_H

#include <QObject>
#include <QFile>
#include <QJsonDocument>

#include "../t_vi_setup.h"

class i_proc_stage : public QObject
{
    Q_OBJECT

private:
    //constructor helpers
    QJsonObject __set_from_file(const QString &path){

        // default config
        QFile f_def(path);  //from resources
        if(f_def.open(QIODevice::ReadOnly | QIODevice::Text)){

            QByteArray f_data = f_def.read(64000);

            QJsonDocument js_doc = QJsonDocument::fromJson(f_data);
            if(!js_doc.isEmpty()){

                //qDebug() << js_doc.toJson();
                return js_doc.object();
            }
        }

        return QJsonObject();
    }

protected:
   t_collection par;

public slots:
   //data processor
   virtual int proc(int p1, void *p2) = 0;
   //init privtaes from configuration
   virtual int reload(int p) = 0;

signals:
    void next(int p1, void *p2);

public:
    //read / update config parameter runtime
    //new value is not promoted to json file
    QVariant config(QString &name, QVarinat *value){

        if(!par.ask(name))
            return QVariant();

        if(value){

            QJsonValue jval_set = QJsonValue::fromVariant(*value);
            QJsonValue jval_get = par[name].set(jval_set);

            reload();

            return jval_get.toVariant();
        }

        return par[name].get().toVariant();
    }

    i_proc_stage(const QString &js_config, QObject *parent = NULL):
        QObject(parent),
        par(__set_from_file(js_config))
    {
    }

    virtual ~i_proc_stage(){;}
};

#endif // I_PROC_STAGE_H

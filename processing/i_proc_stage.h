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
    virtual int proc(int p1, void *p2) = 0;

signals:
    void next(int p1, void *p2);

public:

    i_proc_stage(QString &js_config, QObject *parent = NULL):
        QObject(parent),
        par(__set_from_file(js_config))
    {
        js_config = js_config; /*! \todo init collection from file */
    }

    virtual ~i_proc_stage(){;}
};

#endif // I_PROC_STAGE_H

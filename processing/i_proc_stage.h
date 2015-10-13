#ifndef I_PROC_STAGE_H
#define I_PROC_STAGE_H

#include <QObject>
#include "../t_vi_setup.h"

class i_proc_stage : public QObject
{
    Q_OBJECT

private:

   t_collection par;

public slots:
    virtual int proc(int p1, void *p2) = 0;

signals:
    void next(int p1, void *p2);

public:

    i_proc_stage(QString &js_config, QObject *parent = NULL):
        QObject(parent){

        js_config = js_config; /*! \todo init collection from file */
    }

    ~i_proc_stage();
};

#endif // I_PROC_STAGE_H

#ifndef I_PROC_STAGE_H
#define I_PROC_STAGE_H

#include "../t_vi_setup.h"

class i_proc_stage
{
private:

   t_collection par;

public slots:
    virtual proc(int p1, void *p2) = 0;

signals:
    void next(int p1, void *p2);

public:

    i_proc_stage(QString &js_config){

        js_config = js_config; /*! \todo inict collection dfrom file */
    }

    ~i_proc_stage();
};

#endif // I_PROC_STAGE_H

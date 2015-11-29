#ifndef T_ROLL_ID_RECORD_STORE
#define T_ROLL_ID_RECORD_STORE

#include <QString>
#include <QFile>

#define RECORD_PATTERN_LOG  "log%1.txt"
#define RECORD_PATTERN_IMG  "pic%1.bmp"

class t_roll_idn_record_storage {

private:
    QString m_storage_path;
    int m_history;
    int m_counter;

public:
    t_roll_idn_record_storage(QString storage_path, int history = 100):
        m_storage_path(storage_path),
        m_history(history)
    {
        m_counter = 0;
    }

    ~t_roll_idn_record_storage(){

    }

    void add(QString &log, QImage &img){

        m_counter = (m_counter + 1) % m_history;

        QString log_path = QString(RECORD_PATTERN_LOG).arg(m_counter);
        QFile::remove(log_path);

        QFile log_file(log_path);
        log_file.open(QIODevice::WriteOnly | QIODevice::Text);
        log_file.write(log.toLatin1());
        log_file.close();

        QString img_path = QString(RECORD_PATTERN_IMG).arg(m_counter);
        QFile::remove(img_path);
        img.save(img_path);
    }
};

#endif // T_ROLL_ID_RECORD_STORE


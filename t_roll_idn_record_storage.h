#ifndef T_ROLL_ID_RECORD_STORE
#define T_ROLL_ID_RECORD_STORE

#include <QString>
#include <QFile>
#include <QLabel>

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

    void increment(){

        m_counter = (m_counter + 1) % m_history;

        QString log_path = QString(RECORD_PATTERN_LOG).arg(m_counter);
        QFile::remove(log_path);

        QString img_path = QString(RECORD_PATTERN_IMG).arg(m_counter);
        QFile::remove(img_path);
    }

    void append(QString &log){

        qDebug() << log;

        QFile log_file(QString(RECORD_PATTERN_LOG).arg(m_counter));
        log_file.open(QIODevice::Append | QIODevice::Text);
        log_file.write(log.toLatin1());
        log_file.close();
    }

    void insert(QImage &img){

        if(!img.isNull()){

            QLabel vizual;
            vizual.setPixmap(QPixmap::fromImage(img));
            vizual.show();

            QString img_path = QString(RECORD_PATTERN_IMG).arg(m_counter);
            QFile::remove(img_path);
            img.save(img_path);
        }
    }

    void add(QString &log, QImage &img){

        increment();
        append(log);
        insert(img);
    }
};

#endif // T_ROLL_ID_RECORD_STORE


#ifndef T_ROLL_ID_RECORD_STORE
#define T_ROLL_ID_RECORD_STORE

#include <QString>
#include <QFile>
#include <QLabel>
#include <QTime>
#include <QSettings>
#include <QDir>

#define RECORD_PATTERN_LOG  "log%1.txt"
#define RECORD_PATTERN_IMG  "pic%1.bmp"
#define RECORD_PATTERN_CNF  "log.cnf"
#define RECORD_LAST_STDOUT  "stdout.log"
#define RECORD_STORED_STDOUT  "stdout%1.log"

class t_roll_idn_record_storage {

private:
    QString m_storage_path;
    QSettings m_cnf;
    int m_history;
    int m_counter;
    int m_stdout;

public:
    t_roll_idn_record_storage(QString storage_path, int history = 100):
        m_storage_path(storage_path),
        m_cnf(m_storage_path + "\\" + RECORD_PATTERN_CNF, QSettings::IniFormat),
        m_history(history)
    {
        m_counter = 0;
        m_stdout = 0;

        QDir dir(m_storage_path);
        if (!dir.exists()){
          dir.mkdir(".");
        }

        m_counter = m_cnf.value("LAST", m_counter).toInt();
        m_history = m_cnf.value("HISTORY", m_history).toInt();
        m_stdout = m_cnf.value("STDCNT", m_stdout).toInt();

        //vycte a pod unikatnim jmenem zkopiruje stdout z minula
        QFile pre_stdout(RECORD_LAST_STDOUT);
        if(!pre_stdout.exists())
            return;

        QString rec_stdout = m_storage_path + "\\";
        rec_stdout += QString(RECORD_STORED_STDOUT).arg(m_stdout);
        pre_stdout.copy(rec_stdout);

        m_stdout = (1 + m_stdout) % m_history;
        m_cnf.setValue("STDCNT", m_stdout);
        m_cnf.sync();
    }

    ~t_roll_idn_record_storage()
    {
        m_cnf.setValue("LAST", m_counter);
        m_cnf.setValue("HISTORY", m_history);
        m_cnf.setValue("STDCNT", m_stdout);
        m_cnf.sync();
    }

    void increment(){

        m_counter = (m_counter + 1) % m_history;

        QString log_path = QString(RECORD_PATTERN_LOG).arg(m_counter);
        QFile::remove(m_storage_path + "/" + log_path);

        QString img_path = QString(RECORD_PATTERN_IMG).arg(m_counter);
        QFile::remove(m_storage_path + "/" + img_path);

        m_cnf.setValue("LAST", m_counter);
        m_cnf.sync();
    }

    void append(QString &log){

        QString stamp = QTime::currentTime().toString("hh:mm:ss.zzz");
        qDebug() << stamp << "\r\n" << log;

        QFile log_file(m_storage_path + "/" + QString(RECORD_PATTERN_LOG).arg(m_counter));
        log_file.open(QIODevice::Append | QIODevice::Text);
        log_file.write(stamp.toLatin1());
        log_file.write("\r\n");
        log_file.write(log.toLatin1());
        log_file.close();
    }

    void insert(QImage &img){

        if(!img.isNull()){

            QLabel vizual;
            vizual.setPixmap(QPixmap::fromImage(img));
            vizual.show();

            QString img_path = QString(RECORD_PATTERN_IMG).arg(m_counter);
            img.save(m_storage_path + "/" + img_path);
        }
    }

    void add(QString &log, QImage &img){

        increment();
        append(log);
        insert(img);
    }
};

#endif // T_ROLL_ID_RECORD_STORE


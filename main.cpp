#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QTime>
#include <QVector>
#include <QDesktopServices>
#include <QUrl>
//-----------------------------------------------------------------------------------------------------------------------------------

// -i=E:/temperature.csv || --in=E:/temperature.csv
// -o=E:/temperature_.csv || --out=E:/temperature_.csv
//-----------------------------------------------------------------------------------------------------------------------------------

/*!
 * \brief writeInfo              Запись в выходной файл
 * \param path                   Путь к выходному файлу
 * \param strFileIn              Данные из входного файла
 * \param timeWork_count_on_off  Данные из foundTime() ( время во вкл/откл состоянии; какое вкл/откл по счету )
 */
void writeInfo(const QString& path, const QString& strFileIn, const QString& timeWork_count_on_off)
{
    QFile fileOut;
    fileOut.setFileName(path);

    if(fileOut.open(QIODevice::Append))
    {
        QTextStream writeStream(&fileOut);

        writeStream << strFileIn << timeWork_count_on_off << '\n';

        fileOut.close();
    }
    else
    {
        qWarning() << Q_FUNC_INFO << path << "fileOut not open";
    }
}
//-----------------------------------------------------------------------------------------------------------------------------------

/*!
 * \brief foundTime     Расчет времени
 * \param pathIn        Путь входного файла
 * \param countOn       Колличество включений
 * \param countOff      Колличество отключений
 * \param countTimeOn   Время во включенном состоянии (мсек)
 * \param countTimeOff  Время в отключенном состоянии (мсек)
 * \param timeQV        QVector хранит время состояний
 */
void foundTime(const QString& pathIn, int& countOn, int& countOff, int& countTimeOn, int& countTimeOff,  QVector<QString>& timeQV)
{
    QFile fileIn(pathIn); // инициализация пути к файлу через конструктор (вместо вызова: fileIn.setFileName(pathIn))

    if(!fileIn.open(QIODevice::ReadOnly))
    {
        qWarning() << Q_FUNC_INFO << "file not open. Path: " << pathIn;
        return;
    }

    int timePrev = 0;

    QTextStream writeStream(&fileIn);

    QString status;

    int timeWork = 0;

    while(!writeStream.atEnd())
    {
        QString str = writeStream.readLine();

        QStringList str_split = str.split(';');

        // 04.12.2023 00:05:46
        // ON
        //
        const QString& dateTime = str_split[0];
        status                  = str_split[1];

        QStringList dateTime_split = dateTime.split(' ');
        QString     time           = dateTime_split[1];
        QTime       timeNow        = QTime::fromString(time, "hh:mm:ss");

        int time_Now = timeNow.msecsSinceStartOfDay();

//        if(status == "ON ")
        if(status.contains("on", Qt::CaseInsensitive)) // поиск подстроки без учета регистра
        {
            if(timePrev == 0)
            {
                timePrev     = time_Now - QTime(0, 0, 0).msecsSinceStartOfDay();
                countTimeOff = time_Now - QTime(0, 0, 0).msecsSinceStartOfDay();
            }
            else
            {
                timeWork = -(timePrev - time_Now);

                QString timeWorkStr = QTime::fromMSecsSinceStartOfDay(timeWork).toString("hh:mm:ss");

                timePrev      = time_Now;
                countTimeOff += timeWork;
                countOn++;

                QString tmpString = timeWorkStr + ";" + QString::number(countOn);

                timeQV.push_back(tmpString);   // пишем в вектор
            }
        }
        else if(status.contains("off", Qt::CaseInsensitive)) // поиск подстроки без учета регистра
        {
            if(timePrev == 0)
            {
                timePrev    = time_Now - QTime(0, 0, 0).msecsSinceStartOfDay();
                countTimeOn = time_Now - QTime(0, 0, 0).msecsSinceStartOfDay();
            }
            else
            {
                timeWork = -(timePrev - time_Now);

                QString timeWorkStr = QTime::fromMSecsSinceStartOfDay(timeWork).toString("hh:mm:ss");

                timePrev     = time_Now;
                countTimeOn += timeWork;
                countOff++;

                QString tmpString = timeWorkStr + ";" + QString::number(countOff);

                timeQV.push_back(tmpString);
            }
        }
    }

    if(status.contains("on", Qt::CaseInsensitive))
    {
        timeWork     = QTime(23, 59, 59).msecsSinceStartOfDay() - timePrev;  // 23.59.59 - время последнего переключения
        countTimeOn += QTime(23, 59, 59).msecsSinceStartOfDay() - timePrev;

        QString timeWorkStr = QTime::fromMSecsSinceStartOfDay(timeWork).toString("hh:mm:ss");
        QString tmpString   = timeWorkStr + ";" + QString::number(countOn);

        timeQV.push_back(tmpString);
    }
    else if(status.contains("off", Qt::CaseInsensitive))
    {
        timeWork      = QTime(23, 59, 59).msecsSinceStartOfDay() - timePrev; // 23.59.59 - время последнего переключения
        countTimeOff += QTime(23, 59, 59).msecsSinceStartOfDay() - timePrev;

        QString timeWorkStr = QTime::fromMSecsSinceStartOfDay(timeWork).toString("hh:mm:ss");
        QString tmpString   = timeWorkStr + ";" + QString::number(countOff);

        timeQV.push_back(tmpString);
    }

    fileIn.close();
}
//-----------------------------------------------------------------------------------------------------------------------------------
/*!
 * \brief printHelp  Справочная информация как запускать программу
 */
void printHelp()
{
    qDebug() << "-i or --in\n"
             << "Relative or absolute path to input  file.\n"
             << "Example:\n"
             << "   -i=D:/data/file.csv\n"

             << "-o or --out\n"
             << "Relative or absolute path to output  file.\n"
             << "Example:\n"
             << "   -o=D:/data/file_result.csv\n"
             << "Note:\n"
             << "        if the path to the output file is not passed,\n"
             << "        the path to the input file will be used\n"
             << "        and ""out"" will be added to the name of the input file";
}


//-----------------------------------------------------------------------------------------------------------------------------------

/*!
 * \brief parseArgs  Разбор аргументов командной строки
 * \param argc       Колличество аргументов командной строки
 * \param argv       Аргументы командной строки
 * \param pathIn     Путь входного файла
 * \param pathOut    Путь выходного файла
 * \param isHelp     Признак, продолжает программа работать или нет
 *
 *
 * isHelp == true (прекращает работать) если есть аргумент "-h" или "--help", иначе isHelp == false (продолжает работать).
 */
void parseArgs( int argc, char* argv[], QString& pathIn, QString& pathOut, bool& isHelp)
{
    isHelp = false;

    for(int i = 1; i < argc; i++)
    {
        QString path = argv[i];

        if(path == "-h" || path == "--help")
        {
            isHelp = true;
            return;
        }

        QStringList path_split = path.split('=');

        QString& key = path_split[0];
        QString& val = path_split[1];

        if(key == "-i" || key == "--in")
        {
            pathIn = val;
        }

        if(key == "-o" || key == "--out")
        {
            pathOut = val;
        }
    }

    if(pathOut.isEmpty())
    {
        QFileInfo fileInfo(pathIn);                         // Инициализация пути файла через конструктор

        QString pathDir = fileInfo.path();
        QString  fileName(fileInfo.baseName() + "_out");

        pathOut = QString("%1/%2.csv").arg(pathDir).arg(fileName);
    }  
}
//-----------------------------------------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QString pathOut;        // путь выходного файла
    QString pathIn;         // путь входного  файла
    bool isHelp = false;

    parseArgs(argc, argv, pathIn, pathOut, isHelp);

    if(isHelp)
    {
        printHelp();
        return 0;
    }

    QFile fileIn(pathIn);
    if(!fileIn.exists())
    {
        qWarning() << Q_FUNC_INFO << "file not exists. PathIn: " << pathIn;
        return 1;
    }

    int countOn      = 0;   // кол-во включений
    int countOff     = 0;   // кол-во отключений
    int countTimeOn  = 0;   // кол-во времени во вкл состоянии (мсек)
    int countTimeOff = 0;   // кол-во времени во откл состоянии (мсек)

    QVector<QString> timeQV;

    foundTime(pathIn, countOn, countOff, countTimeOn, countTimeOff, timeQV);

    if(QFile::exists(pathOut))  // Проверка существует ли файл по указанному пути
    {
        QFile::remove(pathOut); // Если существует - удаляем
    }

    if(fileIn.open(QIODevice::ReadOnly))
    {
        QString     strs = fileIn.readAll();
        QStringList strs_split = strs.split("\r\n");

        strs_split.removeAll("");

//        info == 04.12.2023 00:05:46;ON;
//        for(QString& str : strs_split)            // Обход по значению
        for(int i = 0; i < strs_split.size(); i++)  // Обход по индексам
        {
            QString str = strs_split[i];            // чтоб не переделывать весь код
            writeInfo(pathOut, str, timeQV[i]);
        }

        fileIn.close();
    }

    QString statusON  = QTime::fromMSecsSinceStartOfDay(countTimeOn ).toString("hh:mm:ss");
    QString statusOFF = QTime::fromMSecsSinceStartOfDay(countTimeOff).toString("hh:mm:ss");

    writeInfo(pathOut, "Total;ON;" , statusON );
    writeInfo(pathOut, "Total;OFF;", statusOFF);

    QString timeOn_countOn   = QTime::fromMSecsSinceStartOfDay(countTimeOn  / countOn ).toString("hh:mm:ss");
    QString timeOff_countOff = QTime::fromMSecsSinceStartOfDay(countTimeOff / countOff).toString("hh:mm:ss");

    writeInfo(pathOut, "Medium;ON;" , timeOn_countOn  );
    writeInfo(pathOut, "Medium;OFF;", timeOff_countOff);

    return a.exec();
}
//-----------------------------------------------------------------------------------------------------------------------------------

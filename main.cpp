#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QTime>
#include <QVector>
//-----------------------------------------------------------------------------------------------------------------------------------

#include "Arguments.h"
#include "TimeWork.h"
//-----------------------------------------------------------------------------------------------------------------------------------

// -i=E:/temperature.csv || --in=E:/temperature.csv
// -o=E:/temperature_.csv || --out=E:/temperature_.csv
//-----------------------------------------------------------------------------------------------------------------------------------

enum
{
    DT     = 0,
    STATUS = 1
};

void saveToFile(Arguments& arguments, QVector<TimeWorkOnOff>& timeWorkOnOff, int& countTimeOnMSec, int& countTimeOffMSec)
{
    QFile fileOut(arguments.pathOut);

    if(QFile:: exists(arguments.pathOut))
    {
        QFile::remove(arguments.pathOut);
    }

    if(!fileOut.open(QIODevice :: Append))
        {
        qWarning() << Q_FUNC_INFO << arguments.pathOut << "not open";
            return;
        }

        QTextStream writeStream(&fileOut);
    for(TimeWorkOnOff& timeWorkOnOff: timeWorkOnOff)
    {
        QString val = timeWorkOnOff.status ? "On" : "Off"; // тернарный оператор (вместо if/else)

        writeStream << timeWorkOnOff.dateTime.toString("dd.MM.yyyy hh:mm:ss") << ";"
                    << val << ";"
                    << QTime::fromMSecsSinceStartOfDay(timeWorkOnOff.timeWorkMSec).toString("hh:mm:ss") << ";"
                    << timeWorkOnOff.countSwitch << ";"
                    <<'\n';
    }

    writeStream << "Total" << ";" << "On"  << ";" << QTime::fromMSecsSinceStartOfDay(countTimeOnMSec). toString("hh:mm:ss") << ";" << '\n';
    writeStream << "Total" << ";" << "Off" << ";" << QTime::fromMSecsSinceStartOfDay(countTimeOffMSec).toString("hh:mm:ss") << ";" << '\n';

    int mediumOn;
    int mediumOff;
    int sizeVec   = timeWorkOnOff.count();
    int lastIndex = sizeVec - 1;

    for(int i = lastIndex - 1; i <= lastIndex; i++)
        {
        if(timeWorkOnOff[i].status)
            {
            mediumOn  = countTimeOnMSec  / timeWorkOnOff[i].countSwitch;
            }

        if(!timeWorkOnOff[i].status)
            {
            mediumOff = countTimeOffMSec / timeWorkOnOff[i].countSwitch;
            }
    }

    writeStream << "Medium" << ";" << "On"  << ";" << QTime::fromMSecsSinceStartOfDay(mediumOn). toString("hh:mm:ss") << ";" <<'\n';
    writeStream << "Medium" << ";" << "Off" << ";" << QTime::fromMSecsSinceStartOfDay(mediumOff).toString("hh:mm:ss") << ";" <<'\n';

    fileOut.close();
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

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Arguments arguments = Arguments(argc, argv);

    if(arguments.isHelp)
    {
        printHelp();
        return 0;
    }

    QFile fileIn(arguments.pathIn);
    if(!fileIn.open(QIODevice::ReadOnly))
    {
        qWarning() << Q_FUNC_INFO << "can not open file : " << arguments.pathIn;
        return 0;
    }

    QVector<TimeWorkOnOff> dateTimeIn;

    QTextStream readStream(&fileIn);

    while(!readStream.atEnd())
    {
        TimeWorkOnOff tWork;

        QString val = readStream.readLine();

        QStringList split_val = val.split(';');

        tWork.dateTime = QDateTime::fromString(split_val[DT], "dd.MM.yyyy hh:mm:ss");

        if(split_val[STATUS].contains("ON", Qt::CaseInsensitive))
    {
            tWork.status = true;
    }
        else if(split_val[STATUS].contains("OFF", Qt::CaseSensitive))
        {
            tWork.status = false;
        }

        dateTimeIn.append(tWork);
    }

    int countTimeOnMSec  = 0;
    int countTimeOffMSec = 0;
    int sizeVec          = dateTimeIn.count();

    if(dateTimeIn[0].status)
    {
        countTimeOffMSec = dateTimeIn[0].dateTime.time().msecsSinceStartOfDay();
    }
    else
    {
        countTimeOnMSec = dateTimeIn[0].dateTime.time().msecsSinceStartOfDay();
    }

    dateTimeIn[0].countSwitch++;

    for(int i = 1; i < sizeVec; i++)
    {
        TimeWorkOnOff& current = dateTimeIn[i];     //текущее состояние
        TimeWorkOnOff& prev    = dateTimeIn[i - 1]; // предыдущее состояние

        prev.timeWorkMSec = prev.dateTime.time().msecsTo(current.dateTime.time());

        current.countSwitch++;

        if(current.status)
        {
            countTimeOffMSec += prev.timeWorkMSec;
            if(i > 1)
        {
                current.countSwitch = dateTimeIn[i - 2].countSwitch + 1;
        }
        }
        else
        {
            countTimeOnMSec += prev.timeWorkMSec;

            if(i > 1)
            {
                current.countSwitch = dateTimeIn[i - 2].countSwitch + 1;
            }
        }
    }

    int lastIndex = sizeVec - 1;

    // время работы последнего срабатывания
    dateTimeIn[ lastIndex ].timeWorkMSec = dateTimeIn [ lastIndex ].dateTime.time().msecsTo(QTime(23, 59, 59));

    if(dateTimeIn[ lastIndex ].status)
    {
        countTimeOnMSec += dateTimeIn[ lastIndex ].timeWorkMSec;
    }
    else
    {
        countTimeOffMSec += dateTimeIn[ lastIndex ].timeWorkMSec;
    }

    saveToFile(arguments, dateTimeIn, countTimeOnMSec, countTimeOffMSec);

    return a.exec();
}
//-----------------------------------------------------------------------------------------------------------------------------------

#include <QStringList>
#include <QFileInfo>
//-----------------------------------------------------------------------------------------------------------------------------------

#include "Arguments.h"
//-----------------------------------------------------------------------------------------------------------------------------------

Arguments::Arguments()
{
    isHelp = false;
}
//-----------------------------------------------------------------------------------------------------------------------------------

/*!
 * \brief                       Конструктор
 * \param argc                  Колличество аргументов командной строки
 * \param argv                  Аргументы командной строки
 */
Arguments::Arguments(int argc, char *argv[])
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
        QString  fileName(fileInfo.baseName() + "__out");

        pathOut = QString("%1/%2.csv").arg(pathDir).arg(fileName);
    }
}
//-----------------------------------------------------------------------------------------------------------------------------------

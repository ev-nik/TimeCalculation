#ifndef TIMEWORK_H
#define TIMEWORK_H

#include <QDateTime>
//-----------------------------------------------------------------------------------------------------------------------------------

struct TimeWorkOnOff
{
    TimeWorkOnOff();
    QDateTime dateTime;      // исходные дата и время
    bool      status;        // сосотояние (On/Off)
    int       timeWorkMSec;  // время работы меду On--Off/Off--On
    int       countSwitch;   // кол-во срабатываний по порядку
};
//-----------------------------------------------------------------------------------------------------------------------------------

#endif // TIMEWORK_H

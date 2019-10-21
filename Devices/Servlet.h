#ifndef CONVERTER_H
#define CONVERTER_H

// This is an abriviated device! Really a subset of the Device structure
#include "Register.h"
#include "Registers.h"
#include "Device.h"

#define SERVLETS_AVAIL   8

typedef struct _servlet_
{
    DeviceTypes     DeviceType;
    DevClass        DeviceClass;
    ThreadStates    Status;
    const char    * Discription;
    Register      * Reference;
    Register      * Output;
    IndicatorBits * SystemStat;
    Boolean         Used;
    unsigned        Id;
    void (*Run)(struct _servlet_ *);
    void (*Clear)(struct _servlet_ *);
}Servlet;

Servlet * NewServlet(void);

#endif
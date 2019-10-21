#include <xc.h>
#include "defs.h"
#include "System.h"
#include "Spi.h"
#include "Device.h"
#include "CD74HCx151.h"
#include "Fops.h"

#define DISCRIPTION_M25LC320 "74151"
#define M25LC320Select PORTBbits.RB10

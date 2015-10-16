#pragma once

#ifndef XCODE
//#include "Platform.h"
#endif
#define ONEWIRE_DS248X

#if defined(ONEWIRE_DS248X)

#include "DS248x.h"

typedef DS248x OneWireDriver;

#else

#error No OneWire implementation defined

#endif

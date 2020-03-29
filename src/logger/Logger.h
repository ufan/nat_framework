#ifndef LOGGER_LOGGER_H
#define LOGGER_LOGGER_H

#if defined FAST_LOGGER_
  #include "FastLogger.h"
#elif defined SIMPLE_LOGGER_
  #include "SimpleLogger.h"
#else
  #include "HeavyLogger.h"
#endif

#endif // LOGGER_LOGGER_H

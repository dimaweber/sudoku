#ifndef SRC_LIB_GLOBAL_H
#define SRC_LIB_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(SRC_LIB_LIBRARY)
#  define SRC_LIBSHARED_EXPORT Q_DECL_EXPORT
#else
#  define SRC_LIBSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // SRC_LIB_GLOBAL_H

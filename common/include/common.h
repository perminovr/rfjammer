#ifndef COMMON_H
#define COMMON_H

#include "config.h"
#include "devstructs.h"
#include "servstructs.h"

#define uptr std::unique_ptr
#define shptr std::shared_ptr
#ifndef mkuptr
# define mkuptr Wt::cpp14::make_unique
#endif
#define mkshptr std::make_shared
#define mv std::move

#define DEBUG 0
#define prgDebug() if (DEBUG) qDebug()

#define scast(t,v) static_cast<t>(v)
#define QueuedUniqueConnection \
	scast(Qt::ConnectionType, (scast(int, Qt::UniqueConnection) | scast(int, Qt::QueuedConnection)))
#define DirectUniqueConnection \
	scast(Qt::ConnectionType, (scast(int, Qt::UniqueConnection) | scast(int, Qt::DirectConnection)))

#define getM(sc) m.sc
#define getMM(sc,f) m.sc.m.f

#endif /* COMMON_H */

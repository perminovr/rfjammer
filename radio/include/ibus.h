#ifndef IBUS_H
#define IBUS_H

#include <fcntl.h>
#include <unistd.h>

/*!
 * @class IBus
 * @brief Интерфейс system шины /dev/
*/
class IBus
{
public:
	IBus(const char *d, int fl = O_RDWR) {
		fd = open(d, fl);
	}
	virtual ~IBus() {
		close(fd);
	}

	virtual int readWrite(const char *msg, char *buf, int sz) const = 0;
	virtual int write(const char *msg, int msz) const = 0;
	virtual int read(char *buf, int bsz) const = 0;

protected:
	int fd;
};

#endif /* IBUS_H */

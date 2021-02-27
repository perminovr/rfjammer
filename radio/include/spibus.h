#ifndef SPIBUS_H
#define SPIBUS_H

#include <ibus.h>

#include <cstdint>
#include <cstring>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

/*!
 * @class SpiBus
 * @brief system spi шина /dev/
*/
class SpiBus : public IBus
{
public:
	SpiBus(const char *dev,
        uint32_t speed = 100*1000, uint8_t mode = SPI_MODE_0,
		uint8_t bits = 8)
	: IBus(dev) {
		int rc = ioctl(fd, SPI_IOC_WR_MODE, &mode);
		rc = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
		rc = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
		mode = 0;
		rc = ioctl(fd, SPI_IOC_WR_LSB_FIRST, &mode);
		(void)rc;
	}
	~SpiBus() = default;

	virtual int readWrite(const char *msg, char *buf, int sz) const {
		static uint8_t bufFF[256] = {0};
		struct spi_ioc_transfer tr;
		if (sz > 256) return -1;
		if (bufFF[0] != 0xFF) { memset(bufFF, 0xFF, 256); }
		bzero(&tr, sizeof(tr));
		tr.tx_buf = (msg)? (unsigned long)msg : (unsigned long)bufFF;
		tr.rx_buf = (unsigned long)buf;
		tr.len = sz;
		int rc = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
		return (rc >= 0)? sz : -1;
	}
	virtual int write(const char *msg, int msz) const override {
		return readWrite(msg, 0, msz);
	}
	virtual int read(char *buf, int bsz) const override {
		return readWrite(0, buf, bsz);
	}
};

#endif /* SPIBUS_H */

#ifndef GPIODRV_H
#define GPIODRV_H

#include <QFile>
#include <fcntl.h>
#include <unistd.h>

/*!
 * @class LinuxGpio
 * @brief работа с gpio через linux /sys/class/gpio
*/
class LinuxGpio {
public:
	enum Direction {
		GpioInput,
		GpioOutput,
	};

	/*!
	* @fn Gpio::getGpioNum
	* @brief: перевод порта и номера ножки в значение gpio linux
	*/
	static int getGpioNum(int port, int pin) { return port*32+pin; }

	LinuxGpio(int num, Direction dir, bool inverse = false) {
		this->inverse = inverse;
		this->num = QString::number(num);
		this->value = QStringLiteral("/sys/class/gpio/gpio") + this->num + QStringLiteral("/value");
		if ( !_isExported() )
			_export();
		_direction(dir);
	}
	~LinuxGpio() = default;

protected:
	QString num;	//!< номер gpio linux
	QString value;	//!< имя файла для чтения/записи значения
	bool inverse;	//!< инвертирование сигнала как на запись, так и на чтение

	inline bool _isExported() const {
		return QFile(value).exists();
	}
	void _export() const {
		QFile f(QStringLiteral("/sys/class/gpio/export"));
		if (f.open(QFile::WriteOnly)) {
			f.write(num.toLocal8Bit(), num.length());
		}
	}
	void _unexport() const {
		QFile f(QStringLiteral("/sys/class/gpio/unexport"));
		if (f.open(QFile::WriteOnly)) {
			f.write(num.toLocal8Bit(), num.length());
		}
	}
	void _direction(Direction dir) const {
		QString fdir = QStringLiteral("/sys/class/gpio/gpio") + this->num + QStringLiteral("/direction");
		QFile f(fdir);
		if (f.open(QFile::WriteOnly)) {
			const char *sdir = (dir == Direction::GpioInput)? "in" : "out";
			f.write(sdir, strlen(sdir));
		}
	}
};

/*!
 * @class LinuxGpioIn
 * @brief работа с input gpio через linux
*/
class LinuxGpioIn : public LinuxGpio {
public:
	/*!
	* @fn LinuxGpioIn::getVal
	* @brief: получить значения сигнала
	*/
	bool getVal(bool &val) const {
		QFile f(value);
		if (f.open(QFile::ReadOnly)) {
			auto a = f.read(1);
			if (a.size() == 1) {
                val = (a[0] == '1') ^ this->inverse;
				return true;
			}
		}
		return false;
	}
	enum Edge {Rising, Falling};
	/*!
	* @fn LinuxGpioIn::prepForEpoll
	* @brief подготовить дескриптор для epoll в целях получения прерывания
	*/
	int prepForEpoll(Edge edge) {
		if (this->fd == -1) {
			this->fd = open(this->value.toLocal8Bit().constData(), O_RDONLY);
		}
		auto ef = QStringLiteral("/sys/class/gpio/gpio") + this->num + QStringLiteral("/edge");
		QFile f(ef);
		if (f.open(QFile::WriteOnly)) {
			f.write(((edge == Edge::Rising)? "rising" : "falling"));
			f.close();
		}
		return this->fd;
	}
	inline int getFdForEpoll() const { return this->fd; }
	LinuxGpioIn(int num, bool inverse = false) : LinuxGpio(num, LinuxGpio::GpioInput, inverse), fd(-1) {}
	~LinuxGpioIn() {
		close(this->fd);
	}

protected:
	int fd;
};

/*!
 * @class LinuxGpioOut
 * @brief работа с output gpio через linux
*/
class LinuxGpioOut : public LinuxGpio {
public:
	LinuxGpioOut(int num, bool inverse = false) : LinuxGpio(num, LinuxGpio::Direction::GpioOutput, inverse) {
		this->prevVal = false;
	}
	~LinuxGpioOut() = default;

	/*!
	* @fn GpioOut::setVal
	* @brief установить сигнал
	*/
	bool setVal(bool val) {
		this->prevVal = val;
		if (this->inverse)
			val = !val;
		QFile f(this->value);
		if (f.open(QFile::WriteOnly)) {
			auto res = f.write((val)? "1" : "0", 1);
			return (res == 1)? true : false;
		}
		return false;
	}
	/*!
	* @fn GpioOut::setVal
	* @brief переключить сигнал на инвертированный
	*/
	bool toggle() {
		return setVal(!this->prevVal);
	}

protected:
	bool prevVal;	//!< сохранение значения до изменения
};

#endif // GPIODRV_H

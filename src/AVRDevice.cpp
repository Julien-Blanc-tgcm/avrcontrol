#include "AVRDevice.hpp"

#include "marantzuart.hpp"

#include <QDebug>
#include <QTcpSocket>

#include <cstring>

namespace eu
{
namespace tgcm
{
namespace avrremote
{
class AvrDevicePrivate
{
	Q_DISABLE_COPY(AvrDevicePrivate)
	Q_DECLARE_PUBLIC(AvrDevice)
	AvrDevice* q_ptr;

	explicit AvrDevicePrivate(AvrDevice* q) : q_ptr{q}, parser_(*this)
	{
		sources_.push_back(QString::fromUtf8(toCStr(avrcommand::Source::Phono)));
		sources_.push_back(QString::fromUtf8(toCStr(avrcommand::Source::CD)));
		sources_.push_back(QString::fromUtf8(toCStr(avrcommand::Source::DVD)));
		sources_.push_back(QString::fromUtf8(toCStr(avrcommand::Source::Bluray)));
		sources_.push_back(QString::fromUtf8(toCStr(avrcommand::Source::TV)));
		sources_.push_back(QString::fromUtf8(toCStr(avrcommand::Source::Cable_Sat)));
		sources_.push_back(QString::fromUtf8(toCStr(avrcommand::Source::Multimedia)));
		sources_.push_back(QString::fromUtf8(toCStr(avrcommand::Source::Game)));
		sources_.push_back(QString::fromUtf8(toCStr(avrcommand::Source::Tuner)));
		sources_.push_back(QString::fromUtf8(toCStr(avrcommand::Source::HdRadio)));
		sources_.push_back(QString::fromUtf8(toCStr(avrcommand::Source::Aux1)));
		sources_.push_back(QString::fromUtf8(toCStr(avrcommand::Source::Aux2)));
		sources_.push_back(QString::fromUtf8(toCStr(avrcommand::Source::Aux3)));
		sources_.push_back(QString::fromUtf8(toCStr(avrcommand::Source::Aux4)));
		sources_.push_back(QString::fromUtf8(toCStr(avrcommand::Source::Aux5)));
		sources_.push_back(QString::fromUtf8(toCStr(avrcommand::Source::Aux6)));
		sources_.push_back(QString::fromUtf8(toCStr(avrcommand::Source::Aux7)));
		sources_.push_back(QString::fromUtf8(toCStr(avrcommand::Source::Network)));
		sources_.push_back(QString::fromUtf8(toCStr(avrcommand::Source::Bluetooth)));
	}

	QString name_;

	QString address_;

	int connectionStatus_{};

	RemoteIntProperty volume_{};

	RemoteStringProperty currentSource_;
	avrcommand::Source currentSourceIndex_{};

	QStringList sources_;

	int minVolume_{};

	RemoteIntProperty maxVolume_{};

	QTcpSocket* socket_ = nullptr;

	bool standby_{};

	bool initPhase_{};

	avrcommand::MarantzUartParser<AvrDevicePrivate> parser_;

  public: // MarantzUartParser interface must be private
	void masterVolumeChanged(int volume);
	void maxVolumeChanged(int maxVolume);
	void powerChanged(bool power);
	void sourceChanged(avrcommand::Source source);

  private:
	void setVolume_(int volume);
	void setStandby_(bool standby);
};

AvrDevice::AvrDevice(QObject* parent) : QObject(parent), d_ptr(new AvrDevicePrivate(this))
{
}

AvrDevice::~AvrDevice() = default;

const QString& AvrDevice::name() const
{
	return d_ptr->name_;
}

void AvrDevice::setName(const QString& newName)
{
	if (d_ptr->name_ == newName)
		return;
	d_ptr->name_ = newName;
	emit nameChanged();
}

const QString &AvrDevice::address() const
{
	return d_ptr->address_;
}

void AvrDevice::setAddress(const QString &newAddress)
{
	if (d_ptr->address_ == newAddress)
		return;
	d_ptr->address_ = newAddress;
	emit addressChanged();
}

int AvrDevice::connectionStatus() const
{
	return d_ptr->connectionStatus_;
}

void AvrDevice::setConnectionStatus(int newConnectionStatus)
{
	if (d_ptr->connectionStatus_ == newConnectionStatus)
		return;
	d_ptr->connectionStatus_ = newConnectionStatus;
	emit connectionStatusChanged();
}

RemoteIntProperty AvrDevice::volume() const
{
	return d_ptr->volume_;
}

void AvrDevicePrivate::setVolume_(int newVolume)
{
	volume_.setValue(newVolume);
	volume_.setState(RemoteProperty::UpToDate);
	emit q_ptr->volumeChanged();
}

void AvrDevicePrivate::setStandby_(bool newStandby)
{
	standby_ = newStandby;
	emit q_ptr->standbyChanged();
}

RemoteStringProperty AvrDevice::currentSource() const
{
	return d_ptr->currentSource_;
}

void AvrDevice::setCurrentSource(const QString &newCurrentSource)
{
	d_ptr->currentSource_.setState(RemoteProperty::UpToDate);
	d_ptr->currentSource_.setValue(newCurrentSource);
	emit currentSourceChanged();
	emit currentSourceIndexChanged();
}

const QStringList &AvrDevice::sources() const
{
	return d_ptr->sources_;
}

void AvrDevice::setSources(const QStringList &newSources)
{
	if (d_ptr->sources_ == newSources)
		return;
	d_ptr->sources_ = newSources;
	emit sourcesChanged();
}

int AvrDevice::minVolume() const
{
	return d_ptr->minVolume_;
}

void AvrDevice::setMinVolume(int newMinVolume)
{
	if (d_ptr->minVolume_ == newMinVolume)
		return;
	d_ptr->minVolume_ = newMinVolume;
	emit minVolumeChanged();
}

RemoteIntProperty AvrDevice::maxVolume() const
{
	return d_ptr->maxVolume_;
}

void AvrDevice::setMaxVolume(int newMaxVolume)
{
	qDebug() << "Set max volume " << newMaxVolume;
	d_ptr->maxVolume_.setState(RemoteProperty::UpToDate);
	d_ptr->maxVolume_.setValue(newMaxVolume);
	emit maxVolumeChanged();
}

void AvrDevice::connectToDevice()
{
	if (d_ptr->socket_ == nullptr)
	{
		d_ptr->socket_ = new QTcpSocket(this);
		d_ptr->socket_->setSocketOption(QAbstractSocket::LowDelayOption, 1);
	}
	else
	{
		d_ptr->socket_->close();
	}
	connect(d_ptr->socket_, &QTcpSocket::connected, this, &AvrDevice::handleConnected_);
	connect(d_ptr->socket_, &QTcpSocket::readyRead, this, &AvrDevice::handleDataAvailable_);
	d_ptr->socket_->connectToHost(address(), 23);
	setConnectionStatus(Connecting);
}

void AvrDevice::handleConnected_()
{
	setConnectionStatus(Connected);
	d_ptr->initPhase_ = true;
	d_ptr->volume_.setState(RemoteProperty::Reading);
	d_ptr->socket_->write(avrcommand::queryPowerStatus);
}

void AvrDevice::handleDataAvailable_()
{
	char data[1024];
	auto nbRead = d_ptr->socket_->read(data, sizeof(data));
	qDebug() << "Data read from socket: " << nbRead;
	if (nbRead > 0)
	{
		qDebug() << QByteArray(data, nbRead);
		interpretResponse_(data, nbRead);
	}
}

bool AvrDevice::standby() const
{
	return d_ptr->standby_;
}

void AvrDevice::interpretResponse_(char const* data, int len)
{
	std::size_t res;
	do
	{
		qDebug() << "Will parse " << QByteArray(data, len);
		res = d_ptr->parser_.parse(data, len);
		data += res;
		len -= res;
	} while (res > 0 && len > 0);
}

void AvrDevicePrivate::masterVolumeChanged(int volume)
{
	setVolume_(volume);
	if (initPhase_)
	{
		socket_->write(avrcommand::querySourceInput);
	}
}

void AvrDevice::volumeUp()
{
	if (d_ptr->connectionStatus_ == Connected)
	{
		d_ptr->socket_->write(avrcommand::masterVolumeUpCommand);
	}
}

void AvrDevice::volumeDown()
{
	if (d_ptr->connectionStatus_ == Connected)
	{
		d_ptr->socket_->write(avrcommand::masterVolumeDownCommand);
	}
}

void AvrDevice::setVolume(int volume)
{
	if (volume >= 1000 || volume < 0)
		return; // invalid volume
	if (d_ptr->connectionStatus_ == Connected)
	{
		std::array<char, 6> d;
		auto res = avrcommand::setMasterVolume(volume, d);
		d_ptr->socket_->write(res.data(), res.size());
	}
}

void AvrDevice::setPowerStandby(bool standby)
{
	if (d_ptr->connectionStatus_ == Connected)
	{
		if (standby)
			d_ptr->socket_->write(avrcommand::powerOffCommand);
		else
			d_ptr->socket_->write(avrcommand::powerOnCommand);
	}
}

void AvrDevice::setSource(int sourceIndex)
{
	if (connectionStatus() == Connected)
	{
		auto cmd = avrcommand::setSource(static_cast<avrcommand::Source>(sourceIndex));
		d_ptr->socket_->write(cmd.data(), cmd.size());
	}
}

void AvrDevicePrivate::maxVolumeChanged(int maxVolume)
{
	q_ptr->setMaxVolume(maxVolume);
}

void AvrDevicePrivate::powerChanged(bool power)
{
	setStandby_(!power);
	if (initPhase_)
	{
		int ret = socket_->write(avrcommand::queryMasterVolume);
		qDebug() << ret;
	}
}

void AvrDevicePrivate::sourceChanged(avrcommand::Source source)
{
	currentSourceIndex_ = source;
	q_ptr->setCurrentSource(toCStr(source));
}

int AvrDevice::currentSourceIndex() const
{
	return static_cast<int>(d_ptr->currentSourceIndex_);
}

} // namespace avrremote
} // namespace tgcm
} // namespace eu

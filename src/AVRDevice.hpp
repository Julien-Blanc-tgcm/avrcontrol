#ifndef AVRDEVICE_HPP
#define AVRDEVICE_HPP

#include <QObject>
#include <QTcpSocket>

#include "RemoteProperty.hpp"

namespace eu
{
namespace tgcm
{
namespace avrremote
{

class AvrDevicePrivate;

class AvrDevice : public QObject
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(AvrDevice)

	QScopedPointer<AvrDevicePrivate> const d_ptr;

  public:
	enum ConnectionStatus
	{
		Unconnected,
		Connecting,
		Connected
	};
	Q_ENUM(ConnectionStatus)

	explicit AvrDevice(QObject *parent = nullptr);
	~AvrDevice() override;

	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(QString address READ address WRITE setAddress NOTIFY addressChanged)
	Q_PROPERTY(int connectionStatus READ connectionStatus WRITE setConnectionStatus NOTIFY connectionStatusChanged)

	Q_PROPERTY(QStringList sources READ sources WRITE setSources NOTIFY sourcesChanged)
	Q_PROPERTY(eu::tgcm::avrremote::RemoteStringProperty currentSource READ currentSource NOTIFY currentSourceChanged)
	Q_PROPERTY(int currentSourceIndex READ currentSourceIndex NOTIFY currentSourceIndexChanged)

	Q_PROPERTY(eu::tgcm::avrremote::RemoteIntProperty volume READ volume NOTIFY volumeChanged)
	Q_PROPERTY(int minVolume READ minVolume WRITE setMinVolume NOTIFY minVolumeChanged)
	Q_PROPERTY(eu::tgcm::avrremote::RemoteIntProperty maxVolume READ maxVolume NOTIFY maxVolumeChanged)

	Q_PROPERTY(bool standby READ standby NOTIFY standbyChanged)

	const QString &name() const;
	void setName(const QString &newName);

	const QString &address() const;
	void setAddress(const QString &newAddress);

	int connectionStatus() const;
	void setConnectionStatus(int newConnectionStatus);

	RemoteIntProperty volume() const;

	RemoteStringProperty currentSource() const;
	void setCurrentSource(const QString &newCurrentSource);

	const QStringList &sources() const;
	void setSources(const QStringList &newSources);

	int minVolume() const;
	void setMinVolume(int newMinVolume);

	RemoteIntProperty maxVolume() const;
	void setMaxVolume(int newMaxVolume);

	bool standby() const;

	Q_INVOKABLE void volumeUp();
	Q_INVOKABLE void volumeDown();
	Q_INVOKABLE void setVolume(int volume);
	Q_INVOKABLE void setSource(int sourceIndex);

	/**
	 * Sets the current power standby, for main zone. True to set to standby mode
	 */
	Q_INVOKABLE void setPowerStandby(bool standby);

	int currentSourceIndex() const;

  public slots:
	void connectToDevice();

  signals:

	void nameChanged();
	void addressChanged();
	void connectionStatusChanged();
	void volumeChanged();
	void currentSourceChanged();
	void sourcesChanged();
	void minVolumeChanged();
	void maxVolumeChanged();

	void standbyChanged();

	void currentSourceIndexChanged();

  private:
	void interpretResponse_(char const* data, int len);

  private slots:
	void handleConnected_();
	void handleDataAvailable_();
};

} // namespace avrremote
} // namespace tgcm
} // namespace eu
#endif // AVRDEVICE_HPP

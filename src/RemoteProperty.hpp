#ifndef EU_TGCM_AVRREMOTE_REMOTEPROPERTY_H
#define EU_TGCM_AVRREMOTE_REMOTEPROPERTY_H

#include <QObject>

namespace eu
{
namespace tgcm
{
namespace avrremote
{

/**
 * This class is only here to hold the enum
 */
class RemoteProperty : public QObject
{
	Q_OBJECT

  public:
	RemoteProperty(QObject* parent = nullptr) : QObject(parent)
	{
	}
	enum State
	{
		Unknown, /**< The property has never been read successfully */
		Reading, /**< The property is unknown, but is currently being read, the answer should arrive soon */
		Refreshing, /**< The property has been read, and is now being refreshed in case the value changed */
		UpToDate, /**< The property is up to date */
		OutOfDate, /**< The property has been read, but for some reason is out of date. Its value may have
		                changed since the last time it was read */
		ReadError /**< The property has never been read, and the read retrieved an error */
	};
	Q_ENUM(State)
};

class RemoteIntProperty
{
	Q_GADGET
	Q_PROPERTY(eu::tgcm::avrremote::RemoteProperty::State state READ state)
	Q_PROPERTY(int value READ value)

	RemoteProperty::State s_;

	int v_;

  public:
	RemoteProperty::State state() const;

	int value() const;

	RemoteIntProperty();

	RemoteIntProperty(RemoteProperty::State s, int v);
	RemoteIntProperty(RemoteIntProperty&&) = default;
	RemoteIntProperty(RemoteIntProperty const&) = default;
	RemoteIntProperty& operator=(RemoteIntProperty&&) = default;
	RemoteIntProperty& operator=(RemoteIntProperty const&) = default;
	~RemoteIntProperty() noexcept = default;

	void setValue(int v);
	void setState(RemoteProperty::State s);
};

class RemoteStringProperty
{
	Q_GADGET
	Q_PROPERTY(eu::tgcm::avrremote::RemoteProperty::State state READ state)
	Q_PROPERTY(QString value READ value)

	RemoteProperty::State s_;

	QString v_;

  public:
	RemoteProperty::State state() const;

	QString value() const;

	RemoteStringProperty();

	RemoteStringProperty(RemoteProperty::State s, QString v);
	RemoteStringProperty(RemoteStringProperty&&) = default;
	RemoteStringProperty(RemoteStringProperty const&) = default;
	RemoteStringProperty& operator=(RemoteStringProperty&&) = default;
	RemoteStringProperty& operator=(RemoteStringProperty const&) = default;
	~RemoteStringProperty() noexcept = default;

	void setValue(QString v);
	void setState(RemoteProperty::State s);
};

} // namespace avrremote
} // namespace tgcm
} // namespace eu

Q_DECLARE_METATYPE(eu::tgcm::avrremote::RemoteIntProperty)
Q_DECLARE_METATYPE(eu::tgcm::avrremote::RemoteStringProperty)

#endif // EU_TGCM_AVRREMOTE_REMOTEPROPERTY_H

#include "RemoteProperty.hpp"

namespace eu
{
namespace tgcm
{
namespace avrremote
{

RemoteIntProperty::RemoteIntProperty() : s_(RemoteProperty::State::Unknown), v_(0)
{
}

RemoteIntProperty::RemoteIntProperty(RemoteProperty::State s, int v) : s_(s), v_(v)
{
}

RemoteProperty::State RemoteIntProperty::state() const
{
	return s_;
}

int RemoteIntProperty::value() const
{
	return v_;
}

void RemoteIntProperty::setState(RemoteProperty::State s)
{
	s_ = s;
}

void RemoteIntProperty::setValue(int v)
{
	v_ = v;
}

RemoteStringProperty::RemoteStringProperty() : s_(RemoteProperty::State::Unknown), v_()
{
}

RemoteStringProperty::RemoteStringProperty(RemoteProperty::State s, QString v) : s_(s), v_(v)
{
}

RemoteProperty::State RemoteStringProperty::state() const
{
	return s_;
}

QString RemoteStringProperty::value() const
{
	return v_;
}

void RemoteStringProperty::setState(RemoteProperty::State s)
{
	s_ = s;
}

void RemoteStringProperty::setValue(QString v)
{
	v_ = v;
}


}
}
}

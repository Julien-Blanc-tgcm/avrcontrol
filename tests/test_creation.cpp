#include <QTest>

#include "AvrDevice.hpp"
#include "marantzuart.hpp"

using namespace eu::tgcm::avrcommand;

class TestCreation : public QObject
{
	Q_OBJECT
  private slots:
	void testCreation()
	{
		eu::tgcm::avrremote::AvrDevice d;
		(void)d;
	}
};

QTEST_MAIN(TestCreation)
#include "test_creation.moc"

#include <QTest>

#include "marantzuart.hpp"

using namespace eu::tgcm::avrcommand;

class TestCommands : public QObject
{
	Q_OBJECT
  private slots:
	void testMasterVolume()
	{
		std::array<char, 6> data{0};
		auto res = setMasterVolume(300, data);
		auto str = QString::fromUtf8(res.data(), res.size());
		QVERIFY(str == "MV30\n");
	}

	void testMasterVolume2()
	{
		std::array<char, 6> data{0};
		auto res = setMasterVolume(50, data);
		auto str = QString::fromUtf8(res.data(), res.size());
		QVERIFY(str == "MV05\n");
	}

	void testMasterVolume3()
	{
		std::array<char, 6> data{0};
		auto res = setMasterVolume(450, data);
		auto str = QString::fromUtf8(res.data(), res.size());
		QVERIFY(str == "MV45\n");
	}
	void testSource()
	{
		testSourceHelper_("SISAT/CBL\n", Source::Cable_Sat);
		testSourceHelper_("SIPHONO\n", Source::Phono);
		testSourceHelper_("SICD\n", Source::CD);
		testSourceHelper_("SIPHONO\n", Source::Phono);
		testSourceHelper_("SIDVD\n", Source::DVD);
		testSourceHelper_("SIBD\n", Source::Bluray);
		testSourceHelper_("SITV\n", Source::TV);
		testSourceHelper_("SIMPLAY\n", Source::Multimedia);
		testSourceHelper_("SIGAME\n", Source::Game);
		testSourceHelper_("SITUNER\n", Source::Tuner);
		testSourceHelper_("SIHDRADIO\n", Source::HdRadio);
		testSourceHelper_("SIAUX1\n", Source::Aux1);
		testSourceHelper_("SIAUX2\n", Source::Aux2);
		testSourceHelper_("SIAUX3\n", Source::Aux3);
		testSourceHelper_("SIAUX4\n", Source::Aux4);
		testSourceHelper_("SIAUX5\n", Source::Aux5);
		testSourceHelper_("SIAUX6\n", Source::Aux6);
		testSourceHelper_("SIAUX7\n", Source::Aux7);
		testSourceHelper_("SINET\n", Source::Network);
		testSourceHelper_("SIBT\n", Source::Bluetooth);
	}

  private:
	void testSourceHelper_(std::string_view s, Source c)
	{
		std::string_view res = setSource(c);
		QVERIFY(s == res);
	}
};

QTEST_MAIN(TestCommands)
#include "test_commands.moc"


#include <QTest>

#include "marantzuart.hpp"

using namespace eu::tgcm::avrcommand;

class ParserCallbacks
{
  public:
	int masterVolume = -1;
	int maxVolume = -1;
	bool powerStatus = false;
	Source source;

	void masterVolumeChanged(int newMasterVolume)
	{
		masterVolume = newMasterVolume;
	}

	void maxVolumeChanged(int newMaxVolume)
	{
		maxVolume = newMaxVolume;
	}

	void powerChanged(bool newPower)
	{
		powerStatus = newPower;
	}

	void sourceChanged(Source newSource)
	{
		source = newSource;
	}
};


class TestParser : public QObject
{
	Q_OBJECT
  private slots:
	void testVolume()
	{
		ParserCallbacks c;
		MarantzUartParser<ParserCallbacks> p(c);

		char const line[] = "MV30\rMVMAX 655\r";

		std::size_t res = 0;
		std::size_t total = 0;
		char const* cur = line;
		do
		{
			res = p.parse(cur, strlen(line) - total);
			total += res;
			cur += res;
		} while (res > 0);

		QVERIFY(c.masterVolume == 300);
		QVERIFY(c.maxVolume == 655);
		QVERIFY(total == strlen(line));
	}

	void testVolume2()
	{
		ParserCallbacks c;
		MarantzUartParser<ParserCallbacks> p(c);

		char const line[] = "MV30\rMVMAX 65\r";

		std::size_t res = 0;
		std::size_t total = 0;
		res = p.parse(line, strlen(line));
		total += res;
		res = p.parse(line + total, strlen(line) - total);
		total += res;
		QVERIFY(c.masterVolume == 300);
		QVERIFY(c.maxVolume == 650);
		QVERIFY(total == strlen(line));
	}

	void testPower()
	{
		ParserCallbacks c;
		MarantzUartParser<ParserCallbacks> p(c);

		char const line[] = "PWON\r";

		std::size_t res = p.parse(line, strlen(line));
		QVERIFY(c.powerStatus);
		QVERIFY(res == 5u);
		char line2[] = "PWSTAND";
		res = p.parse(line2, strlen(line2));
		QVERIFY(res == strlen(line2));
		QVERIFY(c.powerStatus);
		char line3[] = "BY\r";
		res = p.parse(line3, strlen(line3));
		QVERIFY(!c.powerStatus); // power update
		QVERIFY(res == 3);
	}

	void testUnknown()
	{
		char const* line = "AB45\rBA12\rPWON\r";
		ParserCallbacks c;
		MarantzUartParser<ParserCallbacks> p(c);
		c.powerStatus = false;
		std::size_t total = 0;
		auto res = p.parse(line, strlen(line));
		QVERIFY(res == 5);
		total += res;
		res = p.parse(line + total, strlen(line + total));
		total += res;
		QVERIFY(res == 5);
		QVERIFY(total == 10);
		res = p.parse(line + total, strlen(line + total));
		QVERIFY(res == 5);
		QVERIFY(c.powerStatus);
	}

	void testSource()
	{
		char const* line = "SISAT/CBL\r";
		ParserCallbacks c;
		MarantzUartParser<ParserCallbacks> p(c);
		c.source = Source::Aux1;
		testSourceHelper_(c, p, line, Source::Cable_Sat);
		testSourceHelper_(c, p, "SIPHONO\r", Source::Phono);
		testSourceHelper_(c, p, "SICD\r", Source::CD);
		testSourceHelper_(c, p, "SIPHONO\r", Source::Phono);
		testSourceHelper_(c, p, "SIDVD\r", Source::DVD);
		testSourceHelper_(c, p, "SIBD\r", Source::Bluray);
		testSourceHelper_(c, p, "SITV\r", Source::TV);
		testSourceHelper_(c, p, "SIMPLAY\r", Source::Multimedia);
		testSourceHelper_(c, p, "SIGAME\r", Source::Game);
		testSourceHelper_(c, p, "SITUNER\r", Source::Tuner);
		testSourceHelper_(c, p, "SIHDRADIO\r", Source::HdRadio);
		testSourceHelper_(c, p, "SIAUX1\r", Source::Aux1);
		testSourceHelper_(c, p, "SIAUX2\r", Source::Aux2);
		testSourceHelper_(c, p, "SIAUX3\r", Source::Aux3);
		testSourceHelper_(c, p, "SIAUX4\r", Source::Aux4);
		testSourceHelper_(c, p, "SIAUX5\r", Source::Aux5);
		testSourceHelper_(c, p, "SIAUX6\r", Source::Aux6);
		testSourceHelper_(c, p, "SIAUX7\r", Source::Aux7);
		testSourceHelper_(c, p, "SINET\r", Source::Network);
		testSourceHelper_(c, p, "SIBT\r", Source::Bluetooth);

	}

  private:
	void testSourceHelper_(ParserCallbacks& c,
	                       MarantzUartParser<ParserCallbacks>& p,
	                       char const* line,
	                       Source expectedResult)
	{
		auto res = p.parse(line, strlen(line));
		QVERIFY(res == strlen(line));
		QVERIFY(c.source == expectedResult);
	}
};

QTEST_MAIN(TestParser)
#include "test_parser.moc"


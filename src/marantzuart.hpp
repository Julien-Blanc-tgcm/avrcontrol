#ifndef JBC_MARANTZUART_H
#define JBC_MARANTZUART_H

#include <cassert>
#include <cctype>
#include <cstdint>

#include <string_view>

namespace eu
{
namespace tgcm
{
namespace avrcommand
{

enum class Source
{
	Phono,
	CD,
	DVD,
	Bluray,
	TV,
	Cable_Sat,
	Multimedia,
	Game,
	Tuner,
	HdRadio,
	Aux1,
	Aux2,
	Aux3,
	Aux4,
	Aux5,
	Aux6,
	Aux7,
	Network,
	Bluetooth
};

constexpr char const* toCStr(Source source)
{
	switch (source)
	{
		case Source::Phono:
			return "Phono";
		case Source::CD:
			return "CD";
		case Source::DVD:
			return "DVD";
		case Source::Bluray:
			return "Bluray";
		case Source::TV:
			return "TV";
		case Source::Cable_Sat:
			return "Cable_Sat";
		case Source::Multimedia:
			return "Multimedia";
		case Source::Game:
			return "Game";
		case Source::Tuner:
			return "Tuner";
		case Source::HdRadio:
			return "HdRadio";
		case Source::Aux1:
			return "Aux1";
		case Source::Aux2:
			return "Aux2";
		case Source::Aux3:
			return "Aux3";
		case Source::Aux4:
			return "Aux4";
		case Source::Aux5:
			return "Aux5";
		case Source::Aux6:
			return "Aux6";
		case Source::Aux7:
			return "Aux7";
		case Source::Network:
			return "Network";
		case Source::Bluetooth:
			return "Bluetooth";
	}
	return "";
}

/**
 * This class handles the parsing of marantz uart replies, and generate proper events
 * accordingly
 */
template <typename Handler>
class MarantzUartParser
{
  public:
	explicit MarantzUartParser(Handler& h) : h_(h)
	{
	}

	~MarantzUartParser() noexcept = default;

	MarantzUartParser(MarantzUartParser const&) = default;
	MarantzUartParser(MarantzUartParser&&) = default;
	MarantzUartParser& operator=(MarantzUartParser const&) = delete;
	MarantzUartParser& operator=(MarantzUartParser&&) = delete;

	std::size_t parse(char const* data, std::size_t size)
	{
		switch (s_)
		{
			case InternalState::Begin:
				return parseBegin_(data, size);
			case InternalState::Invalid: {
				return parseInvalid_(data, size);
			}
			case InternalState::Parse_M:
				return parseM_(data, size);
			case InternalState::Parse_MV:
				return parseMV_(data, size);
			case InternalState::Parse_MVM:
				return parseMVM_(data, size);
			case InternalState::Parse_MVMA:
				return parseMVMA_(data, size);
			case InternalState::Parse_MVMAX:
				return parseMVMAX_(data, size);
			case InternalState::Parse_MVMAX2:
				return parseMVMAX2_(data, size);
			case InternalState::Parse_P:
				return parseP_(data, size);
			case InternalState::Parse_PW:
				return parsePW_(data, size);
			case InternalState::Parse_PWO:
				return parsePWO_(data, size);
			case InternalState::Parse_PWON:
				return parsePWON_(data, size);
			case InternalState::Parse_PWS:
				return parsePWS_(data, size);
			case InternalState::Parse_S:
				return parseS_(data, size);
			case InternalState::Parse_SI:
				return parseSI_(data, size);
		}
		assert(false && "parser in a very bad state");
		return size; // should not appen !!!
	}

  private:
	enum class InternalState
	{
		Begin, /**< Initial state, wait for a reply */
		Invalid, /**< Invalid state, wait for a '\r' to go back to begin */
		Parse_M,
		Parse_MV,
		Parse_MVM,
		Parse_MVMA,
		Parse_MVMAX,
		Parse_MVMAX2,
		Parse_P,
		Parse_PW,
		Parse_PWO,
		Parse_PWON,
		Parse_PWS,
		Parse_S,
		Parse_SI,
	};

	Handler& h_;

	InternalState s_ = InternalState::Begin;

	/**
	 * Stores the last value parsed, or an index, depending on the current parser state
	 */
	int lastValue_ = 0;

	std::size_t parseBegin_(char const* data, std::size_t size)
	{
		lastValue_ = 0; // always reinitialize last value at begin
		if (size == 0)
			return 0;
		if (data[0] == 'M')
			return parseM_(data + 1, size - 1) + 1;
		if (data[0] == 'P')
			return parseP_(data + 1, size - 1) + 1;
		if (data[0] == 'S')
			return parseS_(data + 1, size - 1) + 1;
		// else need to implement
		return parseInvalid_(data, size);
	}

#define PARSE_SINGLE_EXPECTED_CHAR(funcName, state, expectedChar, nextFunc) \
	std::size_t funcName(char const* data, std::size_t size) \
	{ \
		if (size == 0) \
		{ \
			s_ = state; \
			return 0; \
		}\
		if (data[0] == expectedChar) \
			return nextFunc(data + 1, size - 1) + 1; \
		return parseInvalid_(data, size); \
	}

	PARSE_SINGLE_EXPECTED_CHAR(parseM_, InternalState::Parse_M, 'V', parseMV_);
	PARSE_SINGLE_EXPECTED_CHAR(parseMVM_, InternalState::Parse_MVM, 'A', parseMVMA_);
	PARSE_SINGLE_EXPECTED_CHAR(parseMVMA_, InternalState::Parse_MVMA, 'X', parseMVMAX_);
	PARSE_SINGLE_EXPECTED_CHAR(parseMVMAX_, InternalState::Parse_MVMAX, ' ', parseMVMAX2_);

	std::size_t parseMVMAX2_(char const* data, std::size_t size)
	{
		if (size == 0)
		{
			s_ = InternalState::Parse_MVMAX;
			return 0;
		}
		std::size_t i = 0;
		while (i < size && std::isdigit(data[i]))
		{
			lastValue_ = lastValue_ * 10;
			lastValue_ += data[i] - '0';
			i += 1;
		}
		if (i < size)
		{
			if (data[i] == '\r') // found the '\r'
			{
				s_ = InternalState::Begin;
				if (lastValue_ < 100)
					lastValue_ *= 10;
				h_.maxVolumeChanged(lastValue_);
				return i + 1;
			}
			return parseInvalid_(data + 1, size - 1) + 1;
		}
		s_ = InternalState::Parse_MVMAX;
		return i;
	}

	std::size_t parseMV_(char const* data, std::size_t size)
	{
		std::size_t i = 0;
		if (size == 0)
		{
			s_ = InternalState::Parse_MV;
			return 0;
		}
		if (lastValue_ == 0 && data[i] == 'M')
			return parseMVM_(data + 1, size - 1) + 1;
		while (i < size && std::isdigit(data[i]))
		{
			lastValue_ = lastValue_ * 10;
			lastValue_ += (data[i] - '0');
			i += 1;
		}
		if (i < size)
		{
			if (data[i] == '\r') // found the '\r'
			{
				if (lastValue_ < 100)
					lastValue_ *= 10;
				h_.masterVolumeChanged(lastValue_);
				s_ = InternalState::Begin;
				return i + 1;
			}
			// else consume everything, did not understand command. Can skip char
			// because it is not a \r
			return parseInvalid_(data + i + 1, size - i - 1) + i + 1;
		}
		// else needs more data
		s_ = InternalState::Parse_MV;
		return i;
	}

	std::size_t parseInvalid_(char const* data, std::size_t size)
	{
		std::size_t i = 0;
		while (i < size && data[i] != '\r')
			i += 1;
		if (i < size) // '\r' found
		{
			s_ = InternalState::Begin;
			i += 1; // consume the '\r'
		}
		else
			s_ = InternalState::Invalid;
		return i; // in all cases, we consumed i chars
	}

	std::size_t parseP_(char const* data, std::size_t size)
	{
		if (size == 0)
		{
			s_ = InternalState::Parse_P;
			return 0;
		}
		if (data[0] == 'W')
			return parsePW_(data + 1, size - 1) + 1;
		return parseInvalid_(data, size);
	}

	std::size_t parsePW_(char const* data, std::size_t size)
	{
		if (size == 0)
		{
			s_ = InternalState::Parse_PW;
			return 0;
		}
		if (data[0] == 'O')
			return parsePWO_(data + 1, size - 1) + 1;
		if (data[0] == 'S')
		{
			lastValue_ = 0; // used as index
			return parsePWS_(data + 1, size - 1) + 1;
		}
		return parseInvalid_(data, size);
	}

	std::size_t parsePWO_(char const* data, std::size_t size)
	{
		if (size == 0)
		{
			s_ = InternalState::Parse_PWO;
			return 0;
		}
		if (data[0] == 'N')
		{
			return parsePWON_(data + 1, size - 1) + 1;
		}
		return parseInvalid_(data, size);
	}

	std::size_t parsePWON_(char const* data, std::size_t size)
	{
		if (size == 0)
		{
			s_ = InternalState::Parse_PWON;
			return 0;
		}
		if (data[0] == '\r')
		{
			h_.powerChanged(true);
			s_ = InternalState::Begin;
			return 1;
		}
		// else invalid
		return parseInvalid_(data + 1, size -1) + 1;
	}

	std::size_t parsePWS_(char const* data, std::size_t size)
	{
		std::size_t nbConsumed = 0;
#define CASE(step, expectedChar)                                                                                       \
	case step:                                                                                                         \
		if (size == nbConsumed)                                                                                        \
		{                                                                                                              \
			s_ = InternalState::Parse_PWS;                                                                             \
			lastValue_ += nbConsumed;                                                                                  \
			return nbConsumed;                                                                                         \
		}                                                                                                              \
		if (data[nbConsumed] != expectedChar)                                                                          \
			return parseInvalid_(data + nbConsumed, size - nbConsumed) + nbConsumed;                                   \
		nbConsumed += 1;                                                                                               \
		[[fallthrough]];
		switch (lastValue_)
		{
			CASE(0, 'T')
			CASE(1, 'A')
			CASE(2, 'N')
			CASE(3, 'D')
			CASE(4, 'B')
			CASE(5, 'Y')
			case 6: // terminal state
				if (size == nbConsumed)
					return nbConsumed;
				if (data[nbConsumed] != '\r')
					return parseInvalid_(data + nbConsumed, size - nbConsumed) + nbConsumed;
				h_.powerChanged(false);
				s_ = InternalState::Begin;
				return nbConsumed + 1;
		}
		return parseInvalid_(data, size);
#undef CASE
	}

	std::size_t parseS_(char const* data, std::size_t size)
	{
		if (size == 0)
		{
			s_ = InternalState::Parse_S;
			return 0;
		}
		if (data[0] == 'I')
		{
			return parseSI_(data + 1, size - 1) + 1;
		}
		return parseInvalid_(data + 1, size - 1) + 1;
	}

	std::size_t parseSI_(char const* data, std::size_t size)
	{
		if (size == 0)
		{
			s_ = InternalState::Parse_SI;
			return 0;
		}
#define CASE(value, expected, nextvalue)                                                                               \
	case value:                                                                                                        \
		if (data[i] != expected)                                                                                       \
			return parseInvalid_(data + i, size - i) + i;                                                              \
		lastValue_ = nextvalue;                                                                                        \
		break

#define CASE_TERM(value, source)                                                                                       \
	case value:                                                                                                        \
		if (data[i] != '\r')                                                                                           \
			return parseInvalid_(data + i, size - i) + i;                                                              \
		h_.sourceChanged(source);                                                                                      \
		return i + 1

		for (std::size_t i = 0u; i < size; ++i)
		{
			switch (lastValue_)
			{
				case 0: // use lastValue_ to store state of parser
					if (data[i] == 'A') // AUX...
					{
						lastValue_ = 1000;
						break;
					}
					if (data[i] == 'B') // BD/BT
					{
						lastValue_ = 20;
						break;
					}
					if (data[i] == 'C') // CD
					{
						lastValue_ = 25;
						break;
					}
					if (data[i] == 'D') // DVD
					{
						lastValue_ = 300;
						break;
					}
					if (data[i] == 'G') // GAME
					{
						lastValue_ = 4000;
						break;
					}
					if (data[i] == 'H') // HDRADIO
					{
						lastValue_ = 500000;
						break;
					}
					if (data[i] == 'M') // MPLAY
					{
						lastValue_ = 60000;
						break;
					}
					if (data[i] == 'N') // NET
					{
						lastValue_ = 700;
						break;
					}
					if (data[i] == 'P') // PHONO
					{
						lastValue_ = 80000;
						break;
					}
					if (data[i] == 'S') // SAT/CBL
					{
						lastValue_ = 9000000;
						break;
					}
					if (data[i] == 'T') // TV/Tuner
					{
						lastValue_ = 0xA0000;
						break;
					}
					return parseInvalid_(data + i, size - i) + i;
					CASE(1000, 'U', 1100);
					CASE(1100, 'X', 1110);
				case 1110:
					if (std::isdigit(data[i]))
					{
						lastValue_ += data[i] - '0';
						break;
					}
					return parseInvalid_(data + i, size - i) + i;
				case 1111:
				case 1112:
				case 1113:
				case 1114:
				case 1115:
				case 1116:
				case 1117:
					if (data[i] != '\r')
						return parseInvalid_(data + i, size - i) + i;
					h_.sourceChanged(static_cast<Source>(static_cast<int>(Source::Aux1) + lastValue_ - 1111));
					return i + 1;
				case 20: // BD/BT
					if (data[i] == 'D')
					{
						lastValue_ = 21;
						break;
					}
					if (data[i] == 'T')
					{
						lastValue_ = 22;
						break;
					}
					return parseInvalid_(data + i, size - i) + i;
					CASE_TERM(21, Source::Bluray);
					CASE_TERM(22, Source::Bluetooth);

					CASE(25, 'D', 26); // CD
					CASE_TERM(26, Source::CD);

					CASE(300, 'V', 310); // DVD
					CASE(310, 'D', 311);
					CASE_TERM(311, Source::DVD);

					CASE(4000, 'A', 4100); // GAME
					CASE(4100, 'M', 4110);
					CASE(4110, 'E', 4111);
					CASE_TERM(4111, Source::Game);

					CASE(500000, 'D', 510000); // HDRADIO
					CASE(510000, 'R', 511000);
					CASE(511000, 'A', 511100);
					CASE(511100, 'D', 511110);
					CASE(511110, 'I', 511111);
					CASE(511111, 'O', 511112);
					CASE_TERM(511112, Source::HdRadio);

					CASE(60000, 'P', 61000); // MPLAY
					CASE(61000, 'L', 61100);
					CASE(61100, 'A', 61110);
					CASE(61110, 'Y', 61111);
					CASE_TERM(61111, Source::Multimedia);

					CASE(700, 'E', 710); // NET
					CASE(710, 'T', 711);
					CASE_TERM(711, Source::Network);

					CASE(80000, 'H', 81000); // PHONO
					CASE(81000, 'O', 81100);
					CASE(81100, 'N', 81110);
					CASE(81110, 'O', 81111);
					CASE_TERM(81111, Source::Phono);

					CASE(9000000, 'A', 9100000); // SAT/CBL
					CASE(9100000, 'T', 9110000); // SAT/CBL
					CASE(9110000, '/', 9111000); // SAT/CBL
					CASE(9111000, 'C', 9111100); // SAT/CBL
					CASE(9111100, 'B', 9111110); // SAT/CBL
					CASE(9111110, 'L', 9111111); // SAT/CBL
					CASE_TERM(9111111, Source::Cable_Sat); // SAT/CBL

				case 0xA0000:
					if (data[i] == 'V') // TV
					{
						lastValue_ = 0xA1000;
						break;
					}
					if (data[i] == 'U') // TUNER
					{
						lastValue_ = 0xA2000;
						break;
					}
					return parseInvalid_(data + i, size - i) + i;

					CASE_TERM(0xA1000, Source::TV);

					CASE(0xA2000, 'N', 0xA2100);
					CASE(0xA2100, 'E', 0xA2110);
					CASE(0xA2110, 'R', 0xA2111);
					CASE_TERM(0xA2111, Source::Tuner);
				default:
					return parseInvalid_(data + i, size - i) + 1;
			}
		}
		s_ = InternalState::Parse_SI;
		return size;
#undef CASE
#undef CASE_TERM
	}
};

constexpr std::string_view setSource(Source source)
{
	switch (source)
	{
		case eu::tgcm::avrcommand::Source::Phono:
			return "SIPHONO\n";
		case eu::tgcm::avrcommand::Source::CD:
			return "SICD\n";
		case eu::tgcm::avrcommand::Source::DVD:
			return "SIDVD\n";
		case eu::tgcm::avrcommand::Source::Bluray:
			return "SIBD\n";
		case eu::tgcm::avrcommand::Source::TV:
			return "SITV\n";
		case eu::tgcm::avrcommand::Source::Cable_Sat:
			return "SISAT/CBL\n";
		case eu::tgcm::avrcommand::Source::Multimedia:
			return "SIMPLAY\n";
		case eu::tgcm::avrcommand::Source::Game:
			return "SIGAME\n";
		case eu::tgcm::avrcommand::Source::Tuner:
			return "SITUNER\n";
		case eu::tgcm::avrcommand::Source::HdRadio:
			return "SIHDRADIO\n";
		case eu::tgcm::avrcommand::Source::Aux1:
			return "SIAUX1\n";
		case eu::tgcm::avrcommand::Source::Aux2:
			return "SIAUX2\n";
		case eu::tgcm::avrcommand::Source::Aux3:
			return "SIAUX3\n";
		case eu::tgcm::avrcommand::Source::Aux4:
			return "SIAUX4\n";
		case eu::tgcm::avrcommand::Source::Aux5:
			return "SIAUX5\n";
		case eu::tgcm::avrcommand::Source::Aux6:
			return "SIAUX6\n";
		case eu::tgcm::avrcommand::Source::Aux7:
			return "SIAUX7\n";
		case eu::tgcm::avrcommand::Source::Network:
			return "SINET\n";
		case eu::tgcm::avrcommand::Source::Bluetooth:
			return "SIBT\n";
	}
	return "";
}

constexpr std::string_view setMasterVolume(int volume, std::array<char, 6>& data)
{
	data[0] = 'M';
	data[1] = 'V';
	std::size_t i = 2;
	data[i] = (volume / 100) + '0';
	i += 1;
	data[i] = (volume % 100) / 10 + '0';
	i += 1;
	volume = volume % 10;
	if (volume == 5)
	{
		data[i] = volume + '0';
		i += 1;
	}
	data[i] = '\n';
	i += 1;
	return std::string_view(data.data(), i);
}

constexpr char const* queryMasterVolume = "MV?\n";
constexpr char const* masterVolumeUpCommand = "MVUP\n";
constexpr char const* masterVolumeDownCommand = "MVDOWN\n";

constexpr char const* queryPowerStatus = "PW?\n";
constexpr char const* powerOnCommand = "PWON\n";
constexpr char const* powerOffCommand = "PWSTANDBY\n";

constexpr char const* queryMasterZonePower = "ZM?\n";
constexpr char const* masterZonePowerOnCommand = "ZMON\n";
constexpr char const* masterZonePowerOffCommand = "ZMOFF\n";
constexpr char const* queryZone2Power = "Z2?\n";
constexpr char const* querySourceInput = "SI?\n";

} // namespace avrcommand
} // namespace tgcm
} // namespace eu


#endif // JBC_MARANTZUART_H

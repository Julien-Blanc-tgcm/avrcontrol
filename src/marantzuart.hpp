#ifndef EU_TGCM_AVRCOMMAND_MARANTZUART_H
#define EU_TGCM_AVRCOMMAND_MARANTZUART_H

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

	std::size_t parse(std::string_view data)
	{
		switch (s_)
		{
			case InternalState::Begin:
				return parseBegin_(data);
			case InternalState::Invalid: {
				return parseInvalid_(data);
			}
			case InternalState::Parse_M:
				return parseM_(data);
			case InternalState::Parse_MV:
				return parseMV_(data);
			case InternalState::Parse_MVM:
				return parseMVM_(data);
			case InternalState::Parse_MVMA:
				return parseMVMA_(data);
			case InternalState::Parse_MVMAX:
				return parseMVMAX_(data);
			case InternalState::Parse_MVMAX2:
				return parseMVMAX2_(data);
			case InternalState::Parse_MU:
				return parseMU_(data);
			case InternalState::Parse_MUO:
				return parseMUO_(data);
			case InternalState::Parse_MUON:
				return parseMUON_(data);
			case InternalState::Parse_MUOF:
				return parseMUOF_(data);
			case InternalState::Parse_MUOFF:
				return parseMUOFF_(data);
			case InternalState::Parse_P:
				return parseP_(data);
			case InternalState::Parse_PW:
				return parsePW_(data);
			case InternalState::Parse_PWO:
				return parsePWO_(data);
			case InternalState::Parse_PWON:
				return parsePWON_(data);
			case InternalState::Parse_PWS:
				return parsePWS_(data);
			case InternalState::Parse_S:
				return parseS_(data);
			case InternalState::Parse_SI:
				return parseSI_(data);
		}
		assert(false && "parser in a very bad state");
		return data.size(); // should not happen !!!
	}

  private:
	enum class InternalState
	{
		Begin,   /**< Initial state, wait for a reply */
		Invalid, /**< Invalid state, wait for a '\r' to go back to begin */
		Parse_M,
		Parse_MU,
		Parse_MUO,
		Parse_MUON,
		Parse_MUOF,
		Parse_MUOFF,
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

	std::size_t parseBegin_(std::string_view data)
	{
		lastValue_ = 0; // always reinitialize last value at begin
		if (data.empty())
			return 0;
		if (data[0] == 'M')
			return parseM_(data.substr(1)) + 1;
		if (data[0] == 'P')
			return parseP_(data.substr(1)) + 1;
		if (data[0] == 'S')
			return parseS_(data.substr(1)) + 1;
		// else need to implement
		return parseInvalid_(data);
	}

	// parse invalid, parse everything until finding a \r, allows ignoring
	// unknown status responses
	std::size_t parseInvalid_(std::string_view data)
	{
		std::size_t i = 0;
		while (i < data.size() && data[i] != '\r')
			i += 1;
		if (i < data.size()) // '\r' found
		{
			s_ = InternalState::Begin;
			i += 1; // consume the '\r'
		}
		else
			s_ = InternalState::Invalid;
		return i; // in all cases, we consumed i chars
	}

#define IF_EMPTY_RETURN_0(data, state)                                                                                 \
	if (data.empty())                                                                                                  \
	{                                                                                                                  \
		s_ = state;                                                                                                    \
		return 0;                                                                                                      \
	}

#define IF_DATA_EQUAL_RETURN_NEXTFUNC(data, expectedChar, nextFunc)                                                    \
	if (data[0] == expectedChar)                                                                                       \
		return nextFunc(data.substr(1)) + 1;

#define PARSE_SINGLE_EXPECTED_CHAR(funcName, state, expectedChar, nextFunc)                                            \
	std::size_t funcName(std::string_view data)                                                                        \
	{                                                                                                                  \
		IF_EMPTY_RETURN_0(data, state)                                                                                 \
		IF_DATA_EQUAL_RETURN_NEXTFUNC(data, expectedChar, nextFunc)                                                    \
		return parseInvalid_(data);                                                                                    \
	}

#define PARSE_BINARY_BRANCH(funcName, state, expectedChar1, nextFunc1, expectedChar2, nextFunc2)                       \
	std::size_t funcName(std::string_view data)                                                                        \
	{                                                                                                                  \
		IF_EMPTY_RETURN_0(data, state)                                                                                 \
		IF_DATA_EQUAL_RETURN_NEXTFUNC(data, expectedChar1, nextFunc1)                                                  \
		IF_DATA_EQUAL_RETURN_NEXTFUNC(data, expectedChar2, nextFunc2)                                                  \
		return parseInvalid_(data);                                                                                    \
	}

#define PARSE_TERMINAL(funcName, state, callback)                                                                      \
	std::size_t funcName(std::string_view data)                                                                        \
	{                                                                                                                  \
		IF_EMPTY_RETURN_0(data, state)                                                                                 \
		if (data[0] == '\r')                                                                                           \
		{                                                                                                              \
			callback;                                                                                                  \
			return 1;                                                                                                  \
		}                                                                                                              \
		return parseInvalid_(data);                                                                                    \
	}

	PARSE_BINARY_BRANCH(parseM_, InternalState::Parse_M, 'V', parseMV_, 'U', parseMU_);
	PARSE_SINGLE_EXPECTED_CHAR(parseMU_, InternalState::Parse_MU, 'O', parseMUO_);
	PARSE_BINARY_BRANCH(parseMUO_, InternalState::Parse_MUO, 'F', parseMUOF_, 'N', parseMUON_);
	PARSE_SINGLE_EXPECTED_CHAR(parseMUOF_, InternalState::Parse_MUOFF, 'F', parseMUOFF_);
	PARSE_TERMINAL(parseMUON_, InternalState::Parse_MUON, h_.mutedChanged(true));
	PARSE_TERMINAL(parseMUOFF_, InternalState::Parse_MUOFF, h_.mutedChanged(false));

	PARSE_SINGLE_EXPECTED_CHAR(parseMVM_, InternalState::Parse_MVM, 'A', parseMVMA_);
	PARSE_SINGLE_EXPECTED_CHAR(parseMVMA_, InternalState::Parse_MVMA, 'X', parseMVMAX_);
	PARSE_SINGLE_EXPECTED_CHAR(parseMVMAX_, InternalState::Parse_MVMAX, ' ', parseMVMAX2_);

	PARSE_SINGLE_EXPECTED_CHAR(parseP_, InternalState::Parse_P, 'W', parsePW_);

	PARSE_SINGLE_EXPECTED_CHAR(parsePWO_, InternalState::Parse_PWO, 'N', parsePWON_)
	PARSE_TERMINAL(parsePWON_, InternalState::Parse_PWON, h_.powerChanged(true))

	PARSE_SINGLE_EXPECTED_CHAR(parseS_, InternalState::Parse_S, 'I', parseSI_);

	std::size_t parsePW_(std::string_view data)
	{
		IF_EMPTY_RETURN_0(data, InternalState::Parse_PW)
		IF_DATA_EQUAL_RETURN_NEXTFUNC(data, 'O', parsePWO_);
		if (data[0] == 'S')
		{
			lastValue_ = 0; // used as index
			return parsePWS_(data.substr(1)) + 1;
		}
		return parseInvalid_(data);
	}

	std::size_t parseMVMAX2_(std::string_view data)
	{
		if (data.empty())
		{
			s_ = InternalState::Parse_MVMAX;
			return 0;
		}
		std::size_t i = 0;
		while (i < data.size() && std::isdigit(data[i]))
		{
			lastValue_ = lastValue_ * 10;
			lastValue_ += data[i] - '0';
			i += 1;
		}
		if (i < data.size())
		{
			if (data[i] == '\r') // found the '\r'
			{
				s_ = InternalState::Begin;
				if (lastValue_ < 100)
					lastValue_ *= 10;
				h_.maxVolumeChanged(lastValue_);
				return i + 1;
			}
			return parseInvalid_(data.substr(1)) + 1;
		}
		s_ = InternalState::Parse_MVMAX;
		return i;
	}

	std::size_t parseMV_(std::string_view data)
	{
		std::size_t i = 0;
		IF_EMPTY_RETURN_0(data, InternalState::Parse_MV)
		if (lastValue_ == 0 && data[i] == 'M')
			return parseMVM_(data.substr(1)) + 1;
		while (i < data.size() && std::isdigit(data[i]))
		{
			lastValue_ = lastValue_ * 10;
			lastValue_ += (data[i] - '0');
			i += 1;
		}
		if (i < data.size())
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
			return parseInvalid_(data.substr(i + 1)) + i + 1;
		}
		// else needs more data
		s_ = InternalState::Parse_MV;
		return i;
	}

	std::size_t parsePWS_(std::string_view data)
	{
		std::size_t nbConsumed = 0;
#define CASE(step, expectedChar)                                                                                       \
	case step:                                                                                                         \
		if (data.size() == nbConsumed)                                                                                 \
		{                                                                                                              \
			s_ = InternalState::Parse_PWS;                                                                             \
			lastValue_ += nbConsumed;                                                                                  \
			return nbConsumed;                                                                                         \
		}                                                                                                              \
		if (data[nbConsumed] != expectedChar)                                                                          \
			return parseInvalid_(data.substr(nbConsumed)) + nbConsumed;                                                \
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
				if (nbConsumed == data.size())
					return nbConsumed;
				if (data[nbConsumed] != '\r')
					return parseInvalid_(data.substr(nbConsumed)) + nbConsumed;
				h_.powerChanged(false);
				s_ = InternalState::Begin;
				return nbConsumed + 1;
		}
		return parseInvalid_(data);
#undef CASE
	}

	static constexpr int value_of_char_(char c)
	{
		if (c >= 'A' && c <= 'Z')
			return c - 'A' + 1;
		if (c == '/')
			return 25;
		if (c >= '0' && c <= '9')
			return 26 + c - '0';
		return 0;
	}

	static constexpr int value_of_str(char const* str)
	{
		if (*str == 0)
			return 0;
		return value_of_char_(*str) + (36 * (value_of_str(str + 1) % 59652323)); // this constant ensures no overflow
	}

	std::size_t parseSI_(std::string_view data)
	{
		IF_EMPTY_RETURN_0(data, InternalState::Parse_SI)
#define CASE(value, expected, nextvalue)                                                                               \
	case value_of_str(value):                                                                                          \
		if (data[i] != expected)                                                                                       \
			return parseInvalid_(data.substr(i)) + i;                                                                  \
		lastValue_ = value_of_str(nextvalue);                                                                          \
		break

#define CASE_TERM(value, source)                                                                                       \
	case value_of_str(value):                                                                                          \
		if (data[i] != '\r')                                                                                           \
			return parseInvalid_(data.substr(i)) + i;                                                                  \
		h_.sourceChanged(source);                                                                                      \
		return i + 1

		for (std::size_t i = 0u; i < data.size(); ++i)
		{
			switch (lastValue_)
			{
				case 0: // use lastValue_ to store state of parser
					if (data[i] != '\r')
						lastValue_ = value_of_char_(data[i]);
					else
						return parseInvalid_(data);
					break;

					CASE("A", 'U', "AU");
					CASE("AU", 'X', "AUX");
				case value_of_str("AUX"):
					if (data[i] == '1')
						lastValue_ = value_of_str("AUX1");
					else if (data[i] == '2')
						lastValue_ = value_of_str("AUX2");
					else if (data[i] == '3')
						lastValue_ = value_of_str("AUX3");
					else if (data[i] == '4')
						lastValue_ = value_of_str("AUX4");
					else if (data[i] == '5')
						lastValue_ = value_of_str("AUX5");
					else if (data[i] == '6')
						lastValue_ = value_of_str("AUX6");
					else if (data[i] == '7')
						lastValue_ = value_of_str("AUX7");
					else
						return parseInvalid_(data.substr(i)) + i;
					break;
				case value_of_str("AUX1"):
					if (data[i] != '\r')
						return parseInvalid_(data.substr(i)) + i;
					h_.sourceChanged(Source::Aux1);
					return i + 1;
				case value_of_str("AUX2"):
					if (data[i] != '\r')
						return parseInvalid_(data.substr(i)) + i;
					h_.sourceChanged(Source::Aux2);
					return i + 1;
				case value_of_str("AUX3"):
					if (data[i] != '\r')
						return parseInvalid_(data.substr(i)) + i;
					h_.sourceChanged(Source::Aux3);
					return i + 1;
				case value_of_str("AUX4"):
					if (data[i] != '\r')
						return parseInvalid_(data.substr(i)) + i;
					h_.sourceChanged(Source::Aux4);
					return i + 1;
				case value_of_str("AUX5"):
					if (data[i] != '\r')
						return parseInvalid_(data.substr(i)) + i;
					h_.sourceChanged(Source::Aux5);
					return i + 1;
				case value_of_str("AUX6"):
					if (data[i] != '\r')
						return parseInvalid_(data.substr(i)) + i;
					h_.sourceChanged(Source::Aux6);
					return i + 1;
				case value_of_str("AUX7"):
					if (data[i] != '\r')
						return parseInvalid_(data.substr(i)) + i;
					h_.sourceChanged(Source::Aux7);
					return i + 1;
				case value_of_str("B"): // BD/BT
					if (data[i] == 'D')
					{
						lastValue_ = value_of_str("BD");
						break;
					}
					if (data[i] == 'T')
					{
						lastValue_ = value_of_str("BT");
						break;
					}
					return parseInvalid_(data.substr(i)) + i;
					CASE_TERM("BD", Source::Bluray);
					CASE_TERM("BT", Source::Bluetooth);

					CASE("C", 'D', "CD"); // CD
					CASE_TERM("CD", Source::CD);

					CASE("D", 'V', "DV"); // DVD
					CASE("DV", 'D', "DVD");
					CASE_TERM("DVD", Source::DVD);

					CASE("G", 'A', "GA"); // GAME
					CASE("GA", 'M', "GAM");
					CASE("GAM", 'E', "GAME");
					CASE_TERM("GAME", Source::Game);

					CASE("H", 'D', "HD"); // HDRADIO
					CASE("HD", 'R', "HDR");
					CASE("HDR", 'A', "HDRA");
					CASE("HDRA", 'D', "HDRAD");
					CASE("HDRAD", 'I', "HDRADI");
					CASE("HDRADI", 'O', "HDRADIO");
					CASE_TERM("HDRADIO", Source::HdRadio);

					CASE("M", 'P', "MP"); // MPLAY
					CASE("MP", 'L', "MPL");
					CASE("MPL", 'A', "MPLA");
					CASE("MPLA", 'Y', "MPLAY");
					CASE_TERM("MPLAY", Source::Multimedia);

					CASE("N", 'E', "NE"); // NET
					CASE("NE", 'T', "NET");
					CASE_TERM("NET", Source::Network);

					CASE("P", 'H', "PH"); // PHONO
					CASE("PH", 'O', "PHO");
					CASE("PHO", 'N', "PHON");
					CASE("PHON", 'O', "PHONO");
					CASE_TERM("PHONO", Source::Phono);

					CASE("S", 'A', "SA"); // SAT/CBL
					CASE("SA", 'T', "SAT");
					CASE("SAT", '/', "SAT/");
					CASE("SAT/", 'C', "SAT/C");
					CASE("SAT/C", 'B', "SAT/CB");
					CASE("SAT/CB", 'L', "SAT/CBL");
					CASE_TERM("SAT/CBL", Source::Cable_Sat);

				case value_of_str("T"):
					if (data[i] == 'V') // TV
					{
						lastValue_ = value_of_str("TV");
						break;
					}
					if (data[i] == 'U') // TUNER
					{
						lastValue_ = value_of_str("TU");
						break;
					}
					return parseInvalid_(data.substr(i)) + i;

					CASE_TERM("TV", Source::TV);

					CASE("TU", 'N', "TUN");
					CASE("TUN", 'E', "TUNE");
					CASE("TUNE", 'R', "TUNER");
					CASE_TERM("TUNER", Source::Tuner);
				default:
					return parseInvalid_(data.substr(i)) + 1;
			}
		}
		s_ = InternalState::Parse_SI;
		return data.size();
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

constexpr char const* queryMute = "MU?\n";
constexpr char const* muteOnCommand = "MUON\n";
constexpr char const* muteOffCommand = "MUOFF\n";

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

#endif // EU_TGCM_AVRCOMMAND_MARANTZUART_H

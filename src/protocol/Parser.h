#ifndef AFINA_PROTOCOL_PARSER_H
#define AFINA_PROTOCOL_PARSER_H

#include <memory>
#include <string>
#include <vector>

#include <cstddef>
#include <cstdint>

namespace Afina {
namespace Execute {
class Command;
} // namespace Execute
namespace Protocol {

/**
 * # Memcached protocol parser
 * Parser supports subset of memcached protocol
 */
class Parser {
public:
    Parser() { Reset(); }
    /**
     * Push given string into parser input. Method returns true if it was a command parsed out
     * from comulative input. In a such case method Build will return new command
     *
     * @param input sttring to be added to the parsed input
     * @param parsed output parameter tells how many bytes was consumed from the string
     * @return true if command has been parsed out
     */
    bool Parse(const std::string &input, size_t &parsed) { return Parse(&input[0], input.size(), parsed); }

    /**
     * Push given string into parser input. Method returns true if it was a command parsed out
     * from comulative input. In a such case method Build will return new command
     *
     * @param input string to be added to the parsed input
     * @param size number of bytes in the input buffer that could be read
     * @param parsed output parameter tells how many bytes was consumed from the string
     * @return true if command has been parsed out
     */
    bool Parse(const char *input, const size_t size, size_t &parsed);

    /**
     * Builds new command from parsed input. In case if it wasn't enough input to prse command out
     * method return nullptr
     */
    std::unique_ptr<Execute::Command> Build(size_t &body_size) const;

    /**
     * Reset parse so that it could be used to parse out new command
     */
    void Reset();

    inline const std::string &Name() const { return name; }

private:
    /**
     * State of the command parser. Prefixes are:
     * - s: state for PUT and GET commands
     * - sp: for PUT commands only
     * - sg: for GET commands only
     */
    enum State : uint16_t { sCR, sLF, sName, spKey, spFlags, spExprTimeStart, spExprTime, spBytes, sgKey };

    // Current parser state
    State state;

    // vrious fields of the command
    std::string name;
    std::vector<std::string> keys;

    // <flags> is an arbitrary 16-bit unsigned integer (written out in decimal) that the server stores along with
    // the data and sends back when the item is retrieved. Clients may use this as a bit field to store data-specific
    //  information; this field is opaque to the server. Note that in memcached 1.2.1 and higher, flags may be 32-bits,
    // instead of 16, but you might want to restrict yourself to 16 bits for compatibility with older versions.
    uint32_t flags;

    // <exptime> is expiration time. If it's 0, the item never expires (although it may be deleted from the cache to
    // make place for other items). If it's non-zero (either Unix time or offset in seconds from current time), it is
    // guaranteed that clients will not be able to retrieve this item after the expiration time arrives (measured by
    // server time). If a negative value is given the item is immediately expired.
    int32_t exprtime;

    // <bytes> is the number of bytes in the data block to follow, *not*
    // including the delimiting \r\n. <bytes> may be zero (in which case
    // it's followed by an empty data block).
    uint32_t bytes;

    bool negative;
    std::string curKey;
    bool parse_complete;
};

} // namespace Protocol
} // namespace Afina

#endif // AFINA_MEMCACHED_PARSER_H

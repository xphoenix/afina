#ifndef AFINA_PROTOCOL_TEXT_IMPL_H
#define AFINA_PROTOCOL_TEXT_IMPL_H

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
class TextImpl {
public:
    TextImpl(std::string &&buf, std::size_t size, bool eof = false)
        : _ownership_s(std::move(buf)), _data_beg(&_ownership_s[0]), _data_ptr(&_ownership_s[0]),
          _data_end(&_ownership_s[0] + size), _data_eof(nullptr) {
        if (eof) {
            _data_eof = _data_end;
        }
        Reset();
    }

    TextImpl(std::unique_ptr<const char[]> &&buf, std::size_t size, bool eof = false)
        : _ownership_p(std::move(buf)), _data_beg(_ownership_p.get()), _data_ptr(_ownership_p.get()),
          _data_end(_ownership_p.get() + size), _data_eof(nullptr) {
        if (eof) {
            _data_eof = _data_end;
        }
        Reset();
    }

    TextImpl(const char *buf, std::size_t size, bool eof = false)
        : _data_beg(buf), _data_ptr(buf), _data_end(buf + size), _data_eof(nullptr) {
        if (eof) {
            _data_eof = _data_end;
        }
        Reset();
    }

    /**
     * Reads next command from the buffer if any
     */
    bool Next(std::unique_ptr<Execute::Command> &out);

    /**
     * Reset parse so that it could be used to parse out new command
     */
    void Reset();

protected:
    /**
     * Runs parser on the current buffer until command won't be available
     */
    void scan();

private:
    // Data ownership
    std::string _ownership_s;
    std::unique_ptr<const char[]> _ownership_p;

    // machine state
    int _cur_state;
    int _act;
    int _stack[1];
    int _stack_top;

    // Buffer
    const char *_data_beg;
    const char *_data_ptr;
    const char *_data_end;
    const char *_data_eof;

    // current token
    const char *_tok_start;
    const char *_tok_end;
};

} // namespace Protocol
} // namespace Afina

#endif // AFINA_PROTOCOL_TEXT_IMPL_H

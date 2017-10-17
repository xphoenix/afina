#include <afina/Storage.h>
#include <afina/execute/Get.h>

#include <iostream>
#include <iterator>
#include <sstream>

namespace Afina {
namespace Execute {

/* memcached protocol:

Each item sent by the server looks like this:

VALUE <key> <flags> <bytes>\r\n
<data block>\r\n

After all the items have been transmitted, the server sends the string
"END\r\n"
to indicate the end of response.

*/

void Get::Execute(Storage &storage, const std::string &args, std::string &out) {
    std::stringstream keyStream;
    copy(_keys.begin(), _keys.end(), std::ostream_iterator<std::string>(keyStream, " "));
    std::cout << "Get(" << keyStream.str() << ")" << std::endl;

    std::stringstream outStream;

    std::string value;
    for (auto &key : _keys) {
        if (!storage.Get(key, value))
            continue;
        outStream << "VALUE " << key << " 0 " << value.size() << "\r\n";
        outStream << value << "\r\n";
    }
    outStream << "END"; // networking layer should add the last \r\n

    out = outStream.str();
}

} // namespace Execute
} // namespace Afina

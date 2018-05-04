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

void Get::Execute(Storage &storage, const std::string &args, std::string &out) const {
    std::stringstream keyStream;
    std::string for_cout;
    if (!_strings.empty()) {
        copy(_strings.begin(), _strings.end(), std::ostream_iterator<std::string>(keyStream, " "));
	for_cout = keyStream.str();
	for_cout.pop_back();
    }

    std::cout << "Get(" << for_cout << ")" << std::endl; //pop_back - removes the last space

    std::stringstream outStream;
    std::string value;
    for (auto &key : _strings) {
        if (!storage.Get(key, value))
            continue;
        outStream << "VALUE " << key << " 0 " << value.size() << "\r\n";
        outStream << value << "\r\n";
    }
    outStream << "END"; // networking layer should add the last \r\n

    out = outStream.str();
}

const std::vector<std::string>& Get::keys() const { 
	return strings(); 
}

} // namespace Execute
} // namespace Afina

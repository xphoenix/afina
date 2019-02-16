#include <protocol/TextImpl.h>

#include <iostream>

namespace Afina {
namespace Protocol {

%% machine TextImplScanner;
%% write data;

%% access this->;
%% variable cs this->_cur_state;
%% variable act this->_act;
%% variable stack this->_stack;
%% variable top this->_stack_top;

%% variable p this->_data_ptr;
%% variable pe this->_data_end;
%% variable eof this->_data_eof;

%% variable ts this->_tok_start;
%% variable te this->_tok_end;

// See TextImpl.h
bool TextImpl::Next(std::unique_ptr<Execute::Command> &out) {
    std::cout << "Start scan: at " << (_data_ptr - _data_beg) << " of " << (_data_end - _data_beg) << " bytes" << std::endl;
    scan();
    if (_cur_state == TextImplScanner_error) {
        throw std::runtime_error("Error during command parse");
        // token->type = Token::None return token;
    }

    return false;
}

// See TextImpl.h
void TextImpl::Reset() {
  %% write init;
}

// See TextImpl.h
void TextImpl::scan() {
  %%write exec;
}

// Parser state machine
%%{
  key_format = (any - cntrl - space)+;
  flags_format = [0-9]+;
  time_format = [0-9]+;
  bytes_format = [0-9]+;
  cas_unique_format = [0-9]+;
  separator_format = "\r\n";

  get_names = "get" | "gets";
  gat_names = "gat" | "gats";
  storage_names = "set" | "add" | "replace" | "append" | "prepend";

  cas_command = "cas" key_format flags_format time_format bytes_format cas_unique_format "noreply"? separator_format;
  storage_command = storage_names space+ key_format space+ flags_format space+ time_format space+ bytes_format (space+ "noreply")? separator_format;
  get_command = get_names key_format (" " key_format)*;
  gat_command = gat_names time_format key_format (" " key_format)*;
  delete_command = "delete" key_format "noreply"?;
  incr_command = "incr" key_format key_format "noreply"?;
  decr_command = "decr" key_format key_format "noreply"?;
  touch_command = "touch" key_format time_format "noreply"?;

	main := |*
    # get_command => { std::cout << "get" << std::endl; fbreak; };
    # incr_command => { std::cout << "incr" << std::endl; fbreak; };
    # decr_command => { std::cout << "decr" << std::endl; fbreak; };
    # touch_command => { std::cout << "touch" << std::endl; fbreak; };
    # cas_command => { std::cout << "cas" << std::endl; fbreak; };
    storage_command => { std::cout << "store" << std::endl; fbreak; };
    # delete_command => { std::cout << "delete" << std::endl; fbreak; };
  *|;
}%%

} // namespace Protocol
} // namespace Afina

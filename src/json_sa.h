#ifndef _JSON_SA_H_
#define _JSON_SA_H_

//
// JSON Simple Access (a-la SAX) parser. It is easy to implement and test
// and it allows writing DOM-based parser more easily.
//

#include <string>
#include <istream>

namespace json {
  namespace simple {

    // Interface for tokenizer callback - instance will receive events during parse.
    struct token_callback {
      virtual ~token_callback() {}

      virtual void json_start() {}
      virtual void json_end() {}

      virtual void json_string(const std::string&) {}
      virtual void json_number(double) {}
      virtual void json_boolean(bool) {}
      virtual void json_null() {}

      virtual void json_comma() {}
      virtual void json_colon() {}

      virtual void json_array_starts() {}
      virtual void json_array_ends() {}

      virtual void json_object_starts() {}
      virtual void json_object_ends() {}

      virtual void json_error(const std::string&) {}

      virtual bool need_more_json() { return false; }
    };

    // Actual runner function that reads and tokenizes the JSON and invokes callback accordingly
    void run_tokenizer(const std::string&, token_callback&);
  }
}

#endif

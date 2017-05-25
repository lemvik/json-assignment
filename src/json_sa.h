#ifndef _JSON_SA_H_
#define _JSON_SA_H_

// JSON Simple Access (a-la SAX) tokenizer. It is easy to implement and test
// and it allows writing DOM-based parser more easily.

#include <string>
#include <istream>

namespace json {
  namespace simple {

    // Interface for tokenizer callback - instance will receive tokens and act accordingly.
    // Although all methods could've been purely virtual (abstract), it's easier to
    // make them do nothing and override only interesting ones (for tests, for example).
    struct token_callback {
      // Need virtual destructor since this is an interface to be implemented.
      virtual ~token_callback() {}

      // Invoked before reading first token to allow for (re-)initialization
      virtual void json_start() {}
      // Invoked after reading last token to allow for resource cleanup.
      virtual void json_end() {}

      // Invoked when a JSON string is read. The string that was read is the argument.
      virtual void json_string(const std::string&) {}
      // Invoked when a JSON number is read.
      virtual void json_number(double) {}
      // Invoked when a JSON boolean is read.
      virtual void json_boolean(bool) {}
      // Invoked when a JSON null is read.
      virtual void json_null() {}

      // Invoked when a comma not contained in a string is read.
      virtual void json_comma() {}
      // Invoked when a colon not contained in a string is read.
      virtual void json_colon() {}

      // Invoked when array opening bracket is read ([)
      virtual void json_array_starts() {}
      // Invoked when array closing bracket is read (])
      virtual void json_array_ends() {}

      // Invoked when array opening bracket is read ({)
      virtual void json_object_starts() {}
      // Invoked when array closing bracket is read (})
      virtual void json_object_ends() {}

      // Invoked when json tokenizer encounters an error in token or stream operation.
      // Since tokenizer itself doesn't know about JSON file structure, errors like
      // mismatched brackets should be handled by callback.
      virtual void json_error(const std::string&) {}

      // This function is being polled by tokenizer to check if
      // callback is satisfied with tokens it was fed. Tokenization ends
      // where this function returns false of there are errors reported via json_error.
      virtual bool need_more_json() { return false; }
    };

    // Runs tokenizer on given string, feeding tokens to given callback.
    // Actually delegates work to stream-based version, constructing stringstream from given string, so
    // more detailed doc there.
    void run_tokenizer(const std::string&, token_callback&);

    // Runs tokenizer on a given input stream, feeding tokens to given callback.
    // Attempts to contain any parsing exceptions instead propagating them into callback.json_error()
    // invocations. Stops after first error (another arguably better solution would be to throw exception to caller,
    // but that requires exposing exception type and forces callers to try/catch for parse errors, whereas
    // they might not be interested in errors).
    // callback is passed as a mutable reference to allow calling methods not marked const (see comment for token_callback)
    // if (!is) checks could be replaced with setting exception flags on 'is' to make it throw if any error occurs and wrap into
    // some meaningful exception in a top-level try/catch
    // Parsing continues either until stream encounters eof or callback decides that it has had enough and
    // returns false on need_more_json() invocation.
    void run_tokenizer(std::istream&, token_callback&);
  }
}

#endif

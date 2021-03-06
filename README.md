# Implement JSON (www.json.org) support using C++ 14. 

  Support for arrays is optional so that completing the assignment wouldn't take too long (see details below). Include some sample usage or tests.

## Goals 

   1. [X] Parse a string representation of JSON, e.g. `auto obj = toJSON("{}")`
   2. [X] Turn an object into a string representation, e.g. `auto s = toString(obj)`
   3. [X] Check if a name (i.e. key) is present in an object, e.g. `obj.hasKey("foo")`
   4. [X] Check if a value is of a given type, e.g. `obj.isString()`
   5. [X] Query the type of a value, e.g. `auto type = obj.getType()`
   6. [X] Get a value as a given type, e.g. `double value = obj.getDouble()`
   7. [X] Access and modify values in an object by name, e.g. `obj["foo"]["bar"] = "foobar"`
   8. [X] Add and remove name/value pairs to/from an object, e.g. `obj.remove["foo"]`
   9. [X] Allow iterating an object, e.g. `for(auto it = obj.begin(), it != obj.end(); ++it) {}`
   10. [X] Construct values from `std::string`, `char*`, `bool`, `float`/`double`/`short`/`int`/`long`
   11. [X] Support copy constructors and assignment operators where appropriate
   12. [X] Support move where appropriate
   13. [X] Support `operator==` and `operator!=` where appropriate
   14. [X] Support swap where appropriate
   15. [X] Support `const` where appropriate
   16. [X] Support iterating using `foreach`
   17. [X] Throw exceptions where appropriate
   18. [X] Use the C++ standard library where it makes sense

## Non-goals

   1. [ ] No need to support Unicode
   2. [ ] No need to make the parser very fast or very robust (i.e. handle every possible error in the input)
   3. [ ] No need to optimize the conversion to string representation
   4. [ ] No need to worry excessively about number to string and string to number conversions precision
   5. [ ] No need to handle NaN or infinity values
   6. [X] Optional support for arrays:
   7. [X] Access and modify values in an array by index, e.g. `array[2] = true`
   8. [X] Add and remove values to/from an array, e.g. `array.remove(0)`
   9. [X] Allow iterating an array, e.g. `for(auto it = array.begin(), it != array.end(); ++it) {}`

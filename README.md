# INI-parser
## INI configuration file parsing utilities

Large projects require a human-readable non-binary format for storing configuration files.
This solution include the tools required to parse and extract "an INI style" configuration file.

Supported INI file linguistic rules and format:
* Comments are declared by an '#' or ';' and can appear anywhere along a line.
  Anything after the comment character is ignored.
* Sections are declared inside square brackets ('[', ']') in a single line.
  Sections can not be duplicated in the same section.
  Sections can be nested, example of section nesting:
  i.e.:
  [foo]
  a = 1
  b = banana
  [[foo_nested]]
  a2=2
* Key/value pairs declaration is such that the key is followed by th euqals sign ('=') followed by the value,
  i.e. - <key> = <value>
* Key can be composed of all numerals and characters other then '#' and ';'.
  Keys can not be duplicated in the same section.
* Values which are arrays are declared inside a curly bracket ('{', '}') and the elements are seperated by commas (,).
  Array elements must be of the same type, example of array definition: array = {1, 2, 3}

Q&A:
 * Q: Why not XML?
 * A: Because its not human-readable without a visual aid or some sort of syntax highlighting mechanism.
 * Q: Why not JSON?
 * A: Easier to parse then XML, but still a bit over verbose.
 * Q: Why INI?
 * A: Easily read, unambiguous to parse, understood by everyone, can support everything without "special" rules & tokens.

#include "Parser.h"
#include <sstream>

// syntax sugar
#define $ .Get()

int main() {
    std::stringstream ss;

    ss <<
        "# defining a configuration file                            \n"
        "a-b*2=1 # an original named key!                           \n"
        "b=1                                                        \n\n"
        "# section 'e'                                              \n"
        "[e]                                                        \n"
        "ea=1 ; middle line comment                                 \n"
        "eb=1                                                       \n\n"
        "# section 'c'                                              \n"
        "[c]                                                        \n"
        "ca=2                                                       \n"
        "cb=2 # another middle line comment                         \n\n"
        "# nested section in 'e' called 'd'                         \n"
        "[[d]]                                                      \n"
        "da=3.0                                                     \n"
        "db={3, 4, 5}  ; <- look an integer vector...               \n\n"
        "# section 'e'                                              \n"
        "[A]                                                        \n"
        "Aa=true                                                    \n"
        "A-b=foo-bar # a key with an unusual character inside it    \n";

    // construct a config file structure
    Configuration::Parser config(ss);

    // print config file
    std::ostringstream out;
    config.ExportSection(out);
    std::cout << out.str();

    // extract scalars and vectors from config file
    int a               = Configuration::GetAs<int>             (config $["a-b*2"]);
    float da            = Configuration::GetAs<float>           (config $("c")("d")["da"]);
    std::vector<int> db = Configuration::GetAs<std::vector<int>>(config $("c")("d")["db"]);
    bool aa             = Configuration::GetAs<bool>            (config $("A")["Aa"]);
    std::string ab      =                                        config $("A")["A-b"];

    // clear config file structure
    config.Clear();


    return 1;
}

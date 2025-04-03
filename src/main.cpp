#include <getopt.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <string>

#include "fmt/base.h"
#include "util.hpp"

// clang-format off
// https://cfengine.com/blog/2021/optional-arguments-with-getopt-long/
// because "--opt-arg arg" won't work
// but "--opt-arg=arg" will
#define OPTIONAL_ARGUMENT_IS_PRESENT \
    ((optarg == NULL && optind < argc && argv[optind][0] != '-') \
     ? (bool) (optarg = argv[optind++]) \
     : (optarg != NULL))

static void version()
{
    fmt::print("brfucky " VERSION " branch " BRANCH "\n");
    std::exit(EXIT_SUCCESS);
}

static void help(bool invalid_opt = false)
{
    constexpr std::string_view help =
R"(Usage: brfucky [OPTIONS]...
    -i, --input      Save input text from stdin (pipes) or by typing
    -h, --help       Print this help menu
    -V, --version    Print the version along with the git branch it was built
)";
    fmt::print("{}\n", help);
    std::exit(invalid_opt);
}

bool parseargs(int argc, char* argv[], std::string& string)
{
    int opt               = 0;
    int option_index      = 0;
    const char* optstring = "-Vhi::";

    static const struct option opts[] = {
        {"version", no_argument,       0, 'V'},
        {"help",    no_argument,       0, 'h'},
        {"input",   optional_argument, 0, 'i'},

        {0,0,0,0}
    };

    // clang-format on
    optind = 0;
    while ((opt = getopt_long(argc, argv, optstring, opts, &option_index)) != -1)
    {
        switch (opt)
        {
            case 0:   break;
            case '?': help(EXIT_FAILURE); break;

            case 'V': version(); break;
            case 'h': help(); break;

            case 'i':
                if (OPTIONAL_ARGUMENT_IS_PRESENT)
                    string = optarg;
                else
                    string = getin();
                break;

            default: return false;
        }
    }

    return true;
}

void parse_str(const std::string& src)
{
    std::string buf;
    buf.resize(512);
    size_t i{};
    int closed{}, opened{};
    for (size_t pos = 0; pos < src.size(); ++pos)
    {
        switch (src[pos])
        {
            case '>': ++i; break;
            case '<':
                if (i == 0)
                    die("trying to point to non-existant memory");
                --i;
                break;
            case '+': ++buf[i]; break;
            case '-': --buf[i]; break;
            case '.': std::putchar(buf[i]); break;
            case '[':
                if (!buf[i])
                {
                    for (opened = 0, ++pos; pos < src.length(); ++pos)
                    {
                        if (src[pos] == ']' && !opened)
                            break;
                        else if (src[pos] == '[')
                            opened++;
                        else if (src[pos] == ']')
                            opened--;
                    }
                }
                break;
            case ']':
                if (buf[i])
                {
                    for (closed = 0, pos--; ; pos--)
                    {
                        if (src[pos] == '[' && !closed)
                            break;
                        else if (src[pos] == ']')
                            closed++;
                        else if (src[pos] == '[')
                            closed--;
                    }
                }
                break;
        }
    }
}

int main(int argc, char* argv[])
{
    std::string str;
    if (!parseargs(argc, argv, str))
        return 1;
    parse_str(str);
    return 0;
}

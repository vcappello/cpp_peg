#include <iostream>
#include <iterator>
#include "peg.h"

using namespace peg::literals;

void test_math_expr()
{
    auto opt_whitespace = peg::repeat::make(" "_L, 0, peg::repeat::n);

    auto number = peg::capture::make(peg::repeat::make("09"_R, 1, peg::repeat::n), "number");

    auto string_qualif = "\""_L;

    auto text = peg::capture::make(
        peg::sequence::make(
            {peg::ref::make(string_qualif.get()),
             peg::repeat::make(peg::neg::make(
                                   peg::ref::make(string_qualif.get()),
                                   peg::ref::make(peg::any.get())),
                               0, peg::repeat::n),
             peg::ref::make(string_qualif.get())}),
        "text");

    auto csv_field = peg::choice::make(
        {peg::ref::make(number.get()),
         peg::ref::make(text.get())});

    auto csv_line_rep = peg::repeat::make(
        peg::sequence::make(
            {peg::ref::make(opt_whitespace.get()),
             ","_L,
             peg::ref::make(opt_whitespace.get()),
             peg::ref::make(csv_field.get())}),
        0, peg::repeat::n);

    auto csv_line = peg::sequence::make(
        {peg::ref::make(opt_whitespace.get()),
         peg::ref::make(csv_field.get()),
         peg::ref::make(csv_line_rep.get())});

    std::stringstream ss("  12,  \"34\",  56  ");
    peg::rulse_inserter<std::vector<std::string>> rule_ins;
    auto [res, match_text] = csv_line->parse(ss, &rule_ins);

    std::cout << "Match return: " << res << std::endl;
    std::copy(rule_ins.m_container.begin(), rule_ins.m_container.end(), std::ostream_iterator<std::string>(std::cout, " - "));
}

int main()
{
    test_math_expr();

    return 0;
}

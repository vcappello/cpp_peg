#include <iostream>
#include <iterator>
#include "peg.h"

void test_math_expr()
{
    auto opt_whitespace = std::make_unique<peg::repeat>(
        std::make_unique<peg::literal>(" "), 0, peg::repeat::n);

    auto number = std::make_unique<peg::capture>(
        std::make_unique<peg::repeat>(
            std::make_unique<peg::range>('0', '9'), 1, peg::repeat::n),
        "number");

    auto string_qualif = std::make_unique<peg::literal>("\"");

    auto text = std::make_unique<peg::capture>(
        std::make_unique<peg::sequence>(
            peg::lst<peg::rule::rule_ptr_t>({std::make_unique<peg::ref>(string_qualif.get()),
                                             std::make_unique<peg::repeat>(std::make_unique<peg::neg>(
                                                                               std::make_unique<peg::ref>(string_qualif.get()),
                                                                               std::make_unique<peg::ref>(peg::any.get())),
                                                                           0, peg::repeat::n),
                                             std::make_unique<peg::ref>(string_qualif.get())})),
        "text");

    auto csv_field = std::make_unique<peg::choice>(
        peg::lst<peg::rule::rule_ptr_t>({std::make_unique<peg::ref>(number.get()),
                                         std::make_unique<peg::ref>(text.get())}));

    auto csv_line_rep = std::make_unique<peg::repeat>(
        std::make_unique<peg::sequence>(peg::lst<peg::rule::rule_ptr_t>({std::make_unique<peg::ref>(opt_whitespace.get()),
                                                                         std::make_unique<peg::literal>(","),
                                                                         std::make_unique<peg::ref>(opt_whitespace.get()),
                                                                         std::make_unique<peg::ref>(csv_field.get())})),
        0, peg::repeat::n);

    auto csv_line = std::make_unique<peg::sequence>(
        peg::lst<peg::rule::rule_ptr_t>({std::make_unique<peg::ref>(opt_whitespace.get()),
                                         std::make_unique<peg::ref>(csv_field.get()),
                                         std::make_unique<peg::ref>(csv_line_rep.get())}));

    std::stringstream ss("12,\"34\",56");
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

#include <iostream>
#include <iterator>
#include "peg.h"

void test_math_expr()
{
    peg::literal whitespace(" ");
    peg::repeat opt_whitespace(&whitespace, 0, peg::repeat::n);

    peg::range digit('0', '9');
    peg::repeat number_i(&digit, 1, peg::repeat::n);
    peg::capture number(&number_i, "number");

    // string = ["] [^"]* ["]
    peg::literal string_qualif("\"");
    peg::neg string_char(&string_qualif, &peg::any);
    peg::repeat string_1(&string_char, 0, peg::repeat::n);
    peg::sequence string_2({ &string_qualif, &string_1, &string_qualif });
    peg::capture string_value(&string_2, "string");

    peg::literal comma(",");

    peg::choice csv_field({ &number, &string_value });
    peg::sequence csv_1({ &opt_whitespace, &comma, &opt_whitespace, &csv_field });
    peg::repeat csv_2( &csv_1, 0, peg::repeat::n);
    peg::sequence csv_line({ &opt_whitespace, &csv_field, &csv_2 });

    std::stringstream ss("12,\"34\",56");
    peg::rulse_inserter<std::vector<std::string>> rule_ins;
    auto[ res, match_text ] = csv_line.parse(ss, &rule_ins);

    std::cout << "Match return: " << res << std::endl;
    std::copy(rule_ins.m_container.begin(), rule_ins.m_container.end(), std::ostream_iterator<std::string>(std::cout, " - "));
}

int main()
{
    test_math_expr();
    
    return 0;
}

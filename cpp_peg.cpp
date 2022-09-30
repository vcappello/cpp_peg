#include <iostream>
#include <iterator>
#include "peg.h"

using namespace peg::literals;

void test_basic()
{
    // Rule
    auto number = peg::capture::make(peg::repeat::make("09"_R, 1, peg::repeat::n), "number");
    // Text to parse
    std::stringstream ss("12345678");
    // Match
    auto res = peg::match(number.get(), ss);
    // Print result
    if (res.empty())
    {
        std::cout << "Not a number" << std::endl;
    }
    else
    {
        std::cout << "The number is " << res[0] << std::endl;
    }
}

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

    auto res = peg::match(csv_line.get(), ss);

    for (auto r : res)
    {
        std::cout << r << std::endl;
    }
}

int main()
{
    test_basic();

    test_math_expr();

    return 0;
}

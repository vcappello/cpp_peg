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

void test_csv()
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

void test_math_expr()
{
    // number     = [09]+
    // factor     = number | "(" expression ")"
    // component  = factor (("*" | "/") factor)*
    // expression = component (("+" | "-") component)*

    auto number = peg::capture::make(peg::repeat::make("09"_R, 1, peg::repeat::n), "NUMBER");
    auto open_par = peg::capture::make("("_L, "OPEN_PAR");
    auto close_par = peg::capture::make(")"_L, "CLOSE_PAR");
    auto plus = peg::capture::make("+"_L, "PLUS");
    auto minus = peg::capture::make("-"_L, "MINUS");
    auto multiply = peg::capture::make("*"_L, "MULTIPLY");
    auto divide = peg::capture::make("/"_L, "DIVIDE");

    auto expression_ref = peg::ref::make();

    auto factor = peg::choice::make({peg::ref::make(number.get()),
                                     peg::sequence::make({peg::ref::make(open_par.get()),
                                                          peg::ref::forward_ref(expression_ref.get()),
                                                          peg::ref::make(close_par.get())})});

    auto component = peg::sequence::make({peg::ref::make(factor.get()),
                                          peg::repeat::make(
                                              peg::sequence::make(
                                                  {peg::choice::make(
                                                       {peg::ref::make(multiply.get()),
                                                        peg::ref::make(divide.get())}),
                                                   peg::ref::make(factor.get())}),
                                              0, peg::repeat::n)});

    auto expression = peg::sequence::make({peg::ref::make(component.get()),
                                          peg::repeat::make(
                                              peg::sequence::make(
                                                  {peg::choice::make(
                                                       {peg::ref::make(plus.get()),
                                                        peg::ref::make(minus.get())}),
                                                   peg::ref::make(component.get())}),
                                              0, peg::repeat::n)});

    expression_ref->m_child = expression.get();

    std::stringstream ss("1*(2+3)");
    // expression
    //   component
    //     factor
    //       number
    //   plus
    //   component
    //     factor
    //       expression
    //         component
    //           factor
    //             number
    //           multiply
    //           factor
    //             number

    auto res = peg::match(expression.get(), ss);

    for (auto r : res)
    {
        std::cout << r << std::endl;
    }
}

int main()
{
    // test_basic();

    // test_csv();

    test_math_expr();

    return 0;
}

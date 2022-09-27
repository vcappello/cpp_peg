#include <iostream>
#include "peg.h"

using namespace std;

void test_math_expr()
{
    peg::literal whitespace(" ");
    peg::repeat opt_whitespace(&whitespace, 0, peg::repeat::n);

    peg::range digit('0', '9');
    peg::repeat number_i(&digit, 1, peg::repeat::n);
    peg::capture number(&number_i, "number");

    peg::literal add("+");
    peg::literal sub("-");
    peg::literal mul("*");
    peg::literal div("/");
    peg::choice term_op = {&add, &sub};
    peg::choice factor_op = {&mul, &div};
}

int main()
{
    test_math_expr();
    
    return 0;
}

# PEG for C++
Basic implementation of a PEG parser in C++.

## Example

### Basic example
Parse a string and test if contain an integer number:
```c++
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
```

## Usage

TODO

## Atomic expressions

| class         | abbr. | description                              |
|---------------|-------|------------------------------------------|
| peg::literal  | _L    | Match a string litterally                |
| peg::range    | _R    | Match if a character is in a given range |
| peg::any_char |       | Match any character                      |

## Operators

| class         | appr. | description                                           |
|---------------|-------|-------------------------------------------------------|
| peg::repeat   |       | Match a rule repetion using lower and higher limits   |
| peg::sequence |       | Match if all given rules match in the given order     |
| peg::choice   |       | Match if any of given rules match                     |
| pef::neg      |       | Match the second rule if the first rule doesn't match |
| pef::ref      |       | Reference to an existing rule                         |

## Captures

| class         | description                                           |
|---------------|-------------------------------------------------------|
| peg::capture  | Capture child rule                                    |

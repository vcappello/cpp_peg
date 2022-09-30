# PEG for C++
Basic implementation of a PEG parser in C++.

## Usage

TODO

## Atomic expressions

| class         | abbr. | description                              |
|---------------|-------|------------------------------------------|
| peg::literal  | _L    | Match a string litterally                |
| peg::range    |       | Match if a character is in a given range |
| peg::any_char |       | Match any character                      |

## Operators

| class         | appr. | description                                           |
|---------------|-------|-------------------------------------------------------|
| peg::repeat   |       | Match a rule repetion using lower and higher limits   |
| peg::sequence |       | Match if all given rules match in the given order     |
| peg::choice   |       | Match if any of given rules match                     |
| pef::neg      |       | Match the second rule if the first rule doesn't match |
| pef::ref      |       | Reference an existing rule                            |

## Captures

| class         | description                                           |
|---------------|-------------------------------------------------------|
| peg::capture  | Capture child rule                                    |

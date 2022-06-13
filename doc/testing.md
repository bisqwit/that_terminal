# Application testing

*That terminal* has been tested using unit tests, with coverage testing, and with practical testing.

## Unit tests

The application incorporates the [Google Test](https://github.com/google/googletest) unit testing framework.

The test report can be found here:
* [test report](test-report.txt)

Summary:

    [----------] Global test environment tear-down
    [==========] 65 tests from 16 test suites ran. (140972 ms total)
    [  PASSED  ] 65 tests.

## Coverage testing

Coverage testing is performed using a GCOV.

There is a coverage-testing binary `term_gcov` which can be used to test
coverage in user controlled situations.
Additionally, coverage-testing is performed for the unit-testing binary.

The testing results can be found here:
* [Test results](https://bisqwit.github.io/that_terminal/cov/index.html)

## Practical testing

This software is used for video making purposes by the author from time to time.
This ensures that the software is tested while being used.

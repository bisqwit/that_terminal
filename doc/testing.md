# Application testing

*That terminal* has been tested using unit tests, with coverage testing, and with practical testing.

## Unit tests

The application incorporates the [Google Test](https://github.com/google/googletest) unit testing framework.

The test report can be found here:
* [test report](test-report.txt)

Summary:

    [----------] Global test environment tear-down
    [==========] 70 tests from 16 test suites ran. (216020 ms total)
    [  PASSED  ] 70 tests.

To redo tests, do `make test`. The output will be saved to `doc/test-report.txt`.

Requirements:
* The ForkPTY test requires that a program `stat` is installed in your system.
* The ForkPTY test requires that your shell implements the `LINES` and `COLUMNS` environment variables.
* The rendering module tests require that the test program is able to access the font files in `share/`.
* The UI test requires being able to open a X11 window briefly. You can run the program in `xvfb` if necessary.

## Coverage testing

Coverage testing is performed using a GCOV.

There is a coverage-testing binary `term_gcov` which can be used to test
coverage in user controlled situations.
Additionally, coverage-testing is performed for the unit-testing binary.

The coverage test results can be found here:
* [Coverage test results](https://bisqwit.github.io/that_terminal/cov/index.html)

To redo tests, do `make test`. The output will be saved to `doc/doxygen/docs/cov/`.

## Practical testing

This software is used for video making purposes by the author from time to time.
This ensures that the software is tested while being used.

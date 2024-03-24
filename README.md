# stock-exchange

Simple stock exchange with only limit orders.

## Makefile

Makefile contains typically useful targets for development:

* `make build-debug` - debug build of the app
* `make build-release` - release build of the app with LTO
* `make test-debug` - does a `make build-debug` and runs all the tests on the result
* `make test-release` - does a `make build-release` and runs all the tests on the result
* `make start-debug` - builds the app in debug mode and starts it
* `make start-release` - builds the app in release mode and starts it
* `make` or `make all` - builds and runs all the tests in release and debug modes
* `make clean-` - cleans the object files
* `make dist-clean` - clean all, including the CMake cached configurations

Edit `Makefile.local` to change the default configuration and build options.

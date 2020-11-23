FMTLIB_HEADERS=../../external/fmtlib/fmt/include
FMTLIB_LIBRARY=../../external/fmtlib/fmt/build
DOCTEST_HEADERS=../../external/onqtam/doctest
DEPENDENCIES_HEADERS=../../external/taocpp/pegtl/include

PROJECT=uri
LINK.o=${LINK.cc}
CXXFLAGS+=-std=c++2a -Wall -Wextra -I../${FMTLIB_HEADERS} $(foreach dir, ${DEPENDENCIES_HEADERS}, -I../${dir})
LDFLAGS+= -L../${FMTLIB_LIBRARY}
LDLIBS+= -lfmt


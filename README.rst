isto::uri
===========

A C++20 headers-only library to parse URIs and resolve URI references, using 
`PEGTL <https://github.com/taocpp/PEGTL>`_


Requirements
------------

- `taocpp/PEGTL <https://github.com/taocpp/PEGTL>`_

To run the tests:

- `GNU Make <https://www.gnu.org/software/make/>`_.
- The `onqtam/doctest library <https://github.com/onqtam/doctest>`_.


Installation
------------

This is a headers-only library. Just put it where your compiler can find it.

It will need `taocpp/PEGTL <https://github.com/taocpp/PEGTL>`_ which is also
headers-only.


Tests
-----

The tests require the `onqtam/doctest library`_.
Edit the ``config.mk`` file to make the ``DOCTEST_HEADERS`` variable point to 
the directory containing ``doctest/doctest.h``. 

To execute the tests run

    $ make check

in the root directory of the project.


License
-------

SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception


Affiliation
-----------

This material is developed by the Scientific Computations and Modelling
platform at the Institut des Sciences de la Terre d'Orléans
(https://www.isto-orleans.fr/), a joint laboratory of the University of Orléans
(https://www.univ-orleans.fr/), the french National Center For Scientific
Research (https://www.cnrs.fr/) and the french Geological Survey
(https://www.brgm.eu/).

.. coding: utf-8

===================
Build Your Own Lisp
===================

*Disclaimer : The main reason for this repo is only synchronization in case my
laptop burns. This means that the code is not commented as it should*

I'm following an `online book`_ about building a small Lisp in C so I can learn
more about how Lisp works and learning how to maintain manually a Makefile for
a slightly bigger C project.

.. _online book: http://buildyourownlisp.com

1. Build instructions
=====================

::

    make lisp
    ./lisp

To pass the tests (currently you have to exit prompt manually after loading a
file) :

::

    ./lisp test_native.lspy
    ./lisp test_stdlib.lspy

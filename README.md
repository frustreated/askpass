# Overview

This program is a simple utility to get a password from a terminal with echo
disabled.  It writes the password to standard output so that it can be used to
securely provide a password for programs that accept reading a password from
the output of a program.

One feature that sets this apart from other utilities is that it optionally
supports accepting multiple lines of input, which is explained in detail below.

This program only accepts input from a terminal. If standard input is not a
terminal, it terminates with an error.

# Invocation

```
askpass [--help] [--echo] [--multi-line] [--no-eol-tx]
Option Summary:
  --help                generate help message
  --echo                Echo '*' to terminal for each accepted input character.
                          Note that when echo and multi-line are both enabled, 
                        either a new-line ('\n') or carriage-return ('\r') on 
                        input results in both '\n' and '\r' echoed to the 
                        terminal insted of a '*'.
  --multi-line          only terminate reading from input upon reading 
                        end-of-transmission (EOT), otherwise either ('\n') or 
                        ('\r') also terminates input
  --no-eol-tx           By default, askpass translates '\r' to '\n' on output. 
                        This option disables that translation.  Note: this 
                        option has no effect unless multi-line mode is enabled.

The 'Enter' or 'Return' key normally generates a '\r'.  You can also generate a '\r' using ^M.
You can generate a '\n' using ^J
You can generate an EOT using ^D.
```

# Input Processing

Askpass reads from the terminal connected to standard input.  When multi-line
is disabled (the default), askpass terminates input processing when either a
_newline_ (_\n_) or a _carriage-return_ (_\r_) is entered, or when an _EOT_ is
entered.  When multi-line is enabled, only a _EOT_ terminates input processing.

To erase a previously typed character, use either the __Backspace__ or
__Delete__ key.  Depending on terminal settings, __^H__ may also work.

Note that some keys generate a sequence of characters, depending on terminal
settings.  So if you are using a such keys for a password on one terminal,
entering the same key sequence on another terminal might result in a different
sequence of characters being written to standard output.

# Multi-Line mode

The use case that resulted in multi-line mode is the bitcoin-cli RPC based
program that can read both the RPC password and sensitive RPC commands that
require a password from standard input.  In this case, you use askpass in
multi-line mode and enter the password as the first line, and RPC commands
(that also have passwords as parameters) on subsequent lines.  This allows all
sensitive input to be entered without echoing it to the terminal.

In mult-line mode, pressing the __Enter__/__Return__ key generates a _\r_,
which askpass by default translates to a _\n_ on output.  You can optionally
disable this translation using the _--no-eol-tx_ option.

# Echo Mode

In echo mode, askpass echos a __*__ for any input character it accepts from
standard input, except for _\r_ and _\n_ characters which each echo both a
_\n_ and _\r_ to the terminal.  Typing __Backspace__, __Delete__, or __^H__
(depending on terminal settings) in echo mode erases the previously echoed
__*__ character on the terminal, as well as undoing the previously typed
character so that it is not written to stanard output.

# Signals

This program restores terminal settings upon receiving the following signals:

* SIGHUP
* SIGINT
* SIGTERM
* SIGPIPE

# Exit Codes

* __0__ if no error occurs
* _signal number_ if one of the above signals is received
* __100__ for invalid invocation syntax
* __101__ if standard input is not a terminal
* __102__ if an error occurs setting up the terminal or during input processing

# Limitations

In multi-line mode with echo enabled, if you have already entered a _\r_ or _\n_,
typing backspace multiple times such that you erase more characters than are on
the current line, the echoing does not reflect that characters on the previous
line have been erased from input.  They are in fact erased from input and are
not written to standard output, but the echoing just doesn't reflect this.

# Build Requirements

* [CMake](http://www.cmake.org) >= v2.6 (tested with v3.7.2)
* C++ compiler supporting C++-11 (tested with gcc v6.4.0)
* [GNU Make](https://www.gnu.org/software/make/make.html) (tested with v4.2.1)
* [ncurses](https://www.gnu.org/software/ncurses/) (tested with v6.0-r1)
* [boost](http://www.boost.org/) (tested with 1.63.0)

# Runtime Requirements

* C++ runtime libraries
* ncurses libraries
* boost\_program\_options library

# Build Instructions

1. mkdir -p build/default
1. cd build/default
1. cmake ../..
1. make

Output is single executable _askpass_.  Install in whatever directory you want,
or use _make install_ to install in CMake's default _bin_ directory
_/usr/local/bin_.


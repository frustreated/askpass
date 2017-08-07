# Overview

This program is a simple utility to get a password from a terminal with echo
disabled.  It writes the password to standard output so that it can be used to
securely provide a password for programs that accept reading a password from a
program.

One feature that sets this apart from other utilities is that it optionally
supports accepting multiple lines of input, which is explained in detail below.

This program only accepts input from a terminal. If standard input is not a
terminal, it terminates with an error.

# Invocation

askpass \[--help\] \[--echo\] \[--multi-line\]
Option Summary:
  --help                generate help message
  --multi-line          only terminate reading from input upon reading EOF, 
                        otherwise  also terminates input
  --echo                echo __*__ to terminal for each accepted input character.
                          Note that when multi-line is enabled, a new-line is 
                        echoed for input of new-line or carriage-return instead
                        of __*__.

# Input Processing

Askpass reads from the terminal connected to standard input.  When multi-line
is disabled (the default), askpass terminates input processing when either a
carriage-return(Return/Enter key) or newline (^J) is entered, or when a ^D is
entered.  When multi-line is enabled, only an ^D terminates input processing.

To erase a previously typed character, use either the Backspace or Delete key.
Depending on terminal settings, ^H may also work.

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

In mult-line mode, pressing the Enter/Return key generates a carriage-return,
which askpass writes to standard output with no translation.  If you desire
a newline instead, use ^J to generate it.

# Echo Mode

In echo mode, askpass echos a __*__ for any input character it accepts from
standard input, except for newline or carriage-return characters which echo a
new-line and carriage-return on the terminal.  Typing Backspace, Delete  or ^H
(depending on terminal settings) in echomode erases the previously echoed __*__
character on the terminal, as well as undoing the previously typed character so
that it is not written to stanard output.

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

In multi-line mode with echo enabled, if you have already entered a return,
typing backspace multiple times such that you erase more characters than are on
the current line, the echoing does not reflect that characters on the previous
line have been erased from input.  They are in fact erased from input and are
not written to standard output, but the echoing just doesn't reflect this.

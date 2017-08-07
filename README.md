### Overview

This program is a simple utility to get a password from standard input,
disabling input echo if standard input is a terminal.  It writes the
password to standard output so that it can be used to securely provide a
password for programs that accept reading a password from a program.

One feature that sets this apart from other utilities is that it optionally supports
accepting multiple lines of input, which is explained in detail below.

### Invocation

askpass [--help] [--echo] [--multi-line]
Option Summary:
  --help                generate help message
  --multi-line          only terminate reading from input upon reading EOF, 
                        otherwise  also terminates input
  --echo                echo '*' to terminal for each accepted input character.
                          Note that when multi-line is enabled, a new-line is 
                        echoed for input of new-line or carriage-return instead
                        of '*'.

### Input Processing

Askpass reads from standard input.  When multi-line is disabled (the default), askpass
terminates input when either a carriage-return or newline is entered (e.g., return/enter
key on keyboard), or when an EOT (character value 4) or EOF is received
(e.g., ^D on keyboard).  When multi-line is enabled, only an EOT/EOF terminates input processing.

To erase a previously typed character, use either the Backspace or Delete key.  Depending
on terminal settings, ^H may also work.

Askpass is designed to read input from a terminal, but it also supports reading from a
file or a pipe.

When input is a terminal, note that some keys generate a sequence of characters, depending
on terminal settings.  So if you are using a such keys for a password on one terminal, entering
the same key sequence on another terminal might result in a different sequence of characters
being written to standard output.

### Multi-Line mode

The use case that resulted in multi-line mode is the bitcoin-cli RPC based program that
can read both the RPC password and sensitive RPC commands that require a password from
standard input.  In this case, you use askpass in multi-line mode and enter the password
as the first line, and RPC commands (that also have passwords as parameters) on subsequent
lines.  This allows all sensitive input to be entered without echoing it to the terminal.

### Echo Mode

In echo mode, askpass echos a '*' for any input character it accepts from standard input.
Typing Backspace, Delete  or '^H' (depending on terminal settings) in echo mode erases the
previously echoed '*' character on the terminal, as well as undoing the previously typed
character so that it is not written to stanard output.

### Limitations

In multi-line mode with echo enabled, if you have already entered a return, typing backspace
multiple times such that you erase more characters than are on the current line, the echoing
does not reflect that characters on the previous line have been erased from input.  They are
in fact erased from input and are not written to standard output, but the echoing just doesn't
reflect this.

/*
    askpass - An ncurses based utility to get a password from a terminal.
    Copyright (C) 2017 Joseph Harvell

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    For questions, contact the author at joe.harvell.x@gmail.com
*/

#include <locale.h>
#include <curses.h>
#include <stdexcept>
#include <sstream>
#include <string>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <atomic>
#include <signal.h>
#include <unistd.h>
#include <boost/program_options.hpp>

static const char EchoOnInput = '*';

class ncterm
{
public:
    ncterm(FILE* input, FILE* output)
    :s(newterm(nullptr, input, output))
    {
        if(s == nullptr)
            throw std::runtime_error("Failed to initialize ncurses terminal.\n");
    }

    ~ncterm()
    {
        if(s != nullptr && ERR == endwin())
            std::cerr<<"Failed to finalize ncurses terminal.\n";
    }

    SCREEN* operator*() const { return s; }
    SCREEN* operator->() const { return s; }
private:
    SCREEN* s;
};

std::atomic<int> sig = ATOMIC_VAR_INIT(0);

void handle_signal(int s)
{
    sig.store(s);
}

bool setup_signal_handler(int sig, const struct sigaction* sa)
{
    if(-1 == sigaction(sig,sa,nullptr))
    {
        std::cerr<<"Failed to setup signal handler for signal "<<sig<<": "<<strerror(errno)<<'\n';
        return false;
    }
    return true;
}

// return true if at least one signal handler is installed
bool init_signals()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = handle_signal;

    if(-1 == sigfillset(&sa.sa_mask))
    {
        perror("sigfillset");
        return false;
    }

    bool rval = false;
    rval |= setup_signal_handler(SIGHUP,&sa);
    rval |= setup_signal_handler(SIGINT,&sa);
    rval |= setup_signal_handler(SIGTERM,&sa);
    rval |= setup_signal_handler(SIGPIPE,&sa);

    return rval;
}

int processInput(bool multiLine, bool echo, bool eolTx);

int main(int argc, const char* argv[])
{
    setlocale(LC_ALL,"");
    
    namespace po = boost::program_options;

    const std::string OptHelp("help");
    const std::string OptEcho("echo");
    const std::string OptMultiLine("multi-line");
    const std::string OptNoEolTx("no-eol-tx");

    std::string echoHelp("Echo '");
    echoHelp += EchoOnInput;
    echoHelp += "' to terminal for each accepted input character.  Note that when ";
    echoHelp += OptEcho;
    echoHelp += " and ";
    echoHelp += OptMultiLine;
    echoHelp += " are both enabled, either a new-line ('\\n') or carriage-return ('\\r') on input results in both '\\n' and '\\r' echoed to the terminal insted of a '";
    echoHelp += EchoOnInput;
    echoHelp += "'.";

    std::string noEolTxHelp("By default, askpass translates '\\r' to '\\n' on output.  This option disables that translation.  Note: this option has no effect unless ");
    noEolTxHelp += OptMultiLine;
    noEolTxHelp += " mode is enabled.";

    po::options_description desc("Option Summary");
    desc.add_options()
    (OptHelp.c_str(), "generate help message")
    (OptEcho.c_str(), echoHelp.c_str() )
    (OptMultiLine.c_str(), "only terminate reading from input upon reading end-of-transmission (EOT), otherwise either ('\\n') or ('\\r') also terminates input")
    (OptNoEolTx.c_str(), noEolTxHelp.c_str());

    po::variables_map vm;
    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
    }
    catch(const po::error& poe)
    {
        std::cerr<<"Error processing command line: "<<poe.what()<<'\n';
        return 100;
    }
    
    if(vm.count(OptHelp))
    {
        std::cout<<argv[0]<<" [--"<<OptHelp<<"] [--"<<OptEcho<<"] [--"<<OptMultiLine<<"] [--"<<OptNoEolTx<<"]\n"<<desc
                <<"\nThe 'Enter' or 'Return' key normally generates a '\\r'.  You can also generate a '\\r' using ^M."
                <<"\nYou can generate a '\\n' using ^J"
                <<"\nYou can generate an EOT using ^D.\n";
                return 0;
    }
    
    bool echo = vm.count(OptEcho) != 0;
    bool multiLine = vm.count(OptMultiLine) != 0;
    bool eolTx = vm.count(OptNoEolTx) == 0;

    if(!isatty(fileno(stdin)))
    {
        std::cerr<<"Standard input is not a terminal.\n";
        return 101;
    }
    init_signals();

    try
    {
        return processInput(multiLine, echo, eolTx);
    }
    catch(const std::exception& ex)
    {
        std::cerr<<"Exception processing input: "<<ex.what()<<'\n';
        return 102;
    }
}

int processInput(bool multiLine, bool echo, bool eolTx)
{
    const char CharEot = 4;
    const char CharDel = 127;

    std::size_t endpos = 0;
    std::stringstream ss;

    ncterm nct(stdin, stderr);
    if(ERR == cbreak())
        throw std::runtime_error("Failed to disable line buffering and erase/kill processing on standard input.");
    if(ERR == noecho())
        throw std::runtime_error("Failed to disable input echoing in ncurses library.");
    if(ERR == nonl())
        throw std::runtime_error("Failed to disable newline translation in ncurses library.");

    for(;;)
    {
        int c = getch();
        int s = sig.load();
        if(s != 0)
            return s;
#ifdef DEBUG
        std::cerr<<' '<<c<<' '<<std::flush;
#endif
        if(c == CharEot)
            break;
        if(!multiLine && (c == '\n' || c == '\r'))
            break;
        else if(c == '\b' || c == CharDel)
        {
            if(endpos != 0)
            {
                ss.seekp(-1, std::ios_base::cur);
                --endpos;
                if(echo)
                {
                    echochar('\b');
                    echochar(' ');
                    echochar('\b');
                }
            }
        }
        else
        {
            if(c == '\r' && eolTx)
                c = '\n';
            ss.put(c);
            ++endpos;
            if(echo)
            {
                char echoChar;
                if(c == '\r' || c == '\n')
                    echoChar = '\n';
                else
                    echoChar = EchoOnInput;
                echochar(echoChar);
            }
        }
    }

    std::string str(ss.str());
    str.erase(endpos);
    std::cout<<str;

    return 0;
}

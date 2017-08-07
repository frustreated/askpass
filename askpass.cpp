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
#include <boost/program_options.hpp>

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

int main(int argc, const char* argv[])
{
    setlocale(LC_ALL,"");
    
    namespace po = boost::program_options;

    const std::string OptHelp("help");
    const std::string OptMultiLine("multi-line");
    const std::string OptEcho("echo");
    
    const char CharEot = 4;
    const char CharDel = 127;
    const char EchoOnInput = '*';

    std::string echoHelp("echo '");
    echoHelp += EchoOnInput;
    echoHelp += "' to terminal for each accepted input character.  Note that when ";
    echoHelp += OptMultiLine;
    echoHelp += " is enabled, a new-line is echoed for input of new-line or carriage-return instead of '";
    echoHelp += EchoOnInput;
    echoHelp += "'.";

    po::options_description desc("Option Summary");
    desc.add_options()
    (OptHelp.c_str(), "generate help message")
    (OptMultiLine.c_str(), "only terminate reading from input upon reading EOF, otherwise  also terminates input")
    (OptEcho.c_str(), echoHelp.c_str() );
    
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
        std::cout<<desc;
        return 0;
    }
    
    bool multiLine = vm.count(OptMultiLine) != 0;
    bool echo = vm.count(OptEcho) != 0;

    std::size_t endpos = 0;
    std::stringstream ss;
    init_signals();

    { // new scope for ncurses mode
        ncterm nct(stdin,stderr);
        if(ERR == cbreak())
            throw std::runtime_error("Failed to disable line buffering and erase/kill processing on standard input.\n");
        if(ERR == noecho())
            throw std::runtime_error("Failed to disable input echoing in ncurses library.\n");
        if(ERR == nl())
            throw std::runtime_error("Failed to enable newline translation in ncurses library.\n");

        for(;;)
        {
            int c = getch();
            int s = sig.load();
            if(s != 0)
                return s;
#ifdef DEBUG
            std::cerr<<' '<<c<<' '<<std::flush;
#endif
            if(c == CharEot || c == EOF) // CharEot for terminal, EOF for file or pipe
                break;
            if(!multiLine && c == '\n')
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
                ss.put(c);
                ++endpos;
                if(echo)
                {
                    char ec;
                    if(c == '\n')
                        ec = c;
                    else
                        ec = EchoOnInput;
                    echochar(ec);
                }
            }
        }
    }
    std::string str(ss.str());
    str.erase(endpos);
    std::cout<<str;

    return 0;
}

/******************************************************************************\
 *           ___        __                                                    *
 *          /\_ \    __/\ \                                                   *
 *          \//\ \  /\_\ \ \____    ___   _____   _____      __               *
 *            \ \ \ \/\ \ \ '__`\  /'___\/\ '__`\/\ '__`\  /'__`\             *
 *             \_\ \_\ \ \ \ \L\ \/\ \__/\ \ \L\ \ \ \L\ \/\ \L\.\_           *
 *             /\____\\ \_\ \_,__/\ \____\\ \ ,__/\ \ ,__/\ \__/.\_\          *
 *             \/____/ \/_/\/___/  \/____/ \ \ \/  \ \ \/  \/__/\/_/          *
 *                                          \ \_\   \ \_\                     *
 *                                           \/_/    \/_/                     *
 *                                                                            *
 * Copyright (C) 2011, 2012                                                   *
 * Dominik Charousset <dominik.charousset@haw-hamburg.de>                     *
 *                                                                            *
 * This file is part of libcppa.                                              *
 * libcppa is free software: you can redistribute it and/or modify it under   *
 * the terms of the GNU Lesser General Public License as published by the     *
 * Free Software Foundation, either version 3 of the License                  *
 * or (at your option) any later version.                                     *
 *                                                                            *
 * libcppa is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                       *
 * See the GNU Lesser General Public License for more details.                *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with libcppa. If not, see <http://www.gnu.org/licenses/>.            *
\******************************************************************************/


#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "cppa/cppa.hpp"
#include "cppa/fsm_actor.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::int64_t;

using namespace cppa;

struct fsm_receiver : fsm_actor<fsm_receiver>
{
    int64_t m_value;
    behavior init_state;
    fsm_receiver(int64_t max) : m_value(0)
    {
        init_state =
        (
            on(atom("msg")) >> [=]()
            {
                ++m_value;
                if (m_value == max)
                {
                    quit(exit_reason::normal);
                }
            }
        );
    }
};

void receiver(int64_t max)
{
    int64_t value;
    receive_loop
    (
        on(atom("msg")) >> [&]()
        {
            ++value;
            if (value == max)
            {
                quit(exit_reason::normal);
            }
        }
    );
}

void sender(actor_ptr whom, int64_t count)
{
    any_tuple msg = make_tuple(atom("msg"));
    for (int64_t i = 0; i < count; ++i)
    {
        whom->enqueue(nullptr, msg);
    }
}

void usage()
{
    cout << "usage: mailbox_performance "
            "(stacked|event-based) (sending threads) (msg per thread)" << endl
         << endl;
}

int main(int argc, char** argv)
{
    if (argc == 4)
    {
        char* endptr = nullptr;
        int64_t num_sender = static_cast<int64_t>(strtol(argv[2], &endptr, 10));
        if (endptr == nullptr || *endptr != '\0')
        {
            cerr << "\"" << argv[2] << "\" is not an integer" << endl;
            usage();
            return 1;
        }
        int64_t num_msgs = static_cast<int64_t>(strtol(argv[3], &endptr, 10));
        if (endptr == nullptr || *endptr != '\0')
        {
            cerr << "\"" << argv[3] << "\" is not an integer" << endl;
            usage();
            return 1;
        }
        actor_ptr testee;
        if (strcmp(argv[1], "stacked") == 0)
        {
            testee = spawn(receiver, num_sender * num_msgs);
        }
        else if (strcmp(argv[1], "event-based") == 0)
        {
            testee = spawn(new fsm_receiver(num_sender * num_msgs));
        }
        else
        {
            usage();
            return 1;
        }
        for (int64_t i = 0; i < num_sender; ++i)
        {
            spawn<detached>(sender, testee, num_msgs);
        }
        await_all_others_done();
    }
    else
    {
        usage();
        return 1;
    }
    return 0;
}
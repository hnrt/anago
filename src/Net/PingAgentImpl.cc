// Copyright (C) 2012-2017 Hideaki Narita


#define NO_TRACE


#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "Logger/Trace.h"
#include "Thread/ThreadManager.h"
#include "PingAgentImpl.h"


using namespace hnrt;


PingAgentImpl::PingAgentImpl()
    : _thread(NULL)
    , _interval(60)
{
    TRACEFUN(this, "PingAgentImpl::ctor");
}


PingAgentImpl::~PingAgentImpl()
{
    TRACEFUN(this, "PingAgentImpl::dtor");
    if (_thread)
    {
        close();
    }
}


void PingAgentImpl::open()
{
    TRACEFUN(this, "PingAgentImpl::open");
    if (_thread)
    {
        return;
    }
    _stop = false;
    _thread = ThreadManager::instance().create(sigc::mem_fun(*this, &PingAgentImpl::run), true, "PingAgent");
}


void PingAgentImpl::close()
{
    TRACEFUN(this, "PingAgentImpl::close");
    if (!_thread)
    {
        return;
    }
    _stop = true;
    _cond.signal();
    _thread->join();
    _thread = NULL;
}


void PingAgentImpl::clear()
{
    Glib::Mutex::Lock lock(_mutexRecords);
    _records.clear();
}


void PingAgentImpl::add(const char* hostname)
{
    Glib::Mutex::Lock lock(_mutexRecords);
    for (std::list<Record>::iterator iter = _records.begin(); iter != _records.end(); iter++)
    {
        if (iter->hostname == hostname)
        {
            return;
        }
    }
    append(hostname);
    _cond.signal();
}


void PingAgentImpl::remove(const char* hostname)
{
    Glib::Mutex::Lock lock(_mutexRecords);
    for (std::list<Record>::iterator iter = _records.begin(); iter != _records.end(); iter++)
    {
        if (iter->hostname == hostname)
        {
            _records.erase(iter);
            return;
        }
    }
}


PingAgent::State PingAgentImpl::get(const char* hostname)
{
    TRACEFUN(this, "PingAgentImpl::get(%s)", hostname);
    Glib::Mutex::Lock lock(_mutexRecords);
    for (std::list<Record>::iterator iter = _records.begin(); iter != _records.end(); iter++)
    {
        if (iter->hostname == hostname)
        {
            TRACEPUT("return=%d", (int)iter->state);
            return iter->state;
        }
    }
    append(hostname);
    _cond.signal();
    TRACEPUT("Not found.");
    return UNKNOWN;
}


void PingAgentImpl::append(const char* hostname)
{
    Record record;
    record.hostname = hostname;
    record.state = UNKNOWN;
    record.timestamp = 0;
    _records.push_back(record);
}


void PingAgentImpl::run()
{
    TRACEFUN(this, "PingAgentImpl::run");
    Glib::Mutex::Lock lock(_mutexThread);
    while (!_stop)
    {
        check();
        Glib::TimeVal timeout;
        timeout.assign_current_time();
        timeout.add_seconds(_interval);
        _cond.timed_wait(_mutexThread, timeout);
    }
}


void PingAgentImpl::check()
{
    std::list<Glib::ustring> hostnames;
    getHostnames(hostnames);
    for (std::list<Glib::ustring>::iterator iter = hostnames.begin(); iter != hostnames.end(); iter++)
    {
        State state = check(iter->c_str());
        update(*iter, state);
    }
}


void PingAgentImpl::getHostnames(std::list<Glib::ustring>& hostnames)
{
    time_t currentTime = time(NULL);
    Glib::Mutex::Lock lock(_mutexRecords);
    for (std::list<Record>::iterator iter = _records.begin(); iter != _records.end(); iter++)
    {
        if (iter->timestamp + _interval <= currentTime)
        {
            hostnames.push_back(iter->hostname);
        }
    }
}


PingAgent::State PingAgentImpl::check(const char* hostname)
{
    TRACEFUN(this, "PingAgentImpl::check(%s)", hostname);

    State result = UNKNOWN;

    try
    {
        int fd[2];

        if (pipe(fd))
        {
            TRACEPUT("pipe failed. errno=%d", errno);
            goto done;
        }

        pid_t pid = fork();
        if (pid < 0)
        {
            TRACEPUT("fork failed. errno=%d", errno);
            ::close(fd[0]);
            ::close(fd[1]);
            goto done;
        }
        else if (pid == 0)
        {
            if (::close(STDOUT_FILENO))
            {
                TRACEPUT("close(STDOUT) failed. errno=%d", errno);
            }
            bool ok = true;
            if (dup2(fd[1], STDOUT_FILENO) == -1)
            {
                TRACEPUT("dup2(STDOUT) failed. errno=%d", errno);
                ok = false;
            }
            ::close(fd[0]);
            ::close(fd[1]);
            if (ok)
            {
                execl("/bin/ping", "ping", "-c", "1", "-W", "1", hostname, NULL);
                TRACEPUT("execl(/bin/ping -c 1 -W 1 %s) failed. errno=%d", hostname, errno);
            }
            _exit(EXIT_FAILURE);
        }

        ::close(fd[1]);

        char buf[512] = { 0 };
        char* cur = buf;
        char* end = buf + sizeof(buf);
        while (1)
        {
            if (cur >= end)
            {
                break;
            }
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(fd[0], &fds);
            struct timeval t;
            t.tv_sec = 1;
            t.tv_usec = 250000;
            int rc = select(fd[0] + 1, &fds, NULL, NULL, &t);
            if (rc < 0)
            {
                TRACEPUT("select failed. errno=%d", errno);
                break;
            }
            else if (rc == 0)
            {
                TRACEPUT("Timed out.");
                break;
            }
            ssize_t n = read(fd[0], cur, end - cur);
            if (n < 0)
            {
                TRACEPUT("read failed. errno=%d", errno);
                break;
            }
            else if (n == 0)
            {
                break;
            }
            char* stop = cur + n;
            char* eol = reinterpret_cast<char*>(memchr(cur, '\n', n));
            if (eol)
            {
                static const char succeeded[] = { "1 packets transmitted, 1 received" };
                static const char failed[] = { "1 packets transmitted, 0 received" };
                char* bol = buf;
                for (;;)
                {
                    *eol = '\0';
                    TRACEPUT(">>%s", bol);
                    if (!strncmp(bol, succeeded, strlen(succeeded)))
                    {
                        result = ACTIVE;
                        goto post_process;
                    }
                    else if (!strncmp(bol, failed, strlen(failed)))
                    {
                        result = INACTIVE;
                        goto post_process;
                    }
                    else
                    {
                        bol = eol + 1;
                        if (bol >= stop)
                        {
                            cur = buf;
                            break;
                        }
                        n = stop - bol;
                        eol = reinterpret_cast<char*>(memchr(bol, '\n', n));
                        if (!eol)
                        {
                            memmove(buf, bol, n);
                            cur = buf + n;
                            break;
                        }
                    }
                }
            }
            else
            {
                cur = stop;
            }
        }

    post_process:

        ::close(fd[0]);

        int status = 0;
        pid_t rc = waitpid(pid, &status, 0);
        if (rc == pid)
        {
            TRACEPUT("pid=%d status=%d", pid, WEXITSTATUS(status));
        }
        else
        {
            TRACEPUT("waitpid(%d) failed. errno=%d", pid, errno);
        }
    }
    catch (...)
    {
        TRACEPUT("Unhandled exception caught.");
    }

done:

    TRACEPUT("result=%d", result);

    return result;
}


void PingAgentImpl::update(const Glib::ustring& hostname, State state)
{
    Glib::Mutex::Lock lock(_mutexRecords);
    for (std::list<Record>::iterator iter = _records.begin(); iter != _records.end(); iter++)
    {
        if (iter->hostname == hostname)
        {
            iter->state = state;
            iter->timestamp = time(NULL);
            return;
        }
    }
}

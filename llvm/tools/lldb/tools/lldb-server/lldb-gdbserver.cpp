//===-- lldb-gdbserver.cpp --------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// C Includes
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <signal.h>
#include <unistd.h>
#endif

// C++ Includes

// Other libraries and framework includes
#include "llvm/ADT/StringRef.h"

#include "lldb/Core/Error.h"
#include "lldb/Core/PluginManager.h"
#include "lldb/Host/ConnectionFileDescriptor.h"
#include "lldb/Host/HostGetOpt.h"
#include "lldb/Host/OptionParser.h"
#include "lldb/Host/Pipe.h"
#include "lldb/Host/Socket.h"
#include "lldb/Host/StringConvert.h"
#include "lldb/Target/Platform.h"
#include "Acceptor.h"
#include "LLDBServerUtilities.h"
#include "Plugins/Process/gdb-remote/GDBRemoteCommunicationServerLLGS.h"
#include "Plugins/Process/gdb-remote/ProcessGDBRemoteLog.h"

#ifndef LLGS_PROGRAM_NAME
#define LLGS_PROGRAM_NAME "lldb-server"
#endif

#ifndef LLGS_VERSION_STR
#define LLGS_VERSION_STR "local_build"
#endif

using namespace llvm;
using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::lldb_server;
using namespace lldb_private::process_gdb_remote;

//----------------------------------------------------------------------
// option descriptors for getopt_long_only()
//----------------------------------------------------------------------

static int g_debug = 0;
static int g_verbose = 0;

static struct option g_long_options[] =
{
    { "debug",              no_argument,        &g_debug,           1   },
    { "platform",           required_argument,  NULL,               'p' },
    { "verbose",            no_argument,        &g_verbose,         1   },
    { "log-file",           required_argument,  NULL,               'l' },
    { "log-channels",       required_argument,  NULL,               'c' },
    { "attach",             required_argument,  NULL,               'a' },
    { "named-pipe",         required_argument,  NULL,               'N' },
    { "pipe",               required_argument,  NULL,               'U' },
    { "native-regs",        no_argument,        NULL,               'r' },  // Specify to use the native registers instead of the gdb defaults for the architecture.  NOTE: this is a do-nothing arg as it's behavior is default now.  FIXME remove call from lldb-platform.
    { "reverse-connect",    no_argument,        NULL,               'R' },  // Specifies that llgs attaches to the client address:port rather than llgs listening for a connection from address on port.
    { "setsid",             no_argument,        NULL,               'S' },  // Call setsid() to make llgs run in its own session.
    { NULL,                 0,                  NULL,               0   }
};


//----------------------------------------------------------------------
// Watch for signals
//----------------------------------------------------------------------
static int g_sigpipe_received = 0;
static int g_sighup_received_count = 0;

#ifndef _WIN32

static void
signal_handler(int signo)
{
    Log *log (GetLogIfAnyCategoriesSet(LIBLLDB_LOG_PROCESS));

    fprintf (stderr, "lldb-server:%s received signal %d\n", __FUNCTION__, signo);
    if (log)
        log->Printf ("lldb-server:%s received signal %d", __FUNCTION__, signo);

    switch (signo)
    {
    case SIGPIPE:
        g_sigpipe_received = 1;
        break;
    }
}

static void
sighup_handler(MainLoopBase &mainloop)
{
    ++g_sighup_received_count;

    Log *log (GetLogIfAnyCategoriesSet(LIBLLDB_LOG_PROCESS));
    if (log)
        log->Printf ("lldb-server:%s swallowing SIGHUP (receive count=%d)", __FUNCTION__, g_sighup_received_count);

    if (g_sighup_received_count >= 2)
        mainloop.RequestTermination();
}
#endif // #ifndef _WIN32

static void
display_usage (const char *progname, const char* subcommand)
{
    fprintf(stderr, "Usage:\n  %s %s "
            "[--log-file log-file-name] "
            "[--log-channels log-channel-list] "
            "[--platform platform_name] "
            "[--setsid] "
            "[--named-pipe named-pipe-path] "
            "[--native-regs] "
            "[--attach pid] "
            "[[HOST]:PORT] "
            "[-- PROGRAM ARG1 ARG2 ...]\n", progname, subcommand);
    exit(0);
}

static void
dump_available_platforms (FILE *output_file)
{
    fprintf (output_file, "Available platform plugins:\n");
    for (int i = 0; ; ++i)
    {
        const char *plugin_name = PluginManager::GetPlatformPluginNameAtIndex (i);
        const char *plugin_desc = PluginManager::GetPlatformPluginDescriptionAtIndex (i);

        if (!plugin_name || !plugin_desc)
            break;

        fprintf (output_file, "%s\t%s\n", plugin_name, plugin_desc);
    }

    if ( Platform::GetHostPlatform () )
    {
        // add this since the default platform doesn't necessarily get registered by
        // the plugin name (e.g. 'host' doesn't show up as a
        // registered platform plugin even though it's the default).
        fprintf (output_file, "%s\tDefault platform for this host.\n", Platform::GetHostPlatform ()->GetPluginName ().AsCString ());
    }
}

static lldb::PlatformSP
setup_platform (const std::string &platform_name)
{
    lldb::PlatformSP platform_sp;

    if (platform_name.empty())
    {
        printf ("using the default platform: ");
        platform_sp = Platform::GetHostPlatform ();
        printf ("%s\n", platform_sp->GetPluginName ().AsCString ());
        return platform_sp;
    }

    Error error;
    platform_sp = Platform::Create (lldb_private::ConstString(platform_name), error);
    if (error.Fail ())
    {
        // the host platform isn't registered with that name (at
        // least, not always.  Check if the given name matches
        // the default platform name.  If so, use it.
        if ( Platform::GetHostPlatform () && ( Platform::GetHostPlatform ()->GetPluginName () == ConstString (platform_name.c_str()) ) )
        {
            platform_sp = Platform::GetHostPlatform ();
        }
        else
        {
            fprintf (stderr, "error: failed to create platform with name '%s'\n", platform_name.c_str());
            dump_available_platforms (stderr);
            exit (1);
        }
    }
    printf ("using platform: %s\n", platform_name.c_str ());

    return platform_sp;
}

void
handle_attach_to_pid (GDBRemoteCommunicationServerLLGS &gdb_server, lldb::pid_t pid)
{
    Error error = gdb_server.AttachToProcess (pid);
    if (error.Fail ())
    {
        fprintf (stderr, "error: failed to attach to pid %" PRIu64 ": %s\n", pid, error.AsCString());
        exit(1);
    }
}

void
handle_attach_to_process_name (GDBRemoteCommunicationServerLLGS &gdb_server, const std::string &process_name)
{
    // FIXME implement.
}

void
handle_attach (GDBRemoteCommunicationServerLLGS &gdb_server, const std::string &attach_target)
{
    assert (!attach_target.empty () && "attach_target cannot be empty");

    // First check if the attach_target is convertible to a long. If so, we'll use it as a pid.
    char *end_p = nullptr;
    const long int pid = strtol (attach_target.c_str (), &end_p, 10);

    // We'll call it a match if the entire argument is consumed.
    if (end_p && static_cast<size_t> (end_p - attach_target.c_str ()) == attach_target.size ())
        handle_attach_to_pid (gdb_server, static_cast<lldb::pid_t> (pid));
    else
        handle_attach_to_process_name (gdb_server, attach_target);
}

void
handle_launch (GDBRemoteCommunicationServerLLGS &gdb_server, int argc, const char *const argv[])
{
    Error error;
    error = gdb_server.SetLaunchArguments (argv, argc);
    if (error.Fail ())
    {
        fprintf (stderr, "error: failed to set launch args for '%s': %s\n", argv[0], error.AsCString());
        exit(1);
    }

    unsigned int launch_flags = eLaunchFlagStopAtEntry | eLaunchFlagDebug;

    error = gdb_server.SetLaunchFlags (launch_flags);
    if (error.Fail ())
    {
        fprintf (stderr, "error: failed to set launch flags for '%s': %s\n", argv[0], error.AsCString());
        exit(1);
    }

    error = gdb_server.LaunchProcess ();
    if (error.Fail ())
    {
        fprintf (stderr, "error: failed to launch '%s': %s\n", argv[0], error.AsCString());
        exit(1);
    }
}

Error
writeSocketIdToPipe(Pipe &port_pipe, const std::string &socket_id)
{
    size_t bytes_written = 0;
    // Write the port number as a C string with the NULL terminator.
    return port_pipe.Write(socket_id.c_str(), socket_id.size() + 1, bytes_written);
}

Error
writeSocketIdToPipe(const char *const named_pipe_path, const std::string &socket_id)
{
    Pipe port_name_pipe;
    // Wait for 10 seconds for pipe to be opened.
    auto error = port_name_pipe.OpenAsWriterWithTimeout(named_pipe_path, false,
            std::chrono::seconds{10});
    if (error.Fail())
        return error;
    return writeSocketIdToPipe(port_name_pipe, socket_id);
}

Error
writeSocketIdToPipe(int unnamed_pipe_fd, const std::string &socket_id)
{
#if defined(_WIN32)
    return Error("Unnamed pipes are not supported on Windows.");
#else
    Pipe port_pipe{Pipe::kInvalidDescriptor, unnamed_pipe_fd};
    return writeSocketIdToPipe(port_pipe, socket_id);
#endif
}

void
ConnectToRemote(MainLoop &mainloop, GDBRemoteCommunicationServerLLGS &gdb_server,
        bool reverse_connect, const char *const host_and_port,
        const char *const progname, const char *const subcommand,
        const char *const named_pipe_path, int unnamed_pipe_fd)
{
    Error error;

    if (host_and_port && host_and_port[0])
    {
        // Parse out host and port.
        std::string final_host_and_port;
        std::string connection_host;
        std::string connection_port;
        uint32_t connection_portno = 0;

        // If host_and_port starts with ':', default the host to be "localhost" and expect the remainder to be the port.
        if (host_and_port[0] == ':')
            final_host_and_port.append ("localhost");
        final_host_and_port.append (host_and_port);

        const std::string::size_type colon_pos = final_host_and_port.find (':');
        if (colon_pos != std::string::npos)
        {
            connection_host = final_host_and_port.substr (0, colon_pos);
            connection_port = final_host_and_port.substr (colon_pos + 1);
            connection_portno = StringConvert::ToUInt32 (connection_port.c_str (), 0);
        }

        std::unique_ptr<Connection> connection_up;

        if (reverse_connect)
        {
            // llgs will connect to the gdb-remote client.

            // Ensure we have a port number for the connection.
            if (connection_portno == 0)
            {
                fprintf (stderr, "error: port number must be specified on when using reverse connect");
                exit (1);
            }

            // Build the connection string.
            char connection_url[512];
            snprintf(connection_url, sizeof(connection_url), "connect://%s", final_host_and_port.c_str ());

            // Create the connection.
            connection_up.reset(new ConnectionFileDescriptor);
            auto connection_result = connection_up->Connect (connection_url, &error);
            if (connection_result != eConnectionStatusSuccess)
            {
                fprintf (stderr, "error: failed to connect to client at '%s' (connection status: %d)", connection_url, static_cast<int> (connection_result));
                exit (-1);
            }
            if (error.Fail ())
            {
                fprintf (stderr, "error: failed to connect to client at '%s': %s", connection_url, error.AsCString ());
                exit (-1);
            }
        }
        else
        {
            std::unique_ptr<Acceptor> acceptor_up(Acceptor::Create(final_host_and_port, false, error));
            if (error.Fail())
            {
                fprintf(stderr, "failed to create acceptor: %s", error.AsCString());
                exit(1);
            }
            error = acceptor_up->Listen(1);
            if (error.Fail())
            {
                fprintf(stderr, "failed to listen: %s\n", error.AsCString());
                exit(1);
            }
            const std::string socket_id = acceptor_up->GetLocalSocketId();
            if (!socket_id.empty())
            {
                // If we have a named pipe to write the socket id back to, do that now.
                if (named_pipe_path && named_pipe_path[0])
                {
                    error = writeSocketIdToPipe (named_pipe_path, socket_id);
                    if (error.Fail ())
                        fprintf (stderr, "failed to write to the named pipe \'%s\': %s",
                                 named_pipe_path, error.AsCString());
                }
                // If we have an unnamed pipe to write the socket id back to, do that now.
                else if (unnamed_pipe_fd >= 0)
                {
                    error = writeSocketIdToPipe(unnamed_pipe_fd, socket_id);
                    if (error.Fail())
                        fprintf(stderr, "failed to write to the unnamed pipe: %s",
                                error.AsCString());
                }
            }
            else
            {
                fprintf (stderr, "unable to get the socket id for the listening connection\n");
            }

            Connection* conn = nullptr;
            error = acceptor_up->Accept(false, conn);
            if (error.Fail())
            {
                printf ("failed to accept new connection: %s\n", error.AsCString());
                exit(1);
            }
            connection_up.reset(conn);
        }
        error = gdb_server.InitializeConnection (std::move(connection_up));
        if (error.Fail())
        {
            fprintf(stderr, "Failed to initialize connection: %s\n", error.AsCString());
            exit(-1);
        }
        printf ("Connection established.\n");
    }
}

//----------------------------------------------------------------------
// main
//----------------------------------------------------------------------
int
main_gdbserver (int argc, char *argv[])
{
    Error error;
    MainLoop mainloop;
#ifndef _WIN32
    // Setup signal handlers first thing.
    signal (SIGPIPE, signal_handler);
    MainLoop::SignalHandleUP sighup_handle = mainloop.RegisterSignal(SIGHUP, sighup_handler, error);
#endif
#ifdef __linux__
    // Block delivery of SIGCHLD on linux. NativeProcessLinux will read it using signalfd.
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
#endif

    const char *progname = argv[0];
    const char *subcommand = argv[1];
    argc--;
    argv++;
    int long_option_index = 0;
    int ch;
    std::string platform_name;
    std::string attach_target;
    std::string named_pipe_path;
    std::string log_file;
    StringRef log_channels; // e.g. "lldb process threads:gdb-remote default:linux all"
    int unnamed_pipe_fd = -1;
    bool reverse_connect = false;

    // ProcessLaunchInfo launch_info;
    ProcessAttachInfo attach_info;

    bool show_usage = false;
    int option_error = 0;
#if __GLIBC__
    optind = 0;
#else
    optreset = 1;
    optind = 1;
#endif

    std::string short_options(OptionParser::GetShortOptionString(g_long_options));

    while ((ch = getopt_long_only(argc, argv, short_options.c_str(), g_long_options, &long_option_index)) != -1)
    {
        switch (ch)
        {
        case 0:   // Any optional that auto set themselves will return 0
            break;

        case 'l': // Set Log File
            if (optarg && optarg[0])
                log_file.assign(optarg);
            break;

        case 'c': // Log Channels
            if (optarg && optarg[0])
                log_channels = StringRef(optarg);
            break;

        case 'p': // platform name
            if (optarg && optarg[0])
                platform_name = optarg;
            break;

        case 'N': // named pipe
            if (optarg && optarg[0])
                named_pipe_path = optarg;
            break;

        case 'U': // unnamed pipe
            if (optarg && optarg[0])
                unnamed_pipe_fd = StringConvert::ToUInt32(optarg, -1);

        case 'r':
            // Do nothing, native regs is the default these days
            break;

        case 'R':
            reverse_connect = true;
            break;

#ifndef _WIN32
        case 'S':
            // Put llgs into a new session. Terminals group processes
            // into sessions and when a special terminal key sequences
            // (like control+c) are typed they can cause signals to go out to
            // all processes in a session. Using this --setsid (-S) option
            // will cause debugserver to run in its own sessions and be free
            // from such issues.
            //
            // This is useful when llgs is spawned from a command
            // line application that uses llgs to do the debugging,
            // yet that application doesn't want llgs receiving the
            // signals sent to the session (i.e. dying when anyone hits ^C).
            {
                const ::pid_t new_sid = setsid();
                if (new_sid == -1)
                {
                    const char *errno_str = strerror(errno);
                    fprintf (stderr, "failed to set new session id for %s (%s)\n", LLGS_PROGRAM_NAME, errno_str ? errno_str : "<no error string>");
                }
            }
            break;
#endif

        case 'a': // attach {pid|process_name}
            if (optarg && optarg[0])
                attach_target = optarg;
                break;

        case 'h':   /* fall-through is intentional */
        case '?':
            show_usage = true;
            break;
        }
    }

    if (show_usage || option_error)
    {
        display_usage(progname, subcommand);
        exit(option_error);
    }

    if (!LLDBServerUtilities::SetupLogging(log_file, log_channels, LLDB_LOG_OPTION_PREPEND_TIMESTAMP))
        return -1;

    Log *log(lldb_private::GetLogIfAnyCategoriesSet (GDBR_LOG_VERBOSE));
    if (log)
    {
        log->Printf ("lldb-server launch");
        for (int i = 0; i < argc; i++)
        {
            log->Printf ("argv[%i] = '%s'", i, argv[i]);
        }
    }

    // Skip any options we consumed with getopt_long_only.
    argc -= optind;
    argv += optind;

    if (argc == 0)
    {
        display_usage(progname, subcommand);
        exit(255);
    }

    // Setup the platform that GDBRemoteCommunicationServerLLGS will use.
    lldb::PlatformSP platform_sp = setup_platform (platform_name);

    GDBRemoteCommunicationServerLLGS gdb_server (platform_sp, mainloop);

    const char *const host_and_port = argv[0];
    argc -= 1;
    argv += 1;

    // Any arguments left over are for the program that we need to launch. If there
    // are no arguments, then the GDB server will start up and wait for an 'A' packet
    // to launch a program, or a vAttach packet to attach to an existing process, unless
    // explicitly asked to attach with the --attach={pid|program_name} form.
    if (!attach_target.empty ())
        handle_attach (gdb_server, attach_target);
    else if (argc > 0)
        handle_launch (gdb_server, argc, argv);

    // Print version info.
    printf("%s-%s", LLGS_PROGRAM_NAME, LLGS_VERSION_STR);

    ConnectToRemote(mainloop, gdb_server, reverse_connect,
                    host_and_port, progname, subcommand,
                    named_pipe_path.c_str(), unnamed_pipe_fd);


    if (! gdb_server.IsConnected())
    {
        fprintf (stderr, "no connection information provided, unable to run\n");
        display_usage (progname, subcommand);
        return 1;
    }

    mainloop.Run();
    fprintf(stderr, "lldb-server exiting...\n");

    return 0;
}

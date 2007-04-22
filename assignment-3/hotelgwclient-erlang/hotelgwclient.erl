%%% A simple client for a hotel reservation service using TCP sockets.
%%%
%%% Run with the provided hotelgwclient shell script. Omitting all
%%% command line arguments will print usage information.
%%%
%%%
%%% I tried to do this the Erlang way by using one supervisor process that
%%% spawns application processes and passes on messages between them.
%%%
%%% One application process is responsible for communicating with the user
%%% while the other is responsible for sending requests to the hotel service.
%%%
%%% In the application processes we should not worry about errors. Instead,
%%% when the assumption of a perfect world breaks, the application process
%%% crashes. This causes a message to be sent to the supervisor process which
%%% can then take the appropriate action (e.g. start a new application process
%%% of the same kind).
%%% 
%%%
%%% Copyright 2007, Martijn Vermaat <martijn@vermaat.name>
%%%
%%% All rights reserved.
%%%
%%% Redistribution and use in source and binary forms, with or without
%%% modification, are permitted provided that the following conditions are
%%% met:
%%%
%%% * Redistributions of source code must retain the above copyright notice,
%%%   this list of conditions and the following disclaimer.
%%% * Redistributions in binary form must reproduce the above copyright
%%%   notice, this list of conditions and the following disclaimer in the
%%%   documentation and/or other materials provided with the distribution.
%%% * The name of Martijn Vermaat may not be used to endorse or promote
%%%   products derived from this software without specific prior written
%%%   permission.
%%%
%%% THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
%%% "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
%%% TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
%%% PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
%%% CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
%%% EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
%%% PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
%%% PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
%%% LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
%%% NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
%%% SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


-module(hotelgwclient).

-export([start/0, supervisor/0]).


%% Port to connect to.
-define(PORT, 3242).


%% Start the hotel gateway client.
start() ->
    spawn(?MODULE, supervisor, []).


%% The supervisor process controls two child processes:
%%   - A worker process that can perform requests to the hotel server
%%   - An interface process that communicates with the user and says what
%%     requests to make
%% The supervisor enters a loop and passes on messages from and to the two
%% child processes.
supervisor() ->
    register(supervisor, self()),
    register(worker, spawn_link(fun() -> tcp_worker() end)),
    register(interface, spawn(fun() -> console_interface() end)),
    process_flag(trap_exit, true),
    supervisor_loop().


%% The supervisor loop waits for incoming messages and takes the appropriate
%% action.
supervisor_loop() ->
    receive
        {'EXIT', _Process, Reason} ->
            % An exit message comes from the worker process. Do a small effort
            % to investigate the reason and send an error message to the
            % interface process.
            case Reason of
                {error, Message} when is_atom(Message) ->
                    interface ! {error, atom_to_list(Message)};
                {error, Message} when is_list(Message) ->
                    interface ! {error, Message};
                _ ->
                    interface ! {error, "Connection error"}
            end,
            % We make sure there is always a worker process available (even if
            % the interface process quits on every error with the current
            % implementation).
            register(worker, spawn_link(fun() -> tcp_worker() end));
        {request, Request} ->
            % Pass hotel service request to the worker process.
            worker ! {request, Request};
        {response, Response} ->
            % Pass response messages to the interface process.
            interface ! {response, Response}
    end,
    supervisor_loop().


%% Implements a user interface via the console. Command line arguments are
%% processed and appropriate requests for the hotel service are sent to the
%% supervisor process.
%% We then wait for incoming messages, a response from the hotel service or
%% an error. This interface is a quits after doing one action.
console_interface() ->
    case init:get_plain_arguments() of
        ["book", Host, Type, Guest] ->
            supervisor ! {request, {book, Host, {some, Type}, Guest}};
        ["book", Host, Guest] ->
            supervisor ! {request, {book, Host, none, Guest}};
        ["list", Host] ->
            supervisor ! {request, {list, Host}};
        ["guests", Host] ->
            supervisor ! {request, {guests, Host}};
        _ ->
            print_usage(),
            halt(1)
    end,
    console_loop().


%% Wait for an incoming message, show something to the user, and quit.
console_loop() ->
    receive
        {response, book} ->
            halt();
        {response, {availabilities, Availabilities}} ->
            lists:foreach(
              fun({Type, Price, Number}) ->
                      io:format(
                        "~3s room(s) of type ~s at ~.2f euros per night~n",
                        [integer_to_list(Number), Type, Price])
              end,
              Availabilities),
            halt();
        {response, {guests, Guests}} ->
            lists:foreach(
              fun(Guest) ->
                      io:fwrite(Guest ++ "~n")
              end,
              Guests),
            halt();
        {response, {error, Message}} ->
            io:fwrite(Message ++ "~n"),
            halt(1);
        {error, Message} ->
            io:fwrite("Received error: ~s~n", [Message]),
            halt(1)
    end,
    % This is never reached, because we halt on every response or error. So in
    % fact the console interface does not loop at all.
    console_loop().


%% Print human-readable usage information.
print_usage() ->
    io:fwrite("Usage: hotelgwclient list <hostname>~n"
              "       hotelgwclient book <hostname> [type] <guest>~n"
              "       hotelgwclient guests <hostname>~n").


%% Implements a connection with a hotel service via a TCP socket. After being
%% started, we run a loop to wait for incoming messages.
tcp_worker() ->
    tcp_loop().


%% Wait for incoming request messages and make the request.
tcp_loop() ->
    receive
        {request, {book, Host, Type, Guest}} ->
            book(Host, Type, Guest);
        {request, {list, Host}} ->
            list(Host);
        {request, {guests, Host}} ->
            guests(Host)
    end,
    tcp_loop().


%% Make a request to book a room of type Type for guest Guest. The hotel
%% service can be found at the host with name Host.
%% The hotel service response is send back to the supervisor process.
%%   Host:  string()
%%   Type:  none | {some, string()}
%%   Guest: string()
book(Host, Type, Guest) ->
    Arguments = case Type of
                    {some, T} ->
                        [T, Guest];
                    none ->
                        [Guest]
                end,
    case request(Host, "book", Arguments) of
        {status_ok, _Response} ->
            supervisor ! {response, book};
        {status_error, Message} ->
            supervisor ! {response, {error, Message}}
    end.


%% Request a list of available rooms. The hotel service can be found at the
%% host with name Host.
%% The hotel service response is send back to the supervisor process.
%%   Host:  string()
list(Host) ->
    case request(Host, "list") of
        {status_ok, Response} ->
            supervisor ! {response, {availabilities,
                                     parse_availabilities(Response)}};
        {status_error, Message} ->
            supervisor ! {response, {error, Message}}
    end.


%% Request a list of registered guests. The hotel service can be found at the
%% host with name Host.
%% The hotel service response is send back to the supervisor process.
%%   Host:  string()
guests(Host) ->
    case request(Host, "guests") of
        {status_ok, Response} ->
            supervisor ! {response, {guests, Response}};
        {status_error, Message} ->
            supervisor ! {response, {error, Message}}
    end.


%% Make a request with no arguments.
%% See request/3.
request(Host, Action) ->
    request(Host, Action, []).

%% Make a request Action to host Host with arguments in Arguments and read and
%% return the response.
%%   Host:      string()
%%   Action:    string()
%%   Arguments: [string()]
%%   Return:    {status_ok, [string()]} | {status_error, string()}
request(Host, Action, Arguments) ->
    {ok, Socket} = connect(Host),
    % Action on first line, one argument per line, end with an empty line.
    Request = Action ++ "\n" ++ lists:flatmap(fun(A) -> A ++ "\n" end,
                                              Arguments) ++ "\n",
    ok = gen_tcp:send(Socket, Request),
    Response = read_response(Socket),
    ok = gen_tcp:close(Socket),
    Response.


%% Connect to host with name Host.
%%   Host:   string()
%%   Return: {ok, socket()}
connect(Host) ->
    % Receive lists of characters, do explicit recv calls, and read one line
    % at a time.
    case gen_tcp:connect(Host, ?PORT, [list,
                                       {active, false},
                                       {packet, line}]) of
        {ok, Socket} ->
            {ok, Socket};
        {error, Reason} ->
            % If we can't connect, that's a serious failure.
            exit({error, Reason})
    end.


%% Read a response message from socket Socket and return it.
%%   Socket: socket()
%%   Return: {status_ok, [string()]} | {status_error, string()}
read_response(Socket) ->
    Status = read_status(Socket),
    Content = read_content(Socket),
    case Status of
        {status_ok, _Message} ->
            % We don't care for an Ok message, but we do care for the content.
            {status_ok, Content};
        _ ->
            Status
    end.


%% Read response status line from socket Socket and return it.
%%   Socket: socket()
%%   Return: {status_ok, string()} | {status_error, string()}
read_status(Socket) ->
    {ok, [Status, $\s | Message]} = gen_tcp:recv(Socket, 0),
    {case Status of
         $0 ->
             status_ok;
         $1 ->
             % Application errors are handled gracefully.
             status_error
         % Protocol errors are crashers.
     end,
     % Remove ending newline character.
     lists:delete($\n, Message)}.


%% Read response content from socket Socket and return it.
%%   Socket: socket()
%%   Return: [string()]
%% This function is non-tail-recursive!
read_content(Socket) ->
    % Handle response content a line at a time.
    case gen_tcp:recv(Socket, 0) of
        {ok, "\n"} ->
            [];
        {ok, Line} ->
            [lists:delete($\n, Line) | read_content(Socket)]
    end.


%% Return human-readable representation of given room availabilities.
%%   Availabilities: [string()]
%%   Return:         [{string(), float(), integer()}]
parse_availabilities([]) ->
    [];
parse_availabilities([A|Rest]) ->
    {ok, [Type, Price, Number], _} = io_lib:fread("~s ~f ~d", A),
    [{Type, Price, Number} | parse_availabilities(Rest)].

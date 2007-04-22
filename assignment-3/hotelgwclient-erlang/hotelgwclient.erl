%%% A simple client for a hotel reservation service using TCP sockets.
%%%
%%% Run with the provided hotelgwclient shell script. Omitting all
%%% command line arguments will print usage information.
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

-export([start/0, start/1]).


%% Port to connect to
-define(PORT, 3242).


%% Start the client with no command line arguments.
start() ->
    start([]).

%% Start the client with given command line arguments.
%%   Arguments: [string()]
start(Arguments) ->
    case catch case Arguments of
                   ["book", Host, Type, Guest] ->
                       book(Host, {some, Type}, Guest);
                   ["book", Host, Guest] ->
                       book(Host, none, Guest);
                   ["list", Host] ->
                       list(Host);
                   ["guests", Host] ->
                       guests(Host);
                   _ ->
                       usage_error()
               end
        of
        % Most common error is a bad result match of a gen_tcp function; we
        % can catch these this way
        {'EXIT', {{badmatch, {error, Reason}}, _Where}} ->
            io:fwrite("Error: ~w\n", [Reason]),
            halt(1);
        ok ->
            ok
    end.


%% Make a request to book a room of type Type for guest Guest. The hotel
%% service can be found at the host with name Host.
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
            ok;
        {status_error, Message} ->
            io:fwrite(Message ++ "~n"),
            halt(1)
    end.


%% Request a list of available rooms. The hotel service can be found at the
%% host with name Host.
%%   Host:  string()
list(Host) ->
    case request(Host, "list") of
        {status_ok, Response} ->
            print_availabilities(Response);
        {status_error, Message} ->
            io:fwrite(Message ++ "~n"),
            halt(1)
    end.


%% Request a list of registered guests. The hotel service can be found at the
%% host with name Host.
%%   Host:  string()
guests(Host) ->
    case request(Host, "guests") of
        {status_ok, Response} ->
            lists:foreach(fun(R) -> io:fwrite(R ++ "~n") end, Response);
        {status_error, Message} ->
            io:fwrite(Message ++ "~n"),
            halt(1)
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
%%   Return:    {status_ok, [string()]}, {status_error, string()}
request(Host, Action, Arguments) ->
    {ok, Socket} = connect(Host),
    % Action on first line, one argument per line, end with an empty line
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
    % at a time
    {ok, _Socket} = gen_tcp:connect(Host, ?PORT, [list,
                                                  {active, false},
                                                  {packet, line}]).


%% Read a response message from socket Socket and return it.
%%   Socket: socket()
%%   Return: {status_ok, [string()]}, {status_error, string()}
read_response(Socket) ->
    Status = read_status(Socket),
    Content = read_content(Socket),
    case Status of
        {status_ok, _Message} ->
            % We don't care for an Ok message, but we do care for the content
            {status_ok, Content};
        _ ->
            Status
    end.


%% Read response status line from socket Socket and return it.
%%   Socket: socket()
%%   Return: {status_ok, string()}, {status_error, string()}
read_status(Socket) ->
    {ok, [Status, $\s | Message]} = gen_tcp:recv(Socket, 0),
    {case Status of
         $0 ->
             status_ok;
         $1 ->
             % Application errors are handled gracefully
             status_error
         % Protocol errors are crashers
     end,
     % Remove ending newline character
     lists:delete($\n, Message)}.


%% Read response content from socket Socket and return it.
%%   Socket: socket()
%%   Return: [string()]
%% This function is not tail-recursive!
read_content(Socket) ->
    % Handle response content a line at a time
    case gen_tcp:recv(Socket, 0) of
        {ok, "\n"} ->
            [];
        {ok, Line} ->
            [lists:delete($\n, Line) | read_content(Socket)]
    end.


%% Print human-readable representation of given room availabilities.
%%   Availabilities: [string()]
print_availabilities([]) ->
    ok;
print_availabilities([A|Rest]) ->
    {ok, [Type, Price, Number], _} = io_lib:fread("~s ~f ~d", A),
    io:format("~3s room(s) of type ~s at ~.2f euros per night~n", [integer_to_list(Number), Type, Price]),
    print_availabilities(Rest).


%% Print human-readable usage information.
usage_error() ->
    io:fwrite("Usage: hotelgwclient list <hostname>~n"
              "       hotelgwclient book <hostname> [type] <guest>~n"
              "       hotelgwclient guests <hostname>~n"),
    halt(1).

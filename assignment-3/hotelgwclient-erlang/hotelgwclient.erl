-module(hotelgwclient).

-export([start/0, start/1]).


-define(PORT, 3242).


start() ->
    start([]).

start(Arguments) ->
    case Arguments of
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
    end.


book(Host, Type, Guest) ->
    {ok, Socket} = connect(Host),
    {ok, _Response} = request(Socket, "book", case Type of
                                                  {some, T} ->
                                                     [T, Guest];
                                                  none ->
                                                      [Guest]
                                              end),
    ok = gen_tcp:close(Socket).


list(Host) ->
    {ok, Socket} = connect(Host),
    {ok, Response} = request(Socket, "list"),
    ok = gen_tcp:close(Socket),
    lists:foreach(fun(R) -> io:fwrite(R ++ "\n") end, Response).


guests(Host) ->
    {ok, Socket} = connect(Host),
    {ok, Response} = request(Socket, "guests"),
    ok = gen_tcp:close(Socket),
    lists:foreach(fun(R) -> io:fwrite(R ++ "\n") end, Response).


connect(Host) ->
    {ok, _Socket} = gen_tcp:connect(Host, ?PORT, [list, {active, false}, {packet, line}]).


request(Socket, Action) ->
    request(Socket, Action, []).

request(Socket, Action, Arguments) ->
    Request = Action ++ "\n" ++ lists:flatmap(fun(A) -> A ++ "\n" end, Arguments) ++ "\n",
    ok = gen_tcp:send(Socket, Request),
    {ok, [$0 | _]} = gen_tcp:recv(Socket, 0),
    {ok, read_response(Socket)}.


read_response(Socket) ->
    case gen_tcp:recv(Socket, 0) of
        {ok, "\n"} ->
            [];
        {ok, Line} ->
            [lists:delete($\n, Line) | read_response(Socket)]  % non-tail-recursive
    end.


usage_error() ->
    io:fwrite("Usage: hotelgwclient list <hostname>~n"
              "       hotelgwclient book <hostname> [type] <guest>~n"
              "       hotelgwclient guests <hostname>~n"),
    halt(1).

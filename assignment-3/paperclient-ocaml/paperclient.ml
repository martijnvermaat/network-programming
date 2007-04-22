(*
  A simple client for a paper storage system using SUN RPC implemented in
  OCaml.

  Run with no command line arguments for usage information.


  The Rpc module by Gerd Stolpmann is part of the Ocamlnet suite of
  networking modules for OCaml.

  This implementation behaves as much as possible in the same way as the
  implementation in C. Due to the difference in handling names, duplicate
  names in the XDR definition need to be mapped to other names. This is what
  the paperstorage.x.diff does.

  Besides standard OCaml bytecode executable file, the Makefile has another
  rule, resulting in papertop. This is an OCaml toplevel linked with the all
  required modules to be able to use this module interactively in an easy way.
  In the toplevel we need to add a few directories to the search path to be
  able to load all modules:
    # #directory "+rpc";;
    # #directory "+extlib";;
    # #directory "+equeue";;


  Copyright 2007, Martijn Vermaat <martijn@vermaat.name>

  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  * The name of Martijn Vermaat may not be used to endorse or promote products
    derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*)


open Paperstorage_aux

module CLNT = Paperstorage_clnt.PAPERSTORAGE_PROG.PAPERSTORAGE_VERS


(*
  Create our RPC client.
*)
let create_client hostname =
  try
    CLNT.create_portmapped_client hostname Rpc.Tcp
  with
      Unix.Unix_error (e, _, _) -> failwith (Unix.error_message e)
    | Rpc.Rpc_server e          -> failwith (Printexc.to_string
                                               (Rpc.Rpc_server e))


(*
  Call remote procedure.
*)
let call_proc proc client arguments =
  try
    proc client arguments
  with
      Unix.Unix_error (e, _, _) -> failwith (Unix.error_message e)
    | Rpc.Rpc_server e          -> failwith (Printexc.to_string
                                               (Rpc.Rpc_server e))


(*
  Add our paper to the server.
*)
let do_add hostname author title filename =

  let paper =
    let content =
      try
        Std.input_file ~bin:true filename
      with
          Sys_error e -> failwith ("Cannot read from file: " ^ e)
    in
    {paper = {
       number = None;
       author = author;
       title  = title;
       content = Some content}}
  in

  let client = create_client hostname in    
  let result = call_proc CLNT.add_proc client paper
  in

    Rpc_client.shut_down client;

    match result with
        `status_success {number = Some n} ->
          print_endline ("Added paper " ^ (string_of_int n))
      | `status_success _ -> failwith "Added paper but received no number"
      | `status_failure m -> failwith m


(*
  Retreive paper details from the server.
*)
let do_get hostname number include_content =

  let param =
    let n =
      try
        int_of_string number
      with Failure _ -> failwith ("Not a valid paper number: " ^ number)
    in
      {document_number = n;
       representation  = if include_content then detailed else sparse}
  in

  let client = create_client hostname in    
  let result = call_proc CLNT.get_proc client param
  in

    Rpc_client.shut_down client;

    result


(*
  Retreive paper author and name from the server.
*)
let do_details hostname number =

  let result = do_get hostname number false in
    match result with
        `status_success {author = author; title = title} ->
          print_endline ("Author: ``" ^ author ^ "''");
          print_endline ("Title:  ``" ^ title ^ "''")
      | `status_failure m -> failwith m


(*
  Retreive paper content from the server.
*)
let do_fetch hostname number =

  let result = do_get hostname number true in
    match result with
        `status_success {content = Some s} -> print_string s
      | `status_success _ -> failwith "Received paper without content"
      | `status_failure m -> failwith m


(*
  Retreive paper listing from the server.
*)
let do_list hostname =

  let client = create_client hostname in    
  let result = call_proc CLNT.list_proc client ()
  in

    Rpc_client.shut_down client;

    let rec print_papers paper =
      match paper with
          None -> ()
        | Some {item = {number = Some number;
                        author = author;
                        title = title};
                next = p} ->
            print_endline ("Paper " ^ (string_of_int number));
            print_endline ("  Author: ``" ^ author ^ "''");
            print_endline ("  Title:  ``" ^ title ^ "''");
            print_papers p
        | Some _ -> failwith "Received paper without number"
    in
      print_papers result.papers


(*
  Read command line arguments and perform the appropriate action.
*)
let paper_client () =

  let usage_error () =
    print_string "usage: ";
    print_endline
      "paperclient add <hostname> <author> <title> <filename.{pdf|doc}>";
    print_endline "       paperclient details <hostname> <number>";
    print_endline "       paperclient fetch <hostname> <number>";
    print_endline "       paperclient list <hostname>";
    exit 1
  in

  (if Array.length Sys.argv < 2 then usage_error ());

  match Sys.argv.(1) with
      "add" ->
        (if Array.length Sys.argv < 6 then usage_error ());
        do_add Sys.argv.(2) Sys.argv.(3) Sys.argv.(4) Sys.argv.(5)
    | "details" ->
        (if Array.length Sys.argv < 4 then usage_error ());
        do_details Sys.argv.(2) Sys.argv.(3)
    | "fetch" ->
        (if Array.length Sys.argv < 4 then usage_error ());
        do_fetch Sys.argv.(2) Sys.argv.(3)
    | "list" ->
        (if Array.length Sys.argv < 3 then usage_error ());
        do_list Sys.argv.(2)
    | _     ->
        usage_error ()


(*
  Start main program.
*)
let _ =
  try paper_client () with Failure s -> print_endline s; exit 1

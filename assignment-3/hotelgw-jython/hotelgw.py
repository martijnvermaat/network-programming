"""
This program serves as a gateway between a client and a hotel reservation
server.

Communication with the client is done over a TCP socket while communication
with the server is done via Java RMI.

Some Java classes are needed to run the gateway, edit the 'hotelgw' shell
script to your liking. See the Java HotelGateway documentation for more info.


Copyright 2007, Martijn Vermaat (martijn@vermaat.name)

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
* The name of Martijn Vermaat may not be used to endorse or promote products
  derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""


import sys
import socket

import NotAvailableException
from java.rmi.Naming import lookup
from java.rmi import RemoteException
from java.net import BindException


SERVER = "localhost"
GATEWAY_PORT = 3242

STATUS_OK                = 0
STATUS_APPLICATION_ERROR = 1
STATUS_PROTOCOL_ERROR    = 2


class RemoteError(Exception):

    """
    Exception thrown on RMI failure.
    """

    # Our own exception doesn't need anything fancy.
    pass


def create_hotel():

    """
    Lookup the hotel object on the remote server.

    Return hotel object.
    """

    # If the lookup fails, we raise a RemoteError with the reason
    try:
        return lookup("rmi://%s/HotelService" % SERVER)
    except RemoteException, e:
        if e.getCause() == None:
            m = e.getMessage()
        else:
            m = e.getCause().getMessage()
        raise RemoteError("Error contacting hotel service: %s" %
                          m.replace("\n", " "))


def list(client):

    """
    Handle a list request. Query hotel server for availability of room types
    and send this to the client.

    Parameters:
        client    Filehandle to read the request from and write the response
                  to.
    """

    # Read closing empty line
    if client.readline() == "":
        client.write("%d Malformed request: premature end of request\n\n" %
                     STATUS_PROTOCOL_ERROR)
        return

    try:
        hotel = create_hotel()
    except RemoteError, e:
        client.write("%d %s\n\n" % (STATUS_APPLICATION_ERROR, e))
        return

    client.write("%d Ok\n" % STATUS_OK)

    for a in hotel.availableRooms():
        client.write("%d %f %d\n" % (a.getType(),
                                     a.getPrice(),
                                     a.getNumberOfRooms()))

    # End response with an empty line
    client.write("\n")


def guests(client):

    """
    Handle a guests request. Query hotel server for registered guests and send
    this to the client.

    Parameters:
        client    Filehandle to read the request from and write the response
                  to.
    """

    # Read closing empty line
    if client.readline() == "":
        client.write("%d Malformed request: premature end of request\n\n" %
                     STATUS_PROTOCOL_ERROR)
        return

    try:
        hotel = create_hotel()
    except RemoteError, e:
        client.write("%d %s\n\n" % (STATUS_APPLICATION_ERROR, e))
        return

    client.write("%d Ok\n" % STATUS_OK)

    for g in hotel.registeredGuests():
        client.write("%s\n" % g)

    # End response with an empty line
    client.write("\n")


def book(client):

    """
    Handle a book request. Query hotel server to book a room and send an Ok
    response to the client if nothing went wrong.

    Parameters:
        client    Filehandle to read the request from and write the response
                  to.
    """

    # Assume we have a type parameter
    type = client.readline()

    if type == "":
        client.write("%d Malformed request: premature end of request\n\n" %
                     STATUS_PROTOCOL_ERROR)
        return
    elif type == "\n":
        client.write("%d Malformed request: not enough parameters\n\n" %
                     STATUS_PROTOCOL_ERROR)
        return

    # Assume guest is the next parameter
    guest = client.readline()

    if guest == "":
        client.write("%d Malformed request: premature end of request\n\n" %
                     STATUS_PROTOCOL_ERROR)
        return
    elif guest == "\n":
        # There was no second parameter, the first must have been guest
        guest = type
        type = None
    else:
        # Read closing empty line
        if client.readline() == "":
            client.write("%d Malformed request: premature end of request\n\n"
                         % STATUS_PROTOCOL_ERROR)
            return

    try:
        hotel = create_hotel()
    except RemoteError, e:
        client.write("%d %s\n\n" % (STATUS_APPLICATION_ERROR, e))
        return

    # Book a room of given type or of any type
    if type == None:

        try:
            # We need to strip the ending \n from guest
            hotel.bookRoom(guest[:-1])
        except java.NotAvailableException, e:
            client.write("%d %s\n\n" % (STATUS_APPLICATION_ERROR,
                                        e.getMessage().replace("\n", " ")))
            return

    else:

        try:
            # We need to strip the ending \n from type and guest
            hotel.bookRoom(int(type), guest[:-1])
        except ValueError:
            client.write("%d Type must be a number\n\n" %
                         STATUS_APPLICATION_ERROR)
            return
        except NotAvailableException, e:
            client.write("%d %s\n\n" % (STATUS_APPLICATION_ERROR,
                                        e.getMessage().replace("\n", " ")))
            return

    # Booking a room does not return anything special, just say Ok
    client.write("%d Ok\n" % STATUS_OK)

    # End response with an empty line
    client.write("\n")


def handle_request(client):

    """
    Handle an incoming client request and send the appropriate response.

    Parameters:
        client    Filehandle to read the request from and write the response
                  to.
    """

    procedures = {"list"   : list,
                  "guests" : guests,
                  "book"   : book}

    # First line of request contains procedure name (we need to strip \n)
    procedure = client.readline()[:-1]

    try:
        procedures[procedure](client)
    except KeyError:
        client.write("%d Malformed request: unknown procedure\n\n" %
                     STATUS_PROTOCOL_ERROR)

    client.close()


def main():

    """
    Main gateway program.
    """

    # Create listening socket
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Set REUSEADDR socket option to allow rebinding quickly
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    try:
        s.bind(("", GATEWAY_PORT))
        s.listen(5)
    except BindException, e:
        print "Socket error: %s" % e.getMessage()
        sys.exit(1)

    while 1:
        # This is a iterative server, but it would not take much effort to
        # change it to use e.g. a thread per request.
        # By the way, we use a file object to read and write to the socket
        # in an easy way.
        client, _ = s.accept()
        try:
            handle_request(client.makefile("rw"))
        except IOError:
            # What can we do? Do our best for the next customer...
            pass


if __name__ == "__main__":
    main()

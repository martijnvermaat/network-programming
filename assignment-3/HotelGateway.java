import java.net.MalformedURLException;
import java.rmi.NotBoundException;
import java.rmi.RemoteException;
import java.rmi.Naming;
import java.net.ServerSocket;
import java.net.Socket;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.util.List;
import java.util.ArrayList;
import java.util.Set;
import java.util.SortedSet;
import java.util.TreeSet;
import java.io.IOException;


public class HotelGateway {

    private int STATUS_OK = 0;
    private int STATUS_APPLICATION_ERROR = 1;
    private int STATUS_PROTOCOL_ERROR = 2;

    private int PORT = 3242;  // listen socket
    private String HOSTNAME = "localhost";  // RMI server

    private Hotel hotel;

    public HotelGateway() {

        ServerSocket serverSocket = null;
        Socket socket = null;

        BufferedReader in;
        DataOutputStream out;

        try { // TODO: if a client disconnects, we shouldn't crash the entire gateway
            serverSocket = new ServerSocket(PORT);
        } catch (IOException e) {
            System.err.println("Socket error");
            System.exit(1);
        }

        while (true) {

            try {
                socket = serverSocket.accept();

                in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                out = new DataOutputStream(socket.getOutputStream());

                handleRequest(in, out);

                // TODO: socket sluiten
                in.close();
                out.close();
                socket.close();

            } catch (IOException e) {
                System.err.println("Socket error");
            }

        }

    }


    private void handleRequest(BufferedReader request, DataOutputStream response) throws IOException {

        String procedure;
        String parameter;
        List<String> parameters;

        procedure = request.readLine();

        if (procedure == null) {
            sendResponse(response, STATUS_PROTOCOL_ERROR, "Malformed request: no procedure");
            return;
        }

        parameters = new ArrayList<String>();

        while ((parameter = request.readLine()) != null) {
            if (parameter.equals("")) {
                break;
            }
            parameters.add(parameter);
        }

        if (parameter == null) {
            sendResponse(response, STATUS_PROTOCOL_ERROR, "Malformed request: premature end of request");
            return;
        }

        if (procedure.equals("book")) {
            if (parameters.size() < 1) {
                sendResponse(response, STATUS_PROTOCOL_ERROR, "Malformed request: not enough parameters");
                return;
            }
            if (parameters.size() < 2) {
                handleBookRequest(response, parameters.get(0));
            } else {
                try {
                    handleBookRequest(response, Integer.parseInt(parameters.get(0)), parameters.get(1));
                } catch (NumberFormatException e) {
                    sendResponse(response, STATUS_APPLICATION_ERROR, "Type must be a number");
                }
            }
        } else if (procedure.equals("list")) {
            handleListRequest(response);
        } else if (procedure.equals("guests")) {
            handleGuestsRequest(response);
        } else {
            sendResponse(response, STATUS_PROTOCOL_ERROR, "Malformed request: unknown procedure");
            return;
        }

    }


    private void handleBookRequest(DataOutputStream response, int type, String guest) throws IOException {

        try {

            Hotel hotel = (Hotel) Naming.lookup("rmi://" + HOSTNAME + "/HotelService");
            try {
                hotel.bookRoom(type, guest);
            } catch (NotAvailableException e) {
                sendResponse(response, STATUS_APPLICATION_ERROR, "Unfortunately we're full");
                return;
            }

        } catch (MalformedURLException e) {
            sendResponse(response, STATUS_APPLICATION_ERROR, "Invalid host: " + HOSTNAME);
            return;
        } catch (NotBoundException e) {
            sendResponse(response, STATUS_APPLICATION_ERROR, "Hotel service not found");
            return;
        } catch (RemoteException e) {
            sendResponse(response, STATUS_APPLICATION_ERROR, "Error contacting hotel service : " + e.getMessage().replaceAll("\n", "; "));
            return;
        }

        sendResponse(response, STATUS_OK, "Ok");

    }


    private void handleBookRequest(DataOutputStream response, String guest) throws IOException {

        try {

            Hotel hotel = (Hotel) Naming.lookup("rmi://" + HOSTNAME + "/HotelService");
            try {
                hotel.bookRoom(guest);
            } catch (NotAvailableException e) {
                sendResponse(response, STATUS_APPLICATION_ERROR, "Unfortunately we're full");
                return;
            }

        } catch (MalformedURLException e) {
            sendResponse(response, STATUS_APPLICATION_ERROR, "Invalid host: " + HOSTNAME);
            return;
        } catch (NotBoundException e) {
            sendResponse(response, STATUS_APPLICATION_ERROR, "Hotel service not found");
            return;
        } catch (RemoteException e) {
            sendResponse(response, STATUS_APPLICATION_ERROR, "Error contacting hotel service : " + e.getMessage().replaceAll("\n", "; "));
            return;
        }

        sendResponse(response, STATUS_OK, "Ok");

    }


    private void handleListRequest(DataOutputStream response) throws IOException {
        Set<Availability> availables = null;

        try {

            Hotel hotel = (Hotel) Naming.lookup("rmi://" + HOSTNAME + "/HotelService");
            availables = hotel.availableRooms();

        } catch (MalformedURLException e) {
            sendResponse(response, STATUS_APPLICATION_ERROR, "Invalid host: " + HOSTNAME);
            return;
        } catch (NotBoundException e) {
            sendResponse(response, STATUS_APPLICATION_ERROR, "Hotel service not found");
            return;
        } catch (RemoteException e) {
            sendResponse(response, STATUS_APPLICATION_ERROR, "Error contacting hotel service : " + e.getMessage().replaceAll("\n", "; "));
            return;
        }
        
        
        if (availables == null || availables.isEmpty()) {
            sendResponse(response, STATUS_OK, "None");
        } else {

            SortedSet<Availability> sorted_availables = new TreeSet<Availability>(availables);
            List<String> responseList = new ArrayList<String>();

            for (Availability a : sorted_availables) {
                responseList.add(Integer.toString(a.getType())
                                 + " "
                                 + String.format("%.2f", a.getPrice()) // TODO make sure we always use a dot (.) as separator
                                 + " "
                                 + Integer.toString(a.getNumberOfRooms()));
            }
            sendResponse(response, STATUS_OK, "Ok", responseList);
        }

    }


    private void handleGuestsRequest(DataOutputStream response) throws IOException {
        Set<String> registeredGuests = null;
        
        try {

            Hotel hotel = (Hotel) Naming.lookup("rmi://" + HOSTNAME + "/HotelService");
            registeredGuests = hotel.registeredGuests();

        } catch (MalformedURLException e) {
            sendResponse(response, STATUS_APPLICATION_ERROR, "Invalid host: " + HOSTNAME);
            return;
        } catch (NotBoundException e) {
            sendResponse(response, STATUS_APPLICATION_ERROR, "Hotel service not found");
            return;
        } catch (RemoteException e) {
            sendResponse(response, STATUS_APPLICATION_ERROR, "Error contacting hotel service : " + e.getMessage().replaceAll("\n", "; "));
            return;
        }
        
        
        if (registeredGuests == null || registeredGuests.isEmpty()) {
            sendResponse(response, STATUS_OK, "None");
        } else {

            SortedSet<String> sorted_registeredGuests = new TreeSet<String>(registeredGuests);
            List<String> responseList = new ArrayList<String>();

            for (String s : sorted_registeredGuests) {
                responseList.add(s);
            }
            sendResponse(response, STATUS_OK, "Ok", responseList);
        }
    }


    private void sendResponse(DataOutputStream response, int status, String description, List<String> data) throws IOException {

        response.writeBytes(Integer.toString(status) + " " + description);
        response.writeByte('\n');

        for (String s : data) {
            response.writeBytes(s);
            response.writeByte('\n');
        }

        response.writeByte('\n');

    }


    private void sendResponse(DataOutputStream response, int status, String description) throws IOException {

        sendResponse(response, status, description, new ArrayList<String>());

    }


/*
    public Set<Availability> availableRooms() throws RemoteException {
        return hotel.availableRooms();
    }


    public Set<String> registeredGuests() throws RemoteException {
        return hotel.registeredGuests();
    }
*/


    public static void main(String[] args) {

        HotelGateway gateway = new HotelGateway();

    }


}

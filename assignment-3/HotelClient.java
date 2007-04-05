import java.net.MalformedURLException;
import java.rmi.NotBoundException;
import java.rmi.RemoteException;
import java.rmi.Naming;
import java.util.Set;
import java.util.SortedSet;
import java.util.TreeSet;


public class HotelClient {


    Hotel hotel;


    public HotelClient(String host) throws MalformedURLException, NotBoundException, RemoteException {
        hotel = (Hotel) Naming.lookup("rmi://" + host + "/HotelService");
    }


    public Set<Availability> availableRooms() throws RemoteException {
        return hotel.availableRooms();
    }


    public Set<String> registeredGuests() throws RemoteException {
        return hotel.registeredGuests();
    }


    public void bookRoom(String guest) throws NotAvailableException, RemoteException {
        hotel.bookRoom(guest);
    }


    public void bookRoom(int type, String guest) throws NotAvailableException, RemoteException {
        hotel.bookRoom(type, guest);
    }


    private static void usageError() {
        System.out.println("usage: hotelclient list <hostname>");
        System.out.println("       hotelclient book <hostname> [type] <guest>");
        System.out.println("       hotelclient guests <hostname>");
        System.exit(1);
    }


    private static void list(String host) {

        Set<Availability> availables = null;

        try {

            HotelClient c = new HotelClient(host);

            availables = c.availableRooms();
         } catch (MalformedURLException e) {
            System.err.println("Invalid host: " + host);
            System.exit(1);
        } catch (NotBoundException e) {
            System.err.println("Hotel service not found");
            System.exit(1);
        } catch (RemoteException e) {
            System.err.println("Error contacting hotel service: " + e.getMessage());
            System.exit(1);
        }


        if (availables == null || availables.isEmpty()) {

            System.out.println("No available rooms");

        } else {

            System.out.println("Available rooms:");

            SortedSet<Availability> sorted_availables = new TreeSet<Availability>(availables);

            for (Availability a : sorted_availables) {
                System.out.printf("  %3d rooms of type %3d at %.2f euros per night\n",a.getNumberOfRooms(), a.getType(), a.getPrice());
            }

        }

        System.exit(0);

    }


    private static void book(String host, String guest) {

        try {

            HotelClient c = new HotelClient(host);
            try {
                c.bookRoom(guest);
            } catch (NotAvailableException e) {
                System.err.println("Unfortunately we're full");
                System.exit(1);
            }

        } catch (MalformedURLException e) {
            System.err.println("Invalid host: " + host);
            System.exit(1);
        } catch (NotBoundException e) {
            System.err.println("Hotel service not found");
            System.exit(1);
        } catch (RemoteException e) {
            System.err.println("Error contacting hotel service: " + e.getMessage());
            System.exit(1);
        }

        System.exit(0);

    }


    private static void book(String host, int type, String guest) {

        try {

            HotelClient c = new HotelClient(host);
            try {
                c.bookRoom(type, guest);
            } catch (NotAvailableException e) {
                System.err.println("No rooms of type " + type + " available");
                System.exit(1);
            }

        } catch (MalformedURLException e) {
            System.err.println("Invalid host: " + host);
            System.exit(1);
        } catch (NotBoundException e) {
            System.err.println("Hotel service not found");
            System.exit(1);
        } catch (RemoteException e) {
            System.err.println("Error contacting hotel service: " + e.getMessage());
            System.exit(1);
        }

        System.exit(0);

    }


    private static void guests(String host) {

        Set<String> guests = null;

        try {

            HotelClient c = new HotelClient(host);

            guests = c.registeredGuests();

        } catch (MalformedURLException e) {
            System.err.println("Invalid host: " + host);
            System.exit(1);
        } catch (NotBoundException e) {
            System.err.println("Hotel service not found");
            System.exit(1);
        } catch (RemoteException e) {
            System.err.println("Error contacting hotel service: " + e.getMessage());
            System.exit(1);
        }


        if (guests == null || guests.isEmpty()) {

            System.out.println("No registered guests");

        } else {

            System.out.println("Registered guests:");

            SortedSet<String> sorted_guests = new TreeSet<String>(guests);

            for (String g : sorted_guests) {
                System.out.println("  " + g);
            }

        }

        System.exit(0);

    }


    public static void main(String[] args) {

        if (args.length < 1) {
            usageError();
        }

        if (args[0].equals("list")) {

            if (args.length < 2) {
                usageError();
            }

            list(args[1]);

        } else if (args[0].equals("book")) {

            if (args.length < 3) {
                usageError();
            }
            if (args.length < 4) {
                book(args[1], args[2]);
            } else {
                try {
                    book(args[1], Integer.parseInt(args[2]), args[3]);
                } catch (NumberFormatException e) {
                    usageError();
                }
            }

        } else if (args[0].equals("guests")) {

            if (args.length < 2) {
                usageError();
            }

            guests(args[1]);

        }

        usageError();

    }


}

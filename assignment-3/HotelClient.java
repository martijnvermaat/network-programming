import java.net.MalformedURLException;
import java.rmi.NotBoundException;
import java.rmi.RemoteException;
import java.rmi.Naming;
import java.util.Set;


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


    private static void usage() {
        System.out.println("usage: blaaaaa");
        System.exit(1);
    }


    private static void list(String host) {

        Set<Availability> set = null;

        try {

            HotelClient c = new HotelClient(host);

            set = c.availableRooms();
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


        if (set == null || set.isEmpty()) {

            System.out.println("No available rooms");

        } else {

            System.out.println("Available rooms:");

            for (Availability a : set) {
                System.out.println(a.getNumberOfRooms() + " rooms of type " +
                                   a.getType() + " at " +
                                   a.getPrice() + " euros per night");
            }

        }

    }


    private static void book(String host, String guest) {
        try {
            HotelClient c = new HotelClient(host);
            try {
                c.bookRoom(guest);
            } catch (NotAvailableException e) {
                System.err.println("Unfortunately we're full");
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
    }


    private static void book(String host, int type, String guest) {
        try {
            HotelClient c = new HotelClient(host);
            try {
                c.bookRoom(type, guest);
            } catch (NotAvailableException e) {
                System.err.println("Unfortunately we're full");
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

            for (String g : guests) {
                System.out.println(g);
            }

        }

    }


    public static void main(String[] args) {

        if (args.length < 1) {
            usage();
        }

        if (args[0].equals("list")) {

            if (args.length < 2) {
                usage();
            }

            list(args[1]);

        } else if (args[0].equals("book")) {

            if (args.length < 3) {
                usage();
            }
            if (args.length < 4) {
                book(args[1], args[2]);
            } else {
                try {
                    book(args[1], Integer.parseInt(args[2]), args[3]);
                } catch (NumberFormatException e) {
                    usage();
                }
            }

        } else if (args[0].equals("guests")) {

            if (args.length < 2) {
                usage();
            }

            guests(args[1]);

        }

    }


}

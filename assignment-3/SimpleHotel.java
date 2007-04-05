import java.rmi.server.UnicastRemoteObject;
import java.rmi.RemoteException;
import java.util.Set;
import java.util.HashSet;


class SimpleHotel extends UnicastRemoteObject implements Hotel {


    private Set<Room> rooms;


    synchronized public void bookRoom(int roomType, String guest)
        throws NotAvailableException, RemoteException {

        boolean booked = false;

        for (Room room : this.rooms) {

            if (room.getRoomType().getType() == roomType) {
                if (room.isAvailable()) {
                    try {
                        room.book(guest);
                        booked = true;
                        break;
                    } catch (NotAvailableException e) {
                        System.err.println("HotelImpl exception: available room is booked");
                    }
                }
            }

        }

        if (!booked) {
            throw new NotAvailableException();
        }

    }


    public Set<String> registeredGuests()
        throws RemoteException {

        Set<String> guests = new HashSet<String>();

        for (Room room : this.rooms) {
            if (!room.isAvailable()) {
                try {
                    guests.add(room.getGuest());
                } catch (NotBookedException e) {
                    System.err.println("HotelImpl exception: reserved room is not reserved");
                }
            }
        }

        return guests;

    }


    public Set<Availability> availableRooms()
        throws RemoteException {

        Set<Availability> availables = new HashSet<Availability>();
        boolean typeFound;

        for (Room room : this.rooms) {

            if (room.isAvailable()) {

                typeFound = false;

                for (Availability a : availables) {
                    if (a.getType() == room.getRoomType().getType()) {
                        a.setNumberOfRooms(a.getNumberOfRooms() + 1);
                        typeFound = true;
                        break;
                    }
                }

                if (!typeFound) {
                    availables.add(new Availability(room.getRoomType().getType(), room.getRoomType().getPrice(), 1));
                }

            }

        }

        return availables;

    }


    public SimpleHotel()
        throws RemoteException {

        rooms = new HashSet<Room>();

        /* Our hotel must have some rooms */

        RoomType presidential = new RoomType(150);
        RoomType honeymoon = new RoomType(120);
        RoomType backpack = new RoomType(100);

        for (int i = 0; i < 10; i++) {
            rooms.add(new Room(presidential));
        }

        for (int i = 0; i < 20; i++) {
            rooms.add(new Room(honeymoon));
        }

        for (int i = 0; i < 40; i++) {
            rooms.add(new Room(backpack));
        }

    }


}

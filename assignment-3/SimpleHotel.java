import java.rmi.server.UnicastRemoteObject;
import java.rmi.RemoteException;
import java.util.Set;
import java.util.HashSet;


class SimpleHotel extends UnicastRemoteObject implements Hotel {


    private Set<Room> rooms;


    public void bookRoom(int roomType, String guest)
        throws NotAvailableException, RemoteException {

        boolean booked = false;

        for (Room room : this.rooms) {

            if (room.getRoomType().getType() == roomType) {

                /*
                  The hack here is that we just try to book the room without
                  checking if it is available first. Since room.book() is
                  synchronized, this will just work.
                  We might get a NotAvailableException, which is kind of
                  expected behaviour in this code. We just try to book another
                  room.
                  In a concurrent environment, whoever is first gets the room.
                  The set of rooms itself is never modified, so we don't have
                  to worry about concurrent modifications to this.rooms.
                */

                try {
                    room.book(guest);
                    booked = true;
                    break;
                } catch (NotAvailableException e) {
                    // Try booking the next room
                }

            }

        }

        if (!booked) {
            throw new NotAvailableException("No room available of type "
                                            + roomType);
        }

    }


    public void bookRoom(String guest)
        throws NotAvailableException, RemoteException {

        boolean booked = false;

        for (Room room : this.rooms) {

            /*
              (See the note in bookRoom(int roomType, String guest) above.)
            */

            try {
                room.book(guest);
                booked = true;
                break;
            } catch (NotAvailableException e) {
                // Try booking the next room
            }

        }

        if (!booked) {
            throw new NotAvailableException("No room available");
        }

    }


    public Set<String> registeredGuests()
        throws RemoteException {

        Set<String> guests = new HashSet<String>();

        /*
          Once a room is booked, it cannot be unbooked. Otherwise, this
          code would not be thread safe.
        */

        for (Room room : this.rooms) {
            if (!room.isAvailable()) {
                try {
                    guests.add(room.getGuest());
                } catch (NotBookedException e) {
                    System.err.println(
                        "HotelImpl exception: reserved room is not reserved");
                }
            }
        }

        return guests;

    }


    public Set<Availability> availableRooms()
        throws RemoteException {

        Set<Availability> availables = new HashSet<Availability>();
        boolean typeFound;

        /*
          The resulting availability snapshot might not represent any specific
          moment in time. However, this is not a big concern in this specific
          case.
          As an example where the snapshot will be weird, consider the
          following course of events:

            A SimpleHotel instance with available rooms A and B of different
            types:
            - Client X invokes the availableRooms() method and the code loops
              over rooms A and B in that order.
            - The isAvailable() method for room A returns true and the
              availability count for the type of room A is incremented.
            - Client Y books room A.
            - Client Y books room B.
            - The isAvailable() method for room B returns false and the
              availability count for the type of room A is not incremented.
            The resulting availability snapshot makes us believe there was a
            moment in time that room A was available and room B was booked. In
            reality there was never such a situation.

          We don't mind this behaviour though.
        */

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
                    availables.add(new Availability(
                                       room.getRoomType().getType(),
                                       room.getRoomType().getPrice(),
                                       1));
                }

            }

        }

        return availables;

    }


    public SimpleHotel()
        throws RemoteException {

        /*
          The HashSet is not synchronized, but this is no problem for
          SimpleHotel because the set of rooms cannot be modified after the
          hotel is created.
        */
        rooms = new HashSet<Room>();

        /*
          Our hotel must have some rooms:
        */
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

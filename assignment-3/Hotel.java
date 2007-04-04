import java.util.Set;
import java.util.HashSet;


class Hotel {


    private Set<Room> rooms;


    public void bookRoom(RoomType type, String guest) throws NotAvailableException {

    }


    public List<String> registeredGuests() {

    }


    public Set<Available> availableRooms() {

    }


    public Hotel() {

        rooms = new HashSet<Room>(); /* be carefull, HashSet is not synchronized */

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

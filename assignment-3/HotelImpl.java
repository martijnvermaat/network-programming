import java.rmi.server.UnicastRemoteObject;
import java.rmi.RemoteException;
import java.util.Set;
import java.util.HashSet;
import java.util.List;
import java.util.ArrayList;


class HotelImpl extends UnicastRemoteObject implements Hotel {


    private Set<Room> rooms;


    public void bookRoom(RoomType type, String guest)
        throws NotAvailableException, RemoteException {

    }


    public List<String> registeredGuests()
        throws RemoteException {
        ArrayList<String> list = new ArrayList<String>();
        return list;

    }


    public Set<Available> availableRooms()
        throws RemoteException {
        HashSet<Available> set = new HashSet<Available>();
        return set;
    }


    public HotelImpl()
        throws RemoteException {

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

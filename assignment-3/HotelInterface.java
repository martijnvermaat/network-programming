import java.rmi.Remote;
import java.rmi.RemoteException;
import java.rmi.Naming;
import java.util.Set;
import java.util.HashSet;
import java.util.List;
import java.util.ArrayList;

public interface HotelInterface extends Remote {
        public void bookRoom(RoomType type, String guest) throws NotAvailableException, RemoteException;
        public List<String> registeredGuests() throws RemoteException;
        public Set<Available> availableRooms() throws RemoteException;
}

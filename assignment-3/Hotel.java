import java.rmi.Remote;
import java.rmi.RemoteException;
import java.util.Set;
import java.util.List;


public interface Hotel extends Remote {

    public void bookRoom(RoomType type, String guest)
        throws NotAvailableException, RemoteException;

    public List<String> registeredGuests()
        throws RemoteException;

    public Set<Available> availableRooms()
        throws RemoteException;

}

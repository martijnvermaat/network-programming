import java.rmi.Remote;
import java.rmi.RemoteException;
import java.util.Set;


public interface Hotel extends Remote {

    public void bookRoom(int roomType, String guest)
        throws NotAvailableException, RemoteException;

    public Set<String> registeredGuests()
        throws RemoteException;

    public Set<Availability> availableRooms()
        throws RemoteException;

}

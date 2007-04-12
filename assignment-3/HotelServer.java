import java.rmi.Naming;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.RemoteException;


public class HotelServer {


    public HotelServer() {

        try {
            LocateRegistry.createRegistry(Registry.REGISTRY_PORT);
        } catch(RemoteException e) {
            System.err.println("Could not start registry. Server aborted.");
            System.err.println(e.getMessage());
            System.exit(1);
        }
           

        try {
            Hotel ourHotel = new SimpleHotel();
            Naming.rebind("rmi://localhost/HotelService", ourHotel);
        } catch(Exception e) { // TODO: too generic? Perhaps add all exceptions
            System.err.println("HotelServer error: " + e.getMessage());
        }

    }


    public static void main(String[] args) {
        new HotelServer();
    }


}


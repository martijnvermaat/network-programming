import java.rmi.*;
import java.rmi.server.*;
import java.rmi.registry.*;
import java.net.InetAddress;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;

public class HotelServer extends UnicastRemoteObject implements HotelServerInterface {

    public HotelServer() throws RemoteException {
        super();
    }
    
    public void start(String[] args) {
        try {
            LocateRegistry.createRegistry(Registry.REGISTRY_PORT);
        } catch(Exception e) {
            System.err.println("Could not create registry: " + e.getMessage());
            e.printStackTrace();
        
        }
        
        try {
            Hotel hotel = new Hotel();
            Naming.bind("//127.0.0.1/Hotel", hotel);
        } catch(Exception e) {
            System.err.println("HotelServer exception: " + e.getMessage());
        }
        
        waitForExit();
        
        try {
            Naming.unbind("//127.0.0.1/Hotel");
        } catch(Exception e) {
            System.err.println("Could not unbind: " + e.getMessage());
        }
    }
    
    private void waitForExit() {
        System.out.println("Type exit to terminate HotelServer");
        try {
            BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
            String str = "";
            while (!str.equals("exit")) {
                System.out.print(">  ");
                str = in.readLine();
            }
        } catch (IOException e) { }
        System.out.println("Bye!");
    }
    
    public static void main(String[] args) {
        try {
            new HotelServer().start(args);
        } catch(Exception e) {
            System.err.println("HotelServer exception: " + e.getMessage());
            e.printStackTrace();
        }
        System.exit(0);
    }
}


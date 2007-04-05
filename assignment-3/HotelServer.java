import java.rmi.Naming;


public class HotelServer {


    public HotelServer() {

        try {
            Hotel h = new HotelImpl();
            Naming.rebind("rmi://localhost/HotelService", h);
        } catch(Exception e) {
            System.err.println("HotelServer exception: " + e.getMessage());
        }

    }


    public static void main(String[] args) {
        new HotelServer();
    }


}


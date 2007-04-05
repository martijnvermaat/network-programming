import java.rmi.Naming;


public class HotelServer {


    public HotelServer() {

        try {
            Hotel ourHotel = new SimpleHotel();
            Naming.rebind("rmi://localhost/HotelService", ourHotel);
        } catch(Exception e) {
            System.err.println("HotelServer exception: " + e.getMessage());
        }

    }


    public static void main(String[] args) {
        new HotelServer();
    }


}


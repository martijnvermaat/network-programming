import java.io.Serializable;


public class Availability implements Serializable {


    private int   type;
    private float price;
    private int   numberOfRooms;


    public int getType() {
        return this.type;
    }


    public float getPrice() {
        return this.price;
    }


    public int getNumberOfRooms() {
        return this.numberOfRooms;
    }


    public void setNumberOfRooms(int n) {
        this.numberOfRooms = n;
    }


    public Availability(int type, float price) {
        this(type, price, 0);
    }


    public Availability(int type, float price, int numberOfRooms) {
        this.type = type;
        this.price = price;
        this.numberOfRooms = numberOfRooms;
    }


}

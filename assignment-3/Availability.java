import java.io.Serializable;


public class Availability implements Serializable, Comparable<Availability> {


    private int   type;
    private float price;
    private int   numberOfRooms;


    public int compareTo(Availability a) {

        if (getType() < a.getType()) {
            return -1;
        } else if (a.getType() < getType()) {
            return 1;
        }

        if (getNumberOfRooms() < a.getNumberOfRooms()) {
            return -1;
        } else if (a.getNumberOfRooms() < getNumberOfRooms()) {
            return 1;
        }

        return 0;

    }


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

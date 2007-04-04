public class Room {


    private RoomType type;
    private boolean reserved = false;
    private String guest = null;


    public boolean isReserved() {
        return this.reserved;
    }


    public void reserve(String guest) throws NotAvailableException {
        if (isReserved()) {
            throw new NotAvailableException();
        }
        this.reserved = true;
        this.guest = guest;
    }


    public String guest() throws NotReservedException {
        if (!isReserved()) {
            throw new NotReservedException();
        }
        return this.guest;
    }


    public Room (RoomType type) {
        this.type = type;
    }


}

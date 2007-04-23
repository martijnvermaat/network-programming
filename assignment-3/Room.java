public class Room {


    private RoomType roomType;
    private boolean  available;
    private String   guest;


    public RoomType getRoomType() {
        return this.roomType;
    }


    public String getGuest() throws NotBookedException {
        if (isAvailable()) {
            throw new NotBookedException();
        }
        return this.guest;
    }


    public boolean isAvailable() {
        return this.available;
    }


    synchronized public void book(String guest) throws NotAvailableException {
        if (!isAvailable()) {
            throw new NotAvailableException("This room is not available");
        }
        this.available = false;
        this.guest = guest;
    }


    public Room (RoomType roomType) {
        this.roomType = roomType;
        this.available = true;
        this.guest = null;
    }


}

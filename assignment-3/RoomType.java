class RoomType {


    private static int highestType = 0;

    private int type;
    private float price;


    public int type() {
        return this.type;
    }


    public float price() {
        return this.price();
    }


    public RoomType(float price) {
        this.price = price;
        this.type = highestType++;
    }


}

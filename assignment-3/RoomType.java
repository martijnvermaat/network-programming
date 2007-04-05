class RoomType {


    private static int highestType = 0;

    private int type;
    private float price;


    public int getType() {
        return this.type;
    }


    public float getPrice() {
        return this.price;
    }


    public RoomType(float price) {
        this.price = price;
        this.type = ++highestType;
    }


}

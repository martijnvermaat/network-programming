class RoomType {


    /*
      This static counter is an easy way to guarantee every RoomType instance
      has a different type number.
    */
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

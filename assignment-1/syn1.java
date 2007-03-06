public class syn1 extends Thread {

    private String message;

    public syn1 (String message) {
        this.message = message;
    }

    synchronized public static void display (String s) {

        for (int i = 0; i < s.length(); i++) {
            System.out.print(s.charAt(i));
            try {
                Thread.sleep(0, 100);
            } catch (InterruptedException e) {
                System.out.println("Hooo!");
            }
        }

    }

    public void run () {

        for (int i = 0; i < 10; i++) {
            syn1.display(message);
        }

    }

    public static void main (String[] args) {

        Thread bonjour = new syn1("Bonjour monde\n");
        Thread hello = new syn1("Hello world\n");
        bonjour.start();
        hello.start();

    }

}

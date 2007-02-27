public class syn2 extends Thread {

    private String message;
    private static Display display;

    public syn2 (String message) {
        this.message = message;
    }


    public void run () {

        for (int i = 0; i < 10; i++) {
            try {
                syn2.display.synchronizedDisplay(message);
            } catch (InterruptedException e) {
                System.out.println("Haaaaa!");
            }
        }

    }

    public static void main (String[] args) {

        display = new Display();

        Thread ab = new syn2("ab");
        Thread cd = new syn2("cd\n");
        ab.start();
        cd.start();

    }

}

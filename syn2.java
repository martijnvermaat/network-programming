public class syn2 extends Thread {

    private String message;
    private boolean isNotified = false;

    public syn2 (String message) {
        this.message = message;
    }

    public static void display (String s) {

        for (int i = 0; i < s.length(); i++) {
            System.out.print(s.charAt(i));
            try {
                Thread.sleep(0, 100);
            } catch (InterruptedException e) {
                System.out.println("Hooo!");
            }
        }

    }

    synchronized public void synchronizedDisplay (String s) throws InterruptedException {
        if (s.equals("ab")) {
            isNotified = false;
            wait();
            isNotified = true;
            syn2.display(s);
            notifyAll();
        } else {
            while (!isNotified) { notifyAll(); }
            wait();
            syn2.display(s);
        }
    }

    public void run () {

        for (int i = 0; i < 10; i++) {
            try {
                synchronizedDisplay(message);
            } catch (InterruptedException e) {
                System.out.println("Haaaaa!");
            }
        }

    }

    public static void main (String[] args) {

        Thread ab = new syn2("ab");
        Thread cd = new syn2("cd\n");
        ab.start();
        cd.start();

    }

}

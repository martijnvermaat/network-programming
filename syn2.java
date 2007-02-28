public class syn2 extends Thread {

    private String message;
    private static Display display;

    public syn2 (String message) {
        this.message = message;
    }
    
    private static class Display {

        private void display (String s) {
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
                wait();
                display(s);
                notify();
            } else {
                notify();
                wait();
                display(s);
            }
        }
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

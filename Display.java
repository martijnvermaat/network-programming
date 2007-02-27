public class Display {

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
            notifyAll();
        } else {
            notifyAll();
            wait();
            display(s);
        }
    }

}

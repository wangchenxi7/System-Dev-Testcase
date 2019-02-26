// Case to test Runtime.getRuntime().printInfo(obj);

public class Case99 {
  public static void main(String args[]) {
    Runtime.getRuntime().iterationStart();

    Object test = new Object();
    System.out.println(test.toString());

    int yac_id = Runtime.getRuntime().getYakThreadID();
    System.out.println("Yak thread id: " + yac_id);

    Runtime.getRuntime().iterationEnd();
  }
}

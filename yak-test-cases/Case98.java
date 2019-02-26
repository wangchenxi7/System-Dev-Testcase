// Case to test Runtime.getRuntime().joinRegion(region_id, thread_id);

public class Case98 {
  public static void main(String args[]) {
    int region_id = Runtime.getRuntime().iterationStart();
    int thread_id = Runtime.getRuntime().getYakThreadID();

    System.out.println("Yak region id: " + region_id);
    System.out.println("Yak thread id: " + thread_id);
    Runtime.getRuntime().joinRegion(region_id, thread_id);

    Runtime.getRuntime().detachRegion();

    Runtime.getRuntime().iterationEnd();
  }
}

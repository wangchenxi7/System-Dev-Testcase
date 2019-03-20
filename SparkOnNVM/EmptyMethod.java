import java.util.HashMap;
import java.util.UUID;
import java.util.ArrayList;

public class EmptyMethod {

  static final int MAX = 30;

  public static void method() {

  }

  public static HashMap<String, String> generateHashMap() {
    HashMap<String, String> m = new HashMap<String, String>();
    int size = UUID.randomUUID().toString().length();
    for (int i = 0; i < size; i++) {
      String k = UUID.randomUUID().toString();
      String v = UUID.randomUUID().toString();
      m.put(k, v);
    }
    return m;
  }

  public static void runTest(int limit) {
    ArrayList<HashMap<String, String>> list = new ArrayList<HashMap<String, String>>();

    long before;
    long after;
    // First, figure out the time for an empty loop
    before = System.currentTimeMillis();
    int count = 0;
    for (int index = 0; index < limit; index++) {
      HashMap<String, String> m = generateHashMap();
      int size = m.size();
      if (size < MAX) {
	for (int i = 0; i < MAX - size; i++) {
	  String k = UUID.randomUUID().toString();
	  String v = UUID.randomUUID().toString();
	  m.put(k, v);
	}
      }
      count++;
      list.add(m);
    }
    after = System.currentTimeMillis();
    long loopTime = after - before;
    // System.out.println("Loop time: " + Long.toString(loopTime)
    // + " milliseconds");
    // Then time the method call in the loop
    before = System.currentTimeMillis();
    for (int index = 0; index < 1 * 1000 * 1000; index += 1) {
      method();
    }
    after = System.currentTimeMillis();
    long methodTime = after - before;
    /*
     * System.out.println("Method time: " + Long.toString(methodTime) +
     * " milliseconds"); System.out.println("Method time - Loop time: " +
     * Long.toString(methodTime - loopTime) + " milliseconds");
     * System.out.println("COUNT = " + count); System.out.println();
     * 
     * 
     */
    DummyThread.threadMessage("COUNT = " + count);
  }

  public static void main(String[] arg) {
    // Warm up the virtual machine, and time it
    if (arg.length < 2) {
      System.out
	.println("Usage: EmptyMethod <times to loop> <times to repeat>");
      System.exit(0);
    }
    System.out.println("Starting Program");
    int lim = Integer.parseInt(arg[0]);
    int times = Integer.parseInt(arg[1]);
    System.out.println("Warming up");
    runTest(100);
    System.out.println("Warm up finished");
      
    for (int i = 0; i < times; i++) {
    DummyThread.threadMessage("Iteration: " + i);
    //// Runtime.getRuntime().gc();
         Runtime.getRuntime().iterationStart();
    //// Runtime.getRuntime().gc();
    //// Runtime.getRuntime().createRegion();
    runTest(lim);
    //// Runtime.getRuntime().gc();
    //// Runtime.getRuntime().deleteRegion();
     Runtime.getRuntime().iterationEnd();
    //
    }
    DummyThread.threadMessage("Main Thread Done");

    /* DummyThread.threadMessage("Main Thread Starting ");
    DummyThread t1 = new DummyThread(lim, times);
    DummyThread t2 = new DummyThread(2*lim, times);
    
    t1.start();
    t2.start();
    
    try {
      t1.join();
      t2.join();
    } catch (InterruptedException e) {
      // TODO Auto-generated catch block
      e.printStackTrace();
    }

    DummyThread t3 = new DummyThread(lim, times);
    t3.start();
    try {
      t3.join();
      
    } catch (InterruptedException e) {
      // TODO Auto-generated catch block
      e.printStackTrace();
    }

    
    DummyThread.threadMessage("Main Thread Done");
    */
  }

  static class DummyThread extends Thread {
    private int lim =0;
    private int times =0;
    
    public DummyThread(int l, int t) {
      this.lim= l;
      this.times = t;
      
    }
    public static HashMap<String, String> generateHashMap() {
      HashMap<String, String> m = new HashMap<String, String>();
      int size = UUID.randomUUID().toString().length();
      for (int i = 0; i < size; i++) {
	String k = UUID.randomUUID().toString();
	String v = UUID.randomUUID().toString();
	m.put(k, v);
      }
      return m;
    }

    public static void runTest(int limit) {
      ArrayList<HashMap<String, String>> list = new ArrayList<HashMap<String, String>>();

      long before;
      long after;
      // First, figure out the time for an empty loop
      before = System.currentTimeMillis();
      int count = 0;
      for (int index = 0; index < limit; index++) {
	HashMap<String, String> m = generateHashMap();
	int size = m.size();
	if (size < MAX) {
	  for (int i = 0; i < MAX - size; i++) {
	    String k = UUID.randomUUID().toString();
	    String v = UUID.randomUUID().toString();
	    m.put(k, v);
	  }
	}
	count++;
	list.add(m);
      }
      after = System.currentTimeMillis();
      long loopTime = after - before;
      // System.out.println("Loop time: " + Long.toString(loopTime)
      // + " milliseconds");
      // Then time the method call in the loop
      before = System.currentTimeMillis();
      for (int index = 0; index < 1 * 1000 * 1000; index += 1) {
	method();
      }
      after = System.currentTimeMillis();
      long methodTime = after - before;
      /*
       * System.out.println("Method time: " + Long.toString(methodTime) +
       * " milliseconds"); System.out.println("Method time - Loop time: " +
       * Long.toString(methodTime - loopTime) + " milliseconds");
       * System.out.println("COUNT = " + count); System.out.println();
       */
    }
    
    public static void threadMessage(String message) {
              String threadName =
		Thread.currentThread().getName();
	      System.out.format("%s: %s%n",
				threadName,
				message);
    }
    @Override 
      public void run() {
      for (int i = 0; i < times; i++) {
	threadMessage(" Iteration: " + i);
	// Runtime.getRuntime().gc();
	Runtime.getRuntime().iterationStart();
	// Runtime.getRuntime().gc();
	// Runtime.getRuntime().createRegion();
	EmptyMethod.runTest(lim);
	// Runtime.getRuntime().gc();
	// Runtime.getRuntime().deleteRegion();
	Runtime.getRuntime().iterationEnd();
	try {
	  threadMessage(" Sleeping for 1 sec");
	  Thread.sleep(1000);
	  threadMessage(" Waking");
	} catch (InterruptedException e) {
	  // TODO Auto-generated catch block
	  e.printStackTrace();
	}
	//Runtime.getRuntime().gc();

      }
    }
  }
}



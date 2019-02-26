// Case to test Runtime.getRuntime().printInfo(obj);

import java.util.*;

public class Case31 {

  public static void main(String args[]) {
    HashMap<Integer, Integer> hashmap = null;
    Runtime.getRuntime().printInfo(hashmap);
    hashmap = new HashMap<Integer, Integer>();
    Runtime.getRuntime().printInfo(hashmap);

    Object obj = new Object();
    Runtime.getRuntime().printInfo(obj);
    obj = new HashMap<String, String>();
    Runtime.getRuntime().printInfo(obj);
  }
}

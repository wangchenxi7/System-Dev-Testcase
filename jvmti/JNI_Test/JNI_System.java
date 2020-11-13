/**
 * Test the added JNI call in the Class System.
 * 
 */
import java.lang.Object;

public class JNI_System {

  public static void main(String[] args) {
  
    // Test#1, invoke into JNI call via class java.lang.System
    System.test();    

    // Test#2, Pass object instance into JVM via JNI call.
    testFunc test_obj = new testFunc();
    System.testWithParameter(test_obj,null, null, null, null, 1);
    
  }

  
}



class testFunc{

	int a = 5;
  
  int getContent(){
    return a;
  }

}

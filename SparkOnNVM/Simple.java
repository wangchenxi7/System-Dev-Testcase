import java.lang.*;

public class Simple{

  public static void main(String[] args){
    //Test tt = new Test();
    Runtime.getRuntime().iterationStart();
    Test t = new Test();
    Runtime.getRuntime().iterationEnd();
    System.out.println(t.toString());

  }
}


class Test {
  int i;
  public Test() {
    i = 0;
  }
}

class A{
  double f = 0;;
} 


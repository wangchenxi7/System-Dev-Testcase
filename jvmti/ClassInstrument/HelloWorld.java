class HelloWorld {

  public static void main(String[] args) {

    System.out.println("Java Hello world!");

    testFunc t = new testFunc();
    System.out.println( "testFunc.a : " + t.getContent() );

  }

}



class testFunc{

	int a = 5;
  
  int getContent(){
    return a;
  }

}

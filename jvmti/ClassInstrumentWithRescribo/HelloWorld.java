class HelloWorld {

  public static void main(String[] args) {

    System.out.println("Java Hello world!");

    testFunc t = new testFunc();
    t.storeContent(10);
    int val = t.getContent();
    System.out.println( "testFunc t.a : " + val );

    testFunc m = new testFunc();
    m.duplicate(t);
    val = m.getContent();
    System.out.println( "testFunc m.a : " + val );

  }

}



class testFunc{

	int a = 0;
  
  void storeContent(int val ){
    this.a = val;
  }

  void duplicate(testFunc obj){
    this. a = obj.a;
  }

  int getContent(){
    return a;
  }

}

class CaseGetPutEntry {

  public static void main(String[] args) {
    
    int i,j;
    int array_length = 64*1024*1024;
    long sum=0;

    System.out.println("Phase 1: allocate object array and its contents.");
    // allocate the object array
    objItem obj_array[] = new objItem[array_length];
    // allocate the object intences
    for(i=0; i< array_length; i++){
      obj_array[i] = new objItem();
    }

    // do this multiple times to trigger the JIT

    for(j=0; j< 5; j++){
      System.out.println("Phase 2: Do initialization.");
      for(i=0; i< array_length; i++ ){
        obj_array[i].storeContent(i);
        obj_array[i].storeContent("String" + i);
      }

      // retrieve the content 
      System.out.println("Phase 3: Read the content of the object array.");
      for(i=0; i< array_length; i++){
        sum += obj_array[i].getContent();
      }
    }// end of for

    System.out.println("sum " + sum +  ", Done and exit.");
    return;
  } // end of main function.

}



class objItem{

	int a = 0;
  String b = "";

  void storeContent(int val ){
    this.a = val;
  }

  void storeContent(String val){
    this.b = val;
  }


  void duplicate(testFunc obj){
    this. a = obj.a;
  }

  int getContent(){
    return a;
  }

}

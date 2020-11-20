class ParameterParsing {

  public static void main(String[] args) {
    
    int i,j;
    byte[] byte_array = "byte array".getBytes();
    double[] double_array = {1.1, 2.2, 3.3, 4.4, 5.5};
    objItem[] object_array = new objItem[5];
    objItem[][] two_dim_object_array = new objItem[4][4];
    objItem target = new objItem();

    for(i=0; i<5; i++){
      object_array[i] = new objItem();
    }

    target.storeContent("Val for String 1", byte_array, 7,  two_dim_object_array, double_array, 5, object_array, 5, "Value for String 2");

    System.out.println(target.str1 + " " + target.str2 );

  }

}



class objItem{

  int a = 0;
  double double_array[] = new double[1024];
  String str1 = "";
  String str2 = "";

  void storeContent(int val ){
    this.a = val;
  }

  void storeContent(String val){
    this.str1 = val;
  }

  void storeContent(String sval, byte[] bval, int ival, objItem two_dim_obj_array[][],  double darray[], int darray_length, objItem obj_array[], int objarray_length, String sval2){
    this.str1 = sval;
    this.a = ival;
    this.str2 = sval2;
    for(int i=0; i< darray_length; i++)
      this.double_array[i] = darray[i]; // value copy
  }


  void duplicate(testFunc obj){
    this. a = obj.a;
  }

  int getContent(){
    return a;
  }

}

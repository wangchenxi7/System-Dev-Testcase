//import java.util.Random;

import java.lang.reflect.Field;
import sun.misc.Unsafe;


public class ReadBarrierTest {
  private static Unsafe unsafe;

  static
  {
      try
      {
          Field field = Unsafe.class.getDeclaredField("theUnsafe");
          field.setAccessible(true);
          unsafe = (Unsafe)field.get(null);
      }
      catch (Exception e)
      {
          e.printStackTrace();
      }
  }


  public static long addressOf(Object o)
  throws Exception
  {
      Object[] array = new Object[] {o};

      long baseOffset = unsafe.arrayBaseOffset(Object[].class);
      int addressSize = unsafe.addressSize();
      long objectAddress;
      switch (addressSize)
      {
          case 4:
              objectAddress = unsafe.getInt(array, baseOffset);
              break;
          case 8:
              objectAddress = unsafe.getLong(array, baseOffset);
              break;
          default:
              throw new Error("unsupported address size: " + addressSize);
      }       

      return(objectAddress);
  }

  
    public static void main(String args[])
    throws Exception
    {
      objItem objTest = new objItem(7,8);
      int val = objTest.val;
      internalItem interObjTest = objTest.field_2;

      System.out.println("objTest address 0x" + Long.toHexString(addressOf(objTest)) + " val is " + val);
      System.out.println("interObjTest id :" + interObjTest.id );
    }
    
  }
  
  // obj header +
  // offload: size 4*2 + 8*4 = 40 bytes
  class objItem{
  
   public int  key;
   public int  val;
  
    // add some fields 
    internalItem field_1;
    internalItem field_2;
    internalItem field_3;
    internalItem field_4;
  
  
    public objItem(int key, int val){
      this.key =key;
      this.val = val;

      initialize();
    }

    public void initialize(){
      field_1 = new internalItem(1);
      field_2 = new internalItem(2);
      field_3 = new internalItem(3);
      field_4 = new internalItem(4);
    }
  
  }
  
  // obj header
  // offload :size 4 bytes
  class internalItem{
    int id;
  
    public internalItem(int id){
      this.id = id;
    }

    public internalItem(){
      this.id = -1;
    }
  }
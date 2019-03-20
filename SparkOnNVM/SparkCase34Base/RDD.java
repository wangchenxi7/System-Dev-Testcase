package SparkCase34Base;

public class RDD {
	
	int     length;
	Datum[] values;
	
	private final int MINIMUM_LENGTH = 256; 
	
	public RDD( int reserve ) {
		length = 0;
		
		reserve = Math.max(reserve, MINIMUM_LENGTH);
		
		values = new Datum[reserve]; 
	}
	
	public void add( Datum datum ) {
		if ( values.length <= length ) {
			Datum[] temp = new Datum[values.length * 2];
			
			System.arraycopy(values, 0, temp, 0, length);
			
			values = temp;
		}
		
		values[length++] = datum;
	}
	
	public int size() {
		return length;
	}
	
	public RDDIterForward begin() {
		return new RDDIterForward( this );
	}
	
	public RDDIterBackward rbegin() {
		return new RDDIterBackward( this );
	}

	public Datum get_elem(int inx) {
		return values[inx];
	}
	
}

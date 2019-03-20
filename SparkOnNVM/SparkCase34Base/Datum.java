package SparkCase34Base;

import java.util.Random;

public class Datum {
	
	public int value;
	
	public Datum( Random rand ) {
		value = (int)(Integer.MAX_VALUE * rand.nextGaussian());
	}
	
	public Datum( Datum datum ) {
		this.value = datum.value;
	}
	
	public Datum( int value ) {
		this.value = value;
	}
}
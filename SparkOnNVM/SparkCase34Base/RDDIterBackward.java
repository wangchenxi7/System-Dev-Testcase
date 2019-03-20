package SparkCase34Base;

import java.util.NoSuchElementException;

public class RDDIterBackward extends DatumIterator {

	RDDIterBackward( RDD rdd ) {
		this.values = rdd.values;
		this.position = rdd.length;
	}
	
	@Override
	public Datum next() {
		if ( 0 < position ) {
			return values[--position];
		}
		else
		{
			throw new NoSuchElementException();
		}
	}

	@Override
	public boolean hasNext() {
		return 0 < position;
	}
	
	@Override
	public void remove() {
		throw new UnsupportedOperationException();
	}
	private Datum[] values;
	private int position;
}


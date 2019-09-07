package SparkCase34Base;

import java.util.NoSuchElementException;

public class RDDIterForward extends DatumIterator {

	RDDIterForward( RDD rdd ) {
		this.values = rdd.values;
		this.position = 0;
		this.length = rdd.length;
	}
	
	@Override
	public Datum next() {
		if ( position < length ) {
			return values[position++];
		}
		else
		{
			throw new NoSuchElementException();
		}
	}

	@Override
	public boolean hasNext() {
		return position < length;
	}
	
	@Override
	public void remove() {
		throw new UnsupportedOperationException();
	}
	private Datum[] values;
	private int position;
	private int length;
}
